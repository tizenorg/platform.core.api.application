/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
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

#include <sys/types.h>
#include <sys/inotify.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <linux/version.h>
#include <stdlib.h>
#include <pthread.h>
#include <glib.h>

#include <app_preference.h>
#include <app_preference_internal.h>

#include <glib.h>

#define INOTY_EVENT_MASK   (IN_CLOSE_WRITE | IN_DELETE_SELF)

/* inotify */
struct noti_node {
	int wd;
	char *keyname;
	preference_changed_cb cb;
	void *cb_data;
	struct noti_node *next;
};
typedef struct noti_node noti_node_s;
static GList *g_notilist;

static int _preference_inoti_comp_with_wd(gconstpointer a, gconstpointer b)
{
	int r;

	noti_node_s *key1 = (noti_node_s *) a;
	noti_node_s *key2 = (noti_node_s *) b;

	r = key1->wd - key2->wd;
	return r;
}

static int _kdb_inoti_fd;

static pthread_mutex_t _kdb_inoti_fd_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _kdb_g_ns_mutex = PTHREAD_MUTEX_INITIALIZER;

static GSource *_kdb_handler;

static GList* _preference_copy_noti_list(GList *orig_notilist)
{
	GList *copy_notilist = NULL;
	struct noti_node *n = NULL;
	struct noti_node *t = NULL;

	if (!orig_notilist)
		return NULL;

	orig_notilist = g_list_first(orig_notilist);
	if (!orig_notilist)
		return NULL;

	while (orig_notilist) {
		do {
			t = orig_notilist->data;

			if (t == NULL) {
				WARN("noti item data is null");
				break;
			}

			if ((t->keyname == NULL) || (strlen(t->keyname) == 0)) {
				WARN("noti item data key name is null");
				break;
			}

			n = calloc(1, sizeof(noti_node_s));
			if (n == NULL) {
				ERR("_preference_copy_noti_list : calloc failed. memory full");
				break;
			}

			n->keyname = strndup(t->keyname, PREFERENCE_KEY_PATH_LEN);
			if (n->keyname == NULL)	{
				char err_buf[ERR_LEN] = {0,};
				strerror_r(errno, err_buf, sizeof(err_buf));
				ERR("The memory is insufficient, errno: %d (%s)", errno, err_buf);
				free(n);
				break;
			}
			n->wd = t->wd;
			n->cb_data = t->cb_data;
			n->cb = t->cb;

			copy_notilist = g_list_append(copy_notilist, n);
		} while (0);

		orig_notilist = g_list_next(orig_notilist);
	}
	return copy_notilist;
}

static void _preference_free_noti_node(gpointer data)
{
	struct noti_node *n = (struct noti_node*)data;
	g_free(n->keyname);
	g_free(n);
}

static void _preference_free_noti_list(GList *noti_list)
{
	g_list_free_full(noti_list, _preference_free_noti_node);
}


static gboolean _preference_kdb_gio_cb(GIOChannel *src, GIOCondition cond, gpointer data)
{
	int fd, r, res;
	struct inotify_event ie;
	GList *l_notilist = NULL;

	fd = g_io_channel_unix_get_fd(src);
	r = read(fd, &ie, sizeof(ie));

	while (r > 0) {
		if (ie.mask & INOTY_EVENT_MASK) {

			INFO("read event from GIOChannel. wd : %d", ie.wd);

			pthread_mutex_lock(&_kdb_g_ns_mutex);
			l_notilist = _preference_copy_noti_list(g_notilist);
			pthread_mutex_unlock(&_kdb_g_ns_mutex);

			if (l_notilist) {

				struct noti_node *t = NULL;
				GList *noti_list = NULL;

				noti_list = g_list_first(l_notilist);

				while (noti_list) {
					t = noti_list->data;

					keynode_t* keynode = _preference_keynode_new();
					if (keynode == NULL) {
						ERR("key malloc fail");
						break;
					}

					if ( (t) && (t->wd == ie.wd) && (t->keyname) ) {

						res = _preference_keynode_set_keyname(keynode, t->keyname);
						if (res != PREFERENCE_ERROR_NONE) {
							ERR("_preference_keynode_set_keyname() failed(%d)", res);
							goto out_func;
						}

						if ((ie.mask & IN_DELETE_SELF))
						{
							res = _preference_kdb_del_notify(keynode);
							if (res != PREFERENCE_ERROR_NONE)
								ERR("_preference_kdb_del_notify() failed(%d)", res);
						}
						else
						{
							res = _preference_get_key(keynode);
							if (res != PREFERENCE_ERROR_NONE)
								ERR("_preference_get_key() failed(%d)", res);

							INFO("key(%s) is changed. cb(%p) called", t->keyname, t->cb);
							t->cb(t->keyname, t->cb_data);
						}
					}
					else if ( (t) && (t->keyname == NULL) ) { /* for debugging */
						ERR("preference keyname is null.");
					}
out_func:
					_preference_keynode_free(keynode);

					noti_list = g_list_next(noti_list);
				}

				_preference_free_noti_list(l_notilist);
			}
		}

		if (ie.len > 0)
			(void) lseek(fd, ie.len, SEEK_CUR);

		r = read(fd, &ie, sizeof(ie));
	}
	return TRUE;
}

static int _preference_kdb_noti_init(void)
{
	GIOChannel *gio;
	int ret = 0;

	pthread_mutex_lock(&_kdb_inoti_fd_mutex);

	if (0 < _kdb_inoti_fd) {
		ERR("Error: invalid _kdb_inoti_fd");
		pthread_mutex_unlock(&_kdb_inoti_fd_mutex);
		return PREFERENCE_ERROR_IO_ERROR;
	}
	_kdb_inoti_fd = inotify_init();
	if (_kdb_inoti_fd == -1) {
		char err_buf[100] = { 0, };
		strerror_r(errno, err_buf, sizeof(err_buf));
		ERR("inotify init: %s", err_buf);
		pthread_mutex_unlock(&_kdb_inoti_fd_mutex);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	ret = fcntl(_kdb_inoti_fd, F_SETFD, FD_CLOEXEC);
	if (ret < 0) {
		char err_buf[100] = { 0, };
		strerror_r(errno, err_buf, sizeof(err_buf));
		ERR("inotify init: %s", err_buf);
		pthread_mutex_unlock(&_kdb_inoti_fd_mutex);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	ret = fcntl(_kdb_inoti_fd, F_SETFL, O_NONBLOCK);
	if (ret < 0) {
		char err_buf[100] = { 0, };
		strerror_r(errno, err_buf, sizeof(err_buf));
		ERR("inotify init: %s", err_buf);
		pthread_mutex_unlock(&_kdb_inoti_fd_mutex);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	pthread_mutex_unlock(&_kdb_inoti_fd_mutex);

	gio = g_io_channel_unix_new(_kdb_inoti_fd);
	retvm_if(gio == NULL, -1, "Error: create a new GIOChannel");

	g_io_channel_set_flags(gio, G_IO_FLAG_NONBLOCK, NULL);

	_kdb_handler = g_io_create_watch(gio, G_IO_IN);
	g_source_set_callback(_kdb_handler, (GSourceFunc) _preference_kdb_gio_cb, NULL, NULL);
	g_source_attach(_kdb_handler, NULL);
	g_io_channel_unref(gio);
	g_source_unref(_kdb_handler);

	return PREFERENCE_ERROR_NONE;
}

int _preference_kdb_add_notify(keynode_t *keynode, preference_changed_cb cb, void *data)
{
	char path[PATH_MAX];
	int wd;
	struct noti_node t, *n, *node;
	char err_buf[ERR_LEN] = { 0, };
	int ret = 0;
	GList *list = NULL;
	int func_ret = PREFERENCE_ERROR_NONE;
	char *keyname = keynode->keyname;

	retvm_if((keyname == NULL || cb == NULL), PREFERENCE_ERROR_INVALID_PARAMETER,
			"_preference_kdb_add_notify : Invalid argument - keyname(%s) cb(%p)",
			keyname, cb);

	if (_kdb_inoti_fd <= 0)
		if (_preference_kdb_noti_init())
			return PREFERENCE_ERROR_IO_ERROR;

	ret = _preference_get_key_path(keynode, path);
	if (ret != PREFERENCE_ERROR_NONE) {
		ERR("Invalid argument: key is not valid");
		return PREFERENCE_ERROR_INVALID_PARAMETER;
	}

	if (0 != access(path, F_OK)) {
		if (errno == ENOENT) {
			ERR("_preference_kdb_add_notify : Key(%s) does not exist", keyname);
			return PREFERENCE_ERROR_IO_ERROR;
		}
	}

	wd = inotify_add_watch(_kdb_inoti_fd, path, INOTY_EVENT_MASK);
	if (wd == -1) {
		strerror_r(errno, err_buf, sizeof(err_buf));
		ERR("_preference_kdb_add_notify : add noti(%s)", err_buf);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	t.wd = wd;

	pthread_mutex_lock(&_kdb_g_ns_mutex);

	list = g_list_find_custom(g_notilist, &t, (GCompareFunc)_preference_inoti_comp_with_wd);
	if (list) {
		WARN("_preference_kdb_add_notify : key(%s) change callback(%p)", keyname, cb);

		node = list->data;
		node->wd = wd;
		node->cb_data = data;
		node->cb = cb;

		goto out_func;
	}

	n = calloc(1, sizeof(noti_node_s));
	if (n == NULL) {
		strerror_r(errno, err_buf, sizeof(err_buf));
		ERR("_preference_kdb_add_notify : add noti(%s)", err_buf);
		func_ret = PREFERENCE_ERROR_IO_ERROR;
		goto out_func;
	}

	n->keyname = strndup(keyname, PREFERENCE_KEY_PATH_LEN);
	if (n->keyname == NULL) {
		strerror_r(errno, err_buf, sizeof(err_buf));
		ERR("The memory is insufficient, errno: %d (%s)", errno, err_buf);
		free(n);
		goto out_func;
	}
	n->wd = wd;
	n->cb_data = data;
	n->cb = cb;

	g_notilist = g_list_append(g_notilist, n);
	if (!g_notilist) {
		ERR("g_list_append fail");
	}

	INFO("cb(%p) is added for %s. tot cb cnt : %d\n", cb, n->keyname, g_list_length(g_notilist));

out_func:
	pthread_mutex_unlock(&_kdb_g_ns_mutex);

	return func_ret;
}

int _preference_kdb_del_notify(keynode_t *keynode)
{
	int wd = 0;
	int r = 0;
	struct noti_node *n = NULL;
	struct noti_node t;
	char path[PATH_MAX] = { 0, };
	char err_buf[ERR_LEN] = { 0, };
	int del = 0;
	int ret = 0;
	char *keyname = keynode->keyname;
	int func_ret = PREFERENCE_ERROR_NONE;
	GList *noti_list;

	retvm_if(keyname == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: keyname(%s)", keyname);

	ret = _preference_get_key_path(keynode, path);
	if (ret != PREFERENCE_ERROR_NONE) {
		ERR("Invalid argument: key is not valid");
		return PREFERENCE_ERROR_INVALID_PARAMETER;
	}

	if (0 != access(path, F_OK)) {
		if (errno == ENOENT) {
			ERR("_preference_kdb_del_notify : Key(%s) does not exist", keyname);
			return PREFERENCE_ERROR_IO_ERROR;
		}
	}

	retvm_if(_kdb_inoti_fd == 0, PREFERENCE_ERROR_NONE, "Invalid operation: not exist anything for inotify");

	/* get wd */
	wd = inotify_add_watch(_kdb_inoti_fd, path, INOTY_EVENT_MASK);
	if (wd == -1) {
		strerror_r(errno, err_buf, sizeof(err_buf));
		ERR("Error: inotify_add_watch() [%s]: %s", path, err_buf);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	pthread_mutex_lock(&_kdb_g_ns_mutex);

	t.wd = wd;

	noti_list = g_list_find_custom(g_notilist, &t, (GCompareFunc)_preference_inoti_comp_with_wd);
	if(noti_list) {
		del++;

		n = noti_list->data;
		g_notilist = g_list_remove(g_notilist, n);
		g_free(n->keyname);
		g_free(n);

		r = inotify_rm_watch(_kdb_inoti_fd, wd);
		if(r == -1) {
			strerror_r(errno, err_buf, sizeof(err_buf));
			ERR("Error: inotify_rm_watch [%s]: %s", keyname, err_buf);
			func_ret = PREFERENCE_ERROR_IO_ERROR;
		}

		INFO("key(%s) cb is removed. remained noti list total length(%d)",
				keyname, g_list_length(g_notilist));
	}

	if(g_list_length(g_notilist) == 0) {
		close(_kdb_inoti_fd);
		_kdb_inoti_fd = 0;

		g_source_destroy(_kdb_handler);
		_kdb_handler = NULL;

		g_list_free(g_notilist);
		g_notilist = NULL;

		INFO("all noti list is freed");
	}

	pthread_mutex_unlock(&_kdb_g_ns_mutex);

	if(del == 0) {
		errno = ENOENT;
		func_ret = PREFERENCE_ERROR_IO_ERROR;
	}

	return func_ret;
}
