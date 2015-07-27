/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <bundle.h>
#include <bundle_internal.h>
#include <aul.h>
#include <dlog.h>
#include <unistd.h>
#include <system_info.h>
#include <pkgmgr-info.h>
#include <pkgmgrinfo_resource.h>
#include <vconf.h>

#include "app_resource_manager.h"
#include "app_common.h"

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "CAPI_APPFW_RESOURCE_MANAGER"

#define WEIGHT_SCREEN_DPI 10000
#define WEIGHT_SCREEN_DPI_RANGE 10000
#define WEIGHT_SCREEN_BPP 1000
#define WEIGHT_SCREEN_WIDTH_RANGE 100
#define WEIGHT_SCREEN_LARGE 10
#define WEIGHT_PLATFORM_VERSION 1000000
#define WEIGHT_LANGUAGE 100000

#define THRESHOLD_TO_CLEAN 50	/* app_resource_manager_trim_cache */

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#define MAX_PATH  1024

typedef struct {
	resource_data_t *data;
	GHashTable *cache;
} resource_manager_t;

typedef struct {
	char *output;
	int hit_cnt;
	bool remove;
} resource_cache_context_t;

typedef struct {
	const char *bundle_attr_key;
	unsigned int bundle_attr_value;
} resource_node_attr_t;

enum {
	NODE_ATTR_MIN = 0,
	NODE_ATTR_SCREEN_DPI,
	NODE_ATTR_SCREEN_DPI_RANGE,
	NODE_ATTR_SCREEN_WIDTH_RANGE,
	NODE_ATTR_SCREEN_LARGE,
	NODE_ATTR_SCREEN_BPP,
	NODE_ATTR_PLATFORM_VER,
	NODE_ATTR_LANGUAGE,
	NODE_ATTR_MAX
};

static resource_manager_t *resource_handle = NULL;

static resource_node_attr_t map[] = {
		{ RSC_NODE_ATTR_SCREEN_DPI, NODE_ATTR_SCREEN_DPI },
		{ RSC_NODE_ATTR_SCREEN_DPI_RANGE, NODE_ATTR_SCREEN_DPI_RANGE },
		{ RSC_NODE_ATTR_SCREEN_WIDTH_RANGE, NODE_ATTR_SCREEN_WIDTH_RANGE },
		{ RSC_NODE_ATTR_SCREEN_LARGE, NODE_ATTR_SCREEN_LARGE },
		{ RSC_NODE_ATTR_SCREEN_BPP, NODE_ATTR_SCREEN_BPP },
		{ RSC_NODE_ATTR_PLATFORM_VER, NODE_ATTR_PLATFORM_VER },
		{ RSC_NODE_ATTR_LANGUAGE, NODE_ATTR_LANGUAGE },
};

static GHashTable *attr_key = NULL;
static const char *res_path = NULL;
static char *cur_language = NULL;

static gint __resource_manager_comp(gconstpointer a, gconstpointer b)
{
	resource_group_t *rsc_group = (resource_group_t *) a;

	return strcmp(rsc_group->type, b);
}

static void __bundle_iterator_get_valid_nodes(const char *key, const int type,
		const bundle_keyval_t *kv, void *data)
{
	unsigned int node_attr;
	bool *invalid = (bool *) data;
	bool ret_bool = true;
	int min, max;
	char from[5] = { 0, };
	char to[3] = { 0, };
	bool t_val;
	char *val;
	size_t size;
	static int screen_dpi = -1;
	static int screen_width = -1;
	static int screen_size_large = -1;
	static char *version = NULL;
	static int screen_bpp = -1;

	if (*invalid)
		return;

	bundle_keyval_get_basic_val((bundle_keyval_t *) kv, (void**) &val, &size);

	node_attr = (guint) g_hash_table_lookup(attr_key, key);
	if (node_attr <= NODE_ATTR_MIN || node_attr >= NODE_ATTR_MAX) {
		LOGE("INVALID_PARAMETER(0x%08x), node_attr(%d)",
				APP_RESOURCE_ERROR_INVALID_PARAMETER, node_attr);
		*invalid = true;
		return;
	}

	switch (node_attr) {
		case NODE_ATTR_SCREEN_DPI:
			if (screen_dpi == -1)
				system_info_get_platform_int("http://tizen.org/feature/screen.dpi",
						&screen_dpi);
			if (screen_dpi != atoi(val))
				*invalid = true;
			break;

		case NODE_ATTR_SCREEN_DPI_RANGE:
			sscanf(val, "%s %d %s %d", from, &min, to, &max);
			if (screen_dpi == -1)
				system_info_get_platform_int("http://tizen.org/feature/screen.dpi",
						&screen_dpi);

			if (!(min <= screen_dpi && screen_dpi <= max))
				*invalid = true;
			break;

		case NODE_ATTR_SCREEN_WIDTH_RANGE:
			sscanf(val, "%s %d %s %d", from, &min, to, &max);
			if (screen_width == -1)
				system_info_get_platform_int(
						"http://tizen.org/feature/screen.width", &screen_width);
			if (!(min <= screen_width && screen_width <= max))
				*invalid = true;
			break;

		case NODE_ATTR_SCREEN_LARGE:
			if (!(strcmp(val, "true")))
				t_val = true;
			else
				t_val = false;
			if (screen_size_large == -1) {
				system_info_get_platform_bool(
						"http://tizen.org/feature/screen.size.large", &ret_bool);
				if (ret_bool)
					screen_size_large = 1;
				else
					screen_size_large = 0;
			}
			if (((bool)screen_size_large) != t_val)
				*invalid = true;
			break;

		case NODE_ATTR_SCREEN_BPP:
			if (screen_bpp == -1)
				system_info_get_platform_int("http://tizen.org/feature/screen.bpp",
						&screen_bpp);
			if (screen_bpp != atoi(val))
				*invalid = true;
			break;

		case NODE_ATTR_PLATFORM_VER:
			if (version == NULL)
				system_info_get_platform_string(
						"http://tizen.org/feature/platform.version", &version);
			if (strcmp(version, val))
				*invalid = true;
			break;

		case NODE_ATTR_LANGUAGE:
			if (cur_language == NULL)
				cur_language = vconf_get_str(VCONFKEY_LANGSET);
			if (strncmp(cur_language, val, strlen(val)))
				*invalid = true;
			break;

		default:
			break;
	}
}

static void __bundle_iterator_get_best_node(const char *key, const char *val,
		void *data)
{
	unsigned int node_attr;
	unsigned int *weight = (unsigned int *) data;

	node_attr = (guint) g_hash_table_lookup(attr_key, key);
	if (node_attr <= NODE_ATTR_MIN || node_attr >= NODE_ATTR_MAX) {
		LOGE("INVALID_PARAMETER(0x%08x), node_attr(%d)",
				APP_RESOURCE_ERROR_INVALID_PARAMETER, node_attr);
		return;
	}

	switch (node_attr) {
		case NODE_ATTR_SCREEN_DPI:
			*weight += WEIGHT_SCREEN_DPI;
			break;

		case NODE_ATTR_SCREEN_DPI_RANGE:
			*weight += WEIGHT_SCREEN_DPI_RANGE;
			break;

		case NODE_ATTR_SCREEN_WIDTH_RANGE:
			*weight += WEIGHT_SCREEN_WIDTH_RANGE;
			break;

		case NODE_ATTR_SCREEN_LARGE:
			*weight += WEIGHT_SCREEN_LARGE;
			break;

		case NODE_ATTR_SCREEN_BPP:
			*weight += WEIGHT_SCREEN_BPP;
			break;

		case NODE_ATTR_PLATFORM_VER:
			*weight += WEIGHT_PLATFORM_VERSION;
			break;

		case NODE_ATTR_LANGUAGE:
			*weight += WEIGHT_LANGUAGE;
			break;

		default:
			break;
	}
}

static const char *__get_cache(app_resource_e type,
		const char *id)
{
	unsigned int total_len = 0;
	char *key = NULL;
	char *rsc_type;
	resource_cache_context_t *resource_cache = NULL;

	if (id == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x), id",
				APP_RESOURCE_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	if (type < APP_RESOURCE_TYPE_MIN || type > APP_RESOURCE_TYPE_MAX) {
		LOGE("INVALID_PARAMETER(0x%08x), type(%d)",
				APP_RESOURCE_ERROR_INVALID_PARAMETER, type);
		return NULL;
	} else {
		switch (type) {
			case APP_RESOURCE_TYPE_IMAGE:
				rsc_type = RSC_GROUP_TYPE_IMAGE;
				break;

			case APP_RESOURCE_TYPE_LAYOUT:
				rsc_type = RSC_GROUP_TYPE_LAYOUT;
				break;

			case APP_RESOURCE_TYPE_SOUND:
				rsc_type = RSC_GROUP_TYPE_SOUND;
				break;

			case APP_RESOURCE_TYPE_BIN:
				rsc_type = RSC_GROUP_TYPE_BIN;
				break;

			default:
				rsc_type = "NULL";
		}
	}

	if (resource_handle->cache == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x), hashtable",
				APP_RESOURCE_ERROR_INVALID_PARAMETER);
		return NULL;
	} else {
		total_len = strlen(rsc_type) + strlen(id) + 2;

		key = (char *) calloc(1, total_len);
		if (key == NULL) {
			LOGE("failed to create a resource_cache(0x%08x)",
					APP_RESOURCE_ERROR_OUT_OF_MEMORY);
			free(resource_cache);
			return NULL;
		}

		snprintf(key, total_len, "%s:%s", rsc_type, id);
		LOGD("key : %s", key);

		resource_cache = g_hash_table_lookup(resource_handle->cache, key);
		free(key);
		if (resource_cache == NULL) {
			LOGE("IO_ERROR(0x%08x), find list resource_cache",
					APP_RESOURCE_ERROR_IO_ERROR);
			return NULL;
		}

		resource_cache->hit_cnt++;
	}

	return resource_cache->output;
}

static gint __cache_hit_compare(gconstpointer a, gconstpointer b)
{
	const resource_cache_context_t *lhs = (const resource_cache_context_t *) a;
	const resource_cache_context_t *rhs = (const resource_cache_context_t *) b;

	return lhs->hit_cnt - rhs->hit_cnt;
}

static gboolean __cache_remove(gpointer key, gpointer value, gpointer user_data)
{
	resource_cache_context_t *c = (resource_cache_context_t *) (value);

	if (c->remove) {
		free(key);
		free(c->output);
		free(c);
		return TRUE;
	}

	return FALSE;
}

static void __trim_cache(void)
{
	GList *values = g_hash_table_get_values(resource_handle->cache);
	values = g_list_sort(values, __cache_hit_compare);

	int i = 0;
	GList *iter_list = values;
	while (iter_list != NULL) {
		if (i >= (THRESHOLD_TO_CLEAN / 2))
			break;

		resource_cache_context_t *c =
				(resource_cache_context_t *) (iter_list->data);
		c->remove = true;
		iter_list = g_list_next(iter_list);
		i++;
	}

	g_list_free(values);
	g_hash_table_foreach_remove(resource_handle->cache, __cache_remove, NULL);

}

static void __put_cache(app_resource_e type, const char *id,
		const char *val)
{
	unsigned int total_len = 0;
	char *key;
	char *rsc_type;
	resource_cache_context_t *resource_cache;
	LOGD("@@@@@@@@@@@@@@path : %s", val);

	/* To remove chache from the low frequency of use. */
	if (val == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x), fname",
				APP_RESOURCE_ERROR_INVALID_PARAMETER);
		return;
	}

	if (id == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x), id",
				APP_RESOURCE_ERROR_INVALID_PARAMETER);
		return;
	}

	if (type < APP_RESOURCE_TYPE_MIN || type > APP_RESOURCE_TYPE_MAX) {
		LOGE("INVALID_PARAMETER(0x%08x), type(%d)",
				APP_RESOURCE_ERROR_INVALID_PARAMETER, type);
		return;
	} else {
		switch (type) {
			case APP_RESOURCE_TYPE_IMAGE:
				rsc_type = RSC_GROUP_TYPE_IMAGE;
				break;

			case APP_RESOURCE_TYPE_LAYOUT:
				rsc_type = RSC_GROUP_TYPE_LAYOUT;
				break;

			case APP_RESOURCE_TYPE_SOUND:
				rsc_type = RSC_GROUP_TYPE_SOUND;
				break;

			case APP_RESOURCE_TYPE_BIN:
				rsc_type = RSC_GROUP_TYPE_BIN;
				break;

			default:
				rsc_type = "NULL";
		}
	}

	if (g_hash_table_size(resource_handle->cache) > THRESHOLD_TO_CLEAN)
		__trim_cache();

	resource_cache = (resource_cache_context_t *) calloc(1,
			sizeof(resource_cache_context_t));
	if (resource_cache == NULL) {
		LOGE("failed to create a resource_group(0x%08x)",
				APP_RESOURCE_ERROR_OUT_OF_MEMORY);
		return;
	}

	total_len = strlen(rsc_type) + strlen(id) + 2;

	key = (char *) calloc(1, total_len);
	if (key == NULL) {
		LOGE("failed to create a resource_cache(0x%08x)",
				APP_RESOURCE_ERROR_OUT_OF_MEMORY);
		free(resource_cache);
		return;
	}

	snprintf(key, total_len, "%s:%s", rsc_type, id);
	LOGD("key : %s", key);

	resource_cache->output = strdup(val);
	resource_cache->hit_cnt = 0;
	resource_cache->remove = false;

	g_hash_table_insert(resource_handle->cache, key, resource_cache);
}

static resource_group_t *__find_group(resource_data_t *data,
		int type)
{
	resource_group_t *rsc_group = NULL;
	char *rsc_type;

	if (data == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x), resource_data_t",
				APP_RESOURCE_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	if (type < APP_RESOURCE_TYPE_MIN || type > APP_RESOURCE_TYPE_MAX) {
		LOGE("INVALID_PARAMETER(0x%08x), type(%d)",
				APP_RESOURCE_ERROR_INVALID_PARAMETER, type);
		return NULL;
	} else {
		switch (type) {
			case APP_RESOURCE_TYPE_IMAGE:
				rsc_type = RSC_GROUP_TYPE_IMAGE;
				break;

			case APP_RESOURCE_TYPE_LAYOUT:
				rsc_type = RSC_GROUP_TYPE_LAYOUT;
				break;

			case APP_RESOURCE_TYPE_SOUND:
				rsc_type = RSC_GROUP_TYPE_SOUND;
				break;

			case APP_RESOURCE_TYPE_BIN:
				rsc_type = RSC_GROUP_TYPE_BIN;
				break;

			default:
				rsc_type = "NULL";
		}
	}

	GList* found = g_list_find_custom(data->group_list, rsc_type,
			__resource_manager_comp);
	if (found == NULL) {
		LOGE("IO_ERROR(0x%08x), find list resource_group %s",
				APP_RESOURCE_ERROR_IO_ERROR, rsc_type);
		return NULL;
	}

	rsc_group = (resource_group_t *) (found->data);

	return rsc_group;
}

static GList *__get_valid_nodes(resource_group_t *group,
		const char *id)
{
	GList *list = NULL;
	GList *valid_list = NULL;
	resource_node_t *valid_node = NULL;
	resource_node_t *rsc_node = NULL;

	if (group->node_list == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x), resource_group",
				APP_RESOURCE_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	list = g_list_first(group->node_list);

	char path_buf[MAX_PATH] = { 0, };
	while (list) {
		bool invalid = false;
		rsc_node = (resource_node_t *) list->data;

		snprintf(path_buf, MAX_PATH - 1, "%s%s/%s", res_path,
				rsc_node->folder, id);
		if (access(path_buf, R_OK) == 0) {
			bundle_foreach(rsc_node->attr, __bundle_iterator_get_valid_nodes,
					&invalid);

			if (!invalid) {
				valid_node = (resource_node_t *) list->data;
				valid_list = g_list_append(valid_list, valid_node);
			}
		}

		list = g_list_next(list);
	}

	return valid_list;
}

static resource_node_t *__get_best_node(GList *nodes)
{
	unsigned int weight_tmp = 0;
	resource_node_t *best_node = NULL;
	GList *list = NULL;

	if (nodes == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x), resource_node lists",
				APP_RESOURCE_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	list = g_list_first(nodes);

	while (list != NULL) {
		unsigned int weight = 0;
		resource_node_t *res_node = (resource_node_t *) (list->data);

		bundle_iterate(res_node->attr, __bundle_iterator_get_best_node, &weight);
		if (weight > weight_tmp) {
			best_node = res_node;
			weight_tmp = weight;
		}
		list = g_list_next(list);
	}

	return best_node;
}

static int __open(resource_manager_t **handle)
{
	int retval = APP_RESOURCE_ERROR_NONE;
	resource_manager_t *rsc_manager = NULL;
	char buf[1024] = { 0, };

	rsc_manager = (resource_manager_t *) calloc(1, sizeof(resource_manager_t));
	if (!rsc_manager) {
		LOGE("failed to create a resource_manager(0x%08x)",
				APP_RESOURCE_ERROR_OUT_OF_MEMORY);
		return APP_RESOURCE_ERROR_OUT_OF_MEMORY;
	}

	snprintf(buf, 1023, "%sres.xml", res_path);
	retval = pkgmgrinfo_resource_open(buf, &(rsc_manager->data));
	if (retval != PMINFO_R_OK) {
		LOGE("IO_ERROR(0x%08x), failed to get db for resource manager",
				APP_RESOURCE_ERROR_IO_ERROR);
		free(rsc_manager);
		return APP_RESOURCE_ERROR_IO_ERROR;
	}

	rsc_manager->cache = g_hash_table_new(g_str_hash, g_str_equal);
	*handle = rsc_manager;

	return APP_RESOURCE_ERROR_NONE;
}

static void __invalidate_cache()
{
	if (resource_handle != NULL) {
		if (resource_handle->cache != NULL) {
			GHashTableIter iter;
			gpointer key, value;

			g_hash_table_iter_init(&iter, resource_handle->cache);
			while (g_hash_table_iter_next(&iter, &key, &value)) {
				free(key);
				resource_cache_context_t *c = (resource_cache_context_t *) value;
				free(c->output);
				free(value);
			}
			g_hash_table_remove_all(resource_handle->cache);
			if (cur_language) {
				free(cur_language);
				cur_language = NULL;
			}
		}
	}
}

static int __close(resource_manager_t *handle)
{
	if (handle == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x), resource_manager",
				APP_RESOURCE_ERROR_INVALID_PARAMETER);
		return APP_RESOURCE_ERROR_INVALID_PARAMETER;
	}

	__invalidate_cache();
	if (handle->cache != NULL) {
		g_hash_table_destroy(handle->cache);
	}

	if (handle->data != NULL) {
		pkgmgrinfo_resource_close(handle->data);
	}

	free(handle);

	return APP_RESOURCE_ERROR_NONE;
}

static void __vconf_cb(keynode_t *key, void *data)
{
	__invalidate_cache();
}

int app_resource_manager_init()
{
	if (resource_handle == NULL) {
		int retval = APP_RESOURCE_ERROR_NONE;

		res_path = aul_get_app_resource_path();
		if (res_path == NULL) {
			LOGE("IO_ERROR(0x%08x), failed to get resource path",
					APP_RESOURCE_ERROR_IO_ERROR);
			return APP_RESOURCE_ERROR_IO_ERROR;
		}

		retval = __open(&resource_handle);
		if (retval != APP_RESOURCE_ERROR_NONE) {
			LOGE("IO_ERROR(0x%08x), failed to get resource_handle(%d)",
					APP_RESOURCE_ERROR_IO_ERROR, retval);
			return APP_RESOURCE_ERROR_IO_ERROR;
		}

		if (attr_key == NULL) {
			attr_key = g_hash_table_new(g_str_hash, g_str_equal);

			if (attr_key == NULL)
				return APP_RESOURCE_ERROR_OUT_OF_MEMORY;

			unsigned int i;
			for (i = 0; i < ARRAY_SIZE(map); i++) {
				g_hash_table_insert(attr_key, (char *) map[i].bundle_attr_key,
						(gpointer) (map[i].bundle_attr_value));
			}
		}

		int r = vconf_notify_key_changed(VCONFKEY_LANGSET, __vconf_cb, NULL);

		if (r < 0) {
			LOGE("IO_ERROR(0x%08x), failed to register vconf(%d)",
					APP_RESOURCE_ERROR_IO_ERROR, r);
			return APP_RESOURCE_ERROR_IO_ERROR;
		}

	}

	return APP_RESOURCE_ERROR_NONE;
}

int app_resource_manager_get(app_resource_e type, const char *id, char **path)
{
	int retval = APP_RESOURCE_ERROR_NONE;
	char *put_fname = NULL;
	const char *cached_path = NULL;
	GList *list = NULL;
	resource_group_t *resource_group = NULL;
	resource_node_t *resource_node = NULL;

	*path = NULL;

	if (id == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x), resource_data_t",
				APP_RESOURCE_ERROR_INVALID_PARAMETER);
		return APP_RESOURCE_ERROR_INVALID_PARAMETER;
	}

	if (type < APP_RESOURCE_TYPE_MIN || type > APP_RESOURCE_TYPE_MAX) {
		LOGE("INVALID_PARAMETER(0x%08x), type(%d)",
				APP_RESOURCE_ERROR_INVALID_PARAMETER, type);
		return APP_RESOURCE_ERROR_INVALID_PARAMETER;
	}

	if (resource_handle == NULL) {
		retval = app_resource_manager_init();
		if (retval != APP_RESOURCE_ERROR_NONE)
			return retval;
	}

	/* To get fname from cache */
	cached_path = __get_cache(type, id);
	if (cached_path != NULL) {
		*path = strdup(cached_path);
		return APP_RESOURCE_ERROR_NONE;
	} else { /* can't find fname from cache */
		if (resource_handle == NULL)
			return APP_RESOURCE_ERROR_IO_ERROR;

		resource_group = __find_group(resource_handle->data, type);
		if (resource_group == NULL) {
			LOGE("IO_ERROR(0x%08x), failed to get resource_group",
					APP_RESOURCE_ERROR_IO_ERROR);
			retval = APP_RESOURCE_ERROR_IO_ERROR;
			goto Exception;
		}

		list = __get_valid_nodes(resource_group, id);
		if (list == NULL) {
			retval = APP_RESOURCE_ERROR_IO_ERROR;
			goto Exception;
		}

		resource_node = __get_best_node(list);
		if (resource_node == NULL) {
			retval = APP_RESOURCE_ERROR_IO_ERROR;
			goto Exception;
		} else {
			unsigned int total_len = strlen(res_path)
					+ strlen(resource_node->folder) + strlen(id) + 3;
			put_fname = (char *) calloc(1, total_len);
			snprintf(put_fname, total_len, "%s%s/%s", res_path,
					resource_node->folder, id);
			*path = strdup(put_fname);
		}
		__put_cache(type, id, put_fname);
	}

Exception:
	if (list != NULL)
		g_list_free(list);

	if (put_fname == NULL && resource_group != NULL) {
		char path_buf[MAX_PATH] = { 0, };

		snprintf(path_buf, MAX_PATH - 1, "%s%s/%s", res_path,
				resource_group->folder, id);
		if (access(path_buf, R_OK) == 0) {
			__put_cache(type, id, path_buf);
			*path = strdup(path_buf);
			retval = APP_RESOURCE_ERROR_NONE;
		}
	}

	if (put_fname != NULL)
		free(put_fname);

	return retval;
}

int app_resource_manager_release()
{
	if (resource_handle != NULL) {
		__close(resource_handle);
		resource_handle = NULL;
	}

	if (attr_key != NULL) {
		g_hash_table_destroy(attr_key);
		attr_key = NULL;
	}

	if (cur_language) {
		free(cur_language);
		cur_language = NULL;
	}

	vconf_ignore_key_changed(VCONFKEY_LANGSET, __vconf_cb);
	return APP_RESOURCE_ERROR_NONE;
}

