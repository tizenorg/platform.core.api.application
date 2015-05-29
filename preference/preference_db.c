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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>

#include <app_internal.h>

#include <app_preference.h>
#include <app_preference_internal.h>

#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "CAPI_APPFW_APPLICATION_PREFERENCE"
#define DBG_MODE (1)

static sqlite3 *pref_db = NULL;
static bool is_update_hook_registered = false;
static pref_changed_cb_node_t *head = NULL;

static void _finish(void *data)
{
	if (pref_db != NULL)
	{
		sqlite3_close(pref_db);
		pref_db = NULL;
	}
}

static int _busy_handler(void *pData, int count)
{
	if(5 - count > 0) {
		LOGD("Busy Handler Called! : PID(%d) / CNT(%d)\n", getpid(), count+1);
		usleep((count+1)*100000);
		return 1;
	} else {
		LOGD("Busy Handler will be returned SQLITE_BUSY error : PID(%d) \n", getpid());
		return 0;
	}
}

static int _initialize(void)
{
	char *data_path = NULL;
	char db_path[TIZEN_PATH_MAX] = {0, };
	int ret;
	char *errmsg;

	if ((data_path = app_get_data_path()) == NULL)
	{
		LOGE("IO_ERROR(0x%08x) : fail to get data directory", PREFERENCE_ERROR_IO_ERROR);
		return PREFERENCE_ERROR_IO_ERROR;
	}
	snprintf(db_path, sizeof(db_path), "%s/%s", data_path, PREF_DB_NAME);
	free(data_path);

	ret = sqlite3_open(db_path, &pref_db);
	if (ret != SQLITE_OK)
	{
		LOGE("IO_ERROR(0x%08x) : fail to open db(%s)", PREFERENCE_ERROR_IO_ERROR, sqlite3_errmsg(pref_db));
		pref_db = NULL;
		return PREFERENCE_ERROR_IO_ERROR;
	}

	ret = sqlite3_busy_handler(pref_db, _busy_handler, NULL);
	if (ret != SQLITE_OK) {
		LOGW("IO_ERROR(0x%08x) : fail to register busy handler(%s)\n", PREFERENCE_ERROR_IO_ERROR, sqlite3_errmsg(pref_db));
	}

	ret = sqlite3_exec(pref_db, "CREATE TABLE IF NOT EXISTS pref ( pref_key TEXT PRIMARY KEY, pref_type TEXT, pref_data TEXT)",
	               NULL, NULL, &errmsg);
	if (ret != SQLITE_OK)
	{
		LOGE("IO_ERROR(0x%08x) : fail to create db table(%s)", PREFERENCE_ERROR_IO_ERROR, errmsg);
		sqlite3_free(errmsg);
		sqlite3_close(pref_db);
		pref_db = NULL;
		return PREFERENCE_ERROR_IO_ERROR;
	}

	app_finalizer_add(_finish, NULL);

	return PREFERENCE_ERROR_NONE;
}

static int _write_data(const char *key, const char *type, const char *data)
{
	int ret;
	char *buf = NULL;
	bool exist = false;
	sqlite3_stmt *stmt;

	if (key == NULL || key[0] == '\0'  || data == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", PREFERENCE_ERROR_INVALID_PARAMETER);
		return PREFERENCE_ERROR_INVALID_PARAMETER;
	}

	/* insert data or update data if data already exist */
	ret = preference_is_existing(key, &exist);
	if (ret != PREFERENCE_ERROR_NONE)
	{
		return ret;
	}

	// to use sqlite3_update_hook, we have to use INSERT/UPDATE operation instead of REPLACE operation
	if (exist)
	{
		buf = sqlite3_mprintf("UPDATE %s SET %s=?, %s=? WHERE %s=?;",
								PREF_TBL_NAME, PREF_F_TYPE_NAME, PREF_F_DATA_NAME, PREF_F_KEY_NAME);
	}
	else
	{
		buf = sqlite3_mprintf("INSERT INTO %s (%s, %s, %s) values (?, ?, ?);",
								PREF_TBL_NAME, PREF_F_KEY_NAME, PREF_F_TYPE_NAME, PREF_F_DATA_NAME);
	}

	if (buf == NULL)
	{
		LOGE("IO_ERROR(0x%08x) : fail to create query string", PREFERENCE_ERROR_IO_ERROR);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	ret = sqlite3_prepare(pref_db, buf, strlen(buf), &stmt, NULL);
	if (ret != SQLITE_OK) {
		LOGE("IO_ERROR(0x%08x) : fail to prepare query (%d/%s)",
			PREFERENCE_ERROR_IO_ERROR,
			sqlite3_extended_errcode(pref_db),
			sqlite3_errmsg(pref_db));
		return PREFERENCE_ERROR_IO_ERROR;
	}

	if(exist)
	{
		ret = sqlite3_bind_text(stmt, 1, type, strlen(type), SQLITE_STATIC);
		if(ret != SQLITE_OK) {
			LOGE("IO_ERROR(0x%08x) : fail to bind(1) query (%d/%s)",
				PREFERENCE_ERROR_IO_ERROR,
				sqlite3_extended_errcode(pref_db),
				sqlite3_errmsg(pref_db));
			sqlite3_finalize(stmt);
			return PREFERENCE_ERROR_IO_ERROR;
		}
		ret = sqlite3_bind_text(stmt, 2, data, strlen(data), SQLITE_STATIC);
		if(ret != SQLITE_OK) {
			LOGE("IO_ERROR(0x%08x) : fail to bind(2) query (%d/%s)",
				PREFERENCE_ERROR_IO_ERROR,
				sqlite3_extended_errcode(pref_db),
				sqlite3_errmsg(pref_db));
			sqlite3_finalize(stmt);
			return PREFERENCE_ERROR_IO_ERROR;
		}
		ret = sqlite3_bind_text(stmt, 3, key, strlen(key), SQLITE_STATIC);
		if(ret != SQLITE_OK) {
			LOGE("IO_ERROR(0x%08x) : fail to bind(3) query (%d/%s)",
				PREFERENCE_ERROR_IO_ERROR,
				sqlite3_extended_errcode(pref_db),
				sqlite3_errmsg(pref_db));
			sqlite3_finalize(stmt);
			return PREFERENCE_ERROR_IO_ERROR;
		}
	}
	else
	{
		ret = sqlite3_bind_text(stmt, 1, key, strlen(key), SQLITE_STATIC);
		if(ret != SQLITE_OK) {
			LOGE("IO_ERROR(0x%08x) : fail to bind(1) query (%d/%s)",
				PREFERENCE_ERROR_IO_ERROR,
				sqlite3_extended_errcode(pref_db),
				sqlite3_errmsg(pref_db));
			sqlite3_finalize(stmt);
			return PREFERENCE_ERROR_IO_ERROR;
		}
		ret = sqlite3_bind_text(stmt, 2, type, strlen(type), SQLITE_STATIC);
		if(ret != SQLITE_OK) {
			LOGE("IO_ERROR(0x%08x) : fail to bind(2) query (%d/%s)",
				PREFERENCE_ERROR_IO_ERROR,
				sqlite3_extended_errcode(pref_db),
				sqlite3_errmsg(pref_db));
			sqlite3_finalize(stmt);
			return PREFERENCE_ERROR_IO_ERROR;
		}
		ret = sqlite3_bind_text(stmt, 3, data, strlen(data), SQLITE_STATIC);
		if(ret != SQLITE_OK) {
			LOGE("IO_ERROR(0x%08x) : fail to bind(3) query (%d/%s)",
				PREFERENCE_ERROR_IO_ERROR,
				sqlite3_extended_errcode(pref_db),
				sqlite3_errmsg(pref_db));
			sqlite3_finalize(stmt);
			return PREFERENCE_ERROR_IO_ERROR;
		}
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_DONE) {
		LOGE("IO_ERROR(0x%08x): fail to write data(%d/%s)",
			PREFERENCE_ERROR_IO_ERROR,
			sqlite3_extended_errcode(pref_db),
			sqlite3_errmsg(pref_db));
		sqlite3_finalize(stmt);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	sqlite3_finalize(stmt);
	if(buf) {
		sqlite3_free(buf);
		buf = NULL;
	}

	return PREFERENCE_ERROR_NONE;
}

//static int _read_data(const char *key, preference_type_e *type, char *data)
static int _read_data(const char *key, char *type, char *data)
{
	int ret;
	char *buf;
	char **result;
	int rows;
	int columns;
	char *errmsg;

	if (key == NULL || key[0] == '\0'  || data == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", PREFERENCE_ERROR_INVALID_PARAMETER);
		return PREFERENCE_ERROR_INVALID_PARAMETER;
	}

	if (pref_db == NULL)
	{
		if (_initialize() != PREFERENCE_ERROR_NONE)
		{
			LOGE("IO_ERROR(0x%08x) : fail to initialize db", PREFERENCE_ERROR_IO_ERROR);
			return PREFERENCE_ERROR_IO_ERROR;
		}
	}

	buf = sqlite3_mprintf("SELECT %s, %s, %s FROM %s WHERE %s=%Q;",
							PREF_F_KEY_NAME, PREF_F_TYPE_NAME, PREF_F_DATA_NAME, PREF_TBL_NAME, PREF_F_KEY_NAME, key);

	if (buf == NULL)
	{
		LOGE("IO_ERROR(0x%08x) : fail to create query string", PREFERENCE_ERROR_IO_ERROR);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	ret = sqlite3_get_table(pref_db, buf, &result, &rows, &columns, &errmsg);
	sqlite3_free(buf);
	if (ret != SQLITE_OK)
	{
		LOGE("IO_ERROR(0x%08x) : fail to read data (%s)", PREFERENCE_ERROR_IO_ERROR, errmsg);
		sqlite3_free(errmsg);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	if (rows == 0)
	{
		LOGE("NO_KEY(0x%08x) : fail to find given key(%s)", PREFERENCE_ERROR_NO_KEY, key);
		sqlite3_free_table(result);
		return PREFERENCE_ERROR_NO_KEY;
	}

	snprintf(type, 2, "%s", result[4]);			// get type value
	snprintf(data, BUF_LEN, "%s", result[5]);			// get data value

	sqlite3_free_table(result);

	return PREFERENCE_ERROR_NONE;
}


int preference_set_int(const char *key, int value)
{
	char type[2];
	char data[BUF_LEN];
	snprintf(type, 2, "%d", PREFERENCE_TYPE_INT);
	snprintf(data, BUF_LEN, "%d", value);
	return _write_data(key, type, data);
}

int preference_get_int(const char *key, int *value)
{
	char type[2];
	char data[BUF_LEN];
	int ret;

	if (value == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", PREFERENCE_ERROR_INVALID_PARAMETER);
		return PREFERENCE_ERROR_INVALID_PARAMETER;
	}

	ret = _read_data(key, type, data);
	if (ret == PREFERENCE_ERROR_NONE)
	{
		if (atoi(type) == PREFERENCE_TYPE_INT)
		{
			*value = atoi(data);
		}
		else
		{
			LOGE("INVALID_PARAMETER(0x%08x) : param type(%d)", PREFERENCE_ERROR_INVALID_PARAMETER, atoi(type));
			return PREFERENCE_ERROR_INVALID_PARAMETER;
		}
	}

	return ret;
}

int preference_set_double(const char *key, double value)
{
	char type[2];
	char data[BUF_LEN];

	locale_t loc = newlocale(LC_NUMERIC_MASK, "C", NULL);
	uselocale(loc);

	snprintf(type, 2, "%d", PREFERENCE_TYPE_DOUBLE);
	snprintf(data, BUF_LEN, "%f", value);

	freelocale(loc);
	uselocale(LC_GLOBAL_LOCALE);

	return _write_data(key, type, data);
}

int preference_get_double(const char *key, double *value)
{
	char type[2];
	char data[BUF_LEN];

	int ret;

	if (value == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", PREFERENCE_ERROR_INVALID_PARAMETER);
		return PREFERENCE_ERROR_INVALID_PARAMETER;
	}

	ret = _read_data(key, type, data);
	if (ret == PREFERENCE_ERROR_NONE)
	{
		if (atoi(type) == PREFERENCE_TYPE_DOUBLE)
		{
			locale_t loc = newlocale(LC_NUMERIC_MASK, "C", NULL);
			uselocale(loc);

			*value = atof(data);

			freelocale(loc);
			uselocale(LC_GLOBAL_LOCALE);
		}
		else
		{
			LOGE("INVALID_PARAMETER(0x%08x) : param type(%d)", PREFERENCE_ERROR_INVALID_PARAMETER, atoi(type));
			return PREFERENCE_ERROR_INVALID_PARAMETER;
		}
	}

	return ret;
}

int preference_set_string(const char *key, const char *value)
{
	char type[2];

	snprintf(type, 2, "%d", PREFERENCE_TYPE_STRING);
	if (strlen(value) > (BUF_LEN-1))
	{
		LOGE("INVALID_PARAMETER(0x%08x) : param type(%d)", PREFERENCE_ERROR_INVALID_PARAMETER, atoi(type));
		return PREFERENCE_ERROR_INVALID_PARAMETER;
	}
	return _write_data(key, type, value);
}

int preference_get_string(const char *key, char **value)
{
	char type[2];
	char data[BUF_LEN];

	int ret;

	if (value == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", PREFERENCE_ERROR_INVALID_PARAMETER);
		return PREFERENCE_ERROR_INVALID_PARAMETER;
	}

	ret = _read_data(key, type, data);
	if (ret == PREFERENCE_ERROR_NONE)
	{
		if (atoi(type) == PREFERENCE_TYPE_STRING)
		{
			*value = strdup(data);
			if (value == NULL)
			{
				LOGE("OUT_OF_MEMORY(0x%08x)", PREFERENCE_ERROR_OUT_OF_MEMORY);
				return PREFERENCE_ERROR_OUT_OF_MEMORY;
			}
		}
		else
		{
			LOGE("INVALID_PARAMETER(0x%08x) : param type(%d)", PREFERENCE_ERROR_INVALID_PARAMETER, atoi(type));
			return PREFERENCE_ERROR_INVALID_PARAMETER;
		}
	}

	return ret;
}

int preference_set_boolean(const char *key, bool value)
{
	char type[2];
	char data[BUF_LEN];
	snprintf(type, 2, "%d", PREFERENCE_TYPE_BOOLEAN);
	snprintf(data, BUF_LEN, "%d", value);
	return _write_data(key, type, data);
}

int preference_get_boolean(const char *key, bool *value)
{
	char type[2];
	char data[BUF_LEN];

	int ret;

	if (value == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", PREFERENCE_ERROR_INVALID_PARAMETER);
		return PREFERENCE_ERROR_INVALID_PARAMETER;
	}

	ret = _read_data(key, type, data);
	if (ret == PREFERENCE_ERROR_NONE)
	{
		if (atoi(type) == PREFERENCE_TYPE_BOOLEAN)
		{
			*value = (bool)atoi(data);
		}
		else
		{
			LOGE("INVALID_PARAMETER(0x%08x) : param type(%d)", PREFERENCE_ERROR_INVALID_PARAMETER, atoi(type));
			return PREFERENCE_ERROR_INVALID_PARAMETER;
		}
	}

	return ret;
}


// TODO: below operation is too heavy, let's find the light way to check.
int preference_is_existing(const char *key, bool *exist)
{
	int ret;
	char *buf;
	char **result;
	int rows;
	int columns;
	char *errmsg;

	if (key == NULL  || key[0] == '\0'  || exist == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", PREFERENCE_ERROR_INVALID_PARAMETER);
		return PREFERENCE_ERROR_INVALID_PARAMETER;
	}

	if (pref_db == NULL)
	{
		if (_initialize() != PREFERENCE_ERROR_NONE)
		{
			LOGE("IO_ERROR(0x%08x) : fail to initialize db", PREFERENCE_ERROR_IO_ERROR);
			return PREFERENCE_ERROR_IO_ERROR;
		}
	}

	/* check data is exist */
	buf = sqlite3_mprintf("SELECT %s FROM %s WHERE %s=%Q;", PREF_F_KEY_NAME, PREF_TBL_NAME, PREF_F_KEY_NAME, key);

	if (buf == NULL)
	{
		LOGE("IO_ERROR(0x%08x) : fail to create query string", PREFERENCE_ERROR_IO_ERROR);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	ret = sqlite3_get_table(pref_db, buf, &result, &rows, &columns, &errmsg);
	sqlite3_free(buf);
	if (ret != SQLITE_OK)
	{
		LOGE("IO_ERROR(0x%08x) : fail to read data(%s)", PREFERENCE_ERROR_IO_ERROR, errmsg);
		sqlite3_free(errmsg);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	if (rows > 0)
	{
		*exist = true;
	}
	else
	{
		*exist = false;
	}

	sqlite3_free_table(result);
	return PREFERENCE_ERROR_NONE;
}

static pref_changed_cb_node_t* _find_node(const char *key)
{
	pref_changed_cb_node_t *tmp_node;

	if (key == NULL || key[0] == '\0' )
	{
		return NULL;
	}

	tmp_node = head;

	while (tmp_node)
	{
		if (strcmp(tmp_node->key, key) == 0)
		{
			break;
		}
		tmp_node = tmp_node->next;
	}

	return tmp_node;
}


static int _add_node(const char *key, preference_changed_cb cb, void *user_data)
{
	pref_changed_cb_node_t *tmp_node;

	if (key == NULL  || key[0] == '\0'  || cb == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", PREFERENCE_ERROR_INVALID_PARAMETER);
		return PREFERENCE_ERROR_INVALID_PARAMETER;
	}

	tmp_node = _find_node(key);

	if (tmp_node != NULL)
	{
		tmp_node->cb = cb;
		tmp_node->user_data = user_data;
	}
	else
	{
		tmp_node = (pref_changed_cb_node_t*)malloc(sizeof(pref_changed_cb_node_t));
		if (tmp_node == NULL)
		{
			LOGE("OUT_OF_MEMORY(0x%08x)", PREFERENCE_ERROR_OUT_OF_MEMORY);
			return PREFERENCE_ERROR_OUT_OF_MEMORY;
		}

		tmp_node->key = strdup(key);
		if (tmp_node->key == NULL)
		{
			free(tmp_node);
			LOGE("OUT_OF_MEMORY(0x%08x)", PREFERENCE_ERROR_OUT_OF_MEMORY);
			return PREFERENCE_ERROR_OUT_OF_MEMORY;
		}

		if (head != NULL)
			head->prev = tmp_node;
		tmp_node->cb = cb;
		tmp_node->user_data = user_data;
		tmp_node->prev = NULL;
		tmp_node->next = head;
		head = tmp_node;
	}

	return PREFERENCE_ERROR_NONE;
}

static int _remove_node(const char *key)
{
	pref_changed_cb_node_t *tmp_node;

	if (key == NULL || key[0] == '\0' )
	{
		LOGE("INVALID_PARAMETER(0x%08x)", PREFERENCE_ERROR_INVALID_PARAMETER);
		return PREFERENCE_ERROR_INVALID_PARAMETER;
	}

	tmp_node = _find_node(key);

	if (tmp_node == NULL)
	{
		return PREFERENCE_ERROR_NONE;
	}

	if (tmp_node->prev != NULL)
	{
		tmp_node->prev->next = tmp_node->next;
	}
	else
	{
		head = tmp_node->next;
	}

	if (tmp_node->next != NULL)
	{
		tmp_node->next->prev = tmp_node->prev;
	}

	if (tmp_node->key)
	{
		free(tmp_node->key);
	}

	free(tmp_node);

	return PREFERENCE_ERROR_NONE;
}


static void _remove_all_node(void)
{
	pref_changed_cb_node_t *tmp_node;

	while (head)
	{
		tmp_node = head;
		head = tmp_node->next;

		if (tmp_node->key)
		{
			free(tmp_node->key);
		}

		free(tmp_node);
	}
}


static void _update_cb(void *data, int action, char const *db_name, char const *table_name, sqlite_int64 rowid)
{
	int ret;
	char *buf;
	char **result;
	int rows;
	int columns;
	char *errmsg;
	pref_changed_cb_node_t *tmp_node;

	// skip INSERT/DELETE event
	if (action != SQLITE_UPDATE)
	{
		return;
	}

	if (strcmp(table_name, PREF_TBL_NAME) != 0)
	{
		SECURE_LOGE("given table name (%s) is not same", table_name);
		return;
	}

	buf = sqlite3_mprintf("SELECT %s FROM %s WHERE rowid='%lld';", PREF_F_KEY_NAME, PREF_TBL_NAME, rowid);
	if (buf == NULL)
	{
		return;
	}
	ret = sqlite3_get_table(pref_db, buf, &result, &rows, &columns, &errmsg);
	sqlite3_free(buf);
	if (ret != SQLITE_OK)
	{
		LOGI("fail to read data(%s)", errmsg);
		sqlite3_free(errmsg);
		return;
	}

	if (rows == 0)
	{
		sqlite3_free_table(result);
		return;
	}

	tmp_node = _find_node(result[1]);

	if (tmp_node != NULL && tmp_node->cb != NULL)
	{
		tmp_node->cb(result[1], tmp_node->user_data);
	}

	sqlite3_free_table(result);
}


int preference_remove(const char *key)
{
	int ret;
	char *buf;
	bool exist;
	sqlite3_stmt *stmt;

	ret = preference_is_existing(key, &exist);
	if (ret != PREFERENCE_ERROR_NONE)
	{
		return ret;
	}

	if (!exist)
	{
		return PREFERENCE_ERROR_NO_KEY;
	}

	/* insert data or update data if data already exist */
	buf = sqlite3_mprintf("DELETE FROM %s WHERE %s = ?",
							PREF_TBL_NAME, PREF_F_KEY_NAME, key);
	if (buf == NULL)
	{
		LOGE("IO_ERROR(0x%08x) : fail to create query string", PREFERENCE_ERROR_IO_ERROR);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	ret = sqlite3_prepare(pref_db, buf, strlen(buf), &stmt, NULL);
	if (ret != SQLITE_OK) {
		LOGE("IO_ERROR(0x%08x) : fail to prepare query (%d/%s)",
			PREFERENCE_ERROR_IO_ERROR,
			sqlite3_extended_errcode(pref_db),
			sqlite3_errmsg(pref_db));
		return PREFERENCE_ERROR_IO_ERROR;
	}

	ret = sqlite3_bind_text(stmt, 1, key, strlen(key), SQLITE_STATIC);
	if(ret != SQLITE_OK) {
		LOGE("IO_ERROR(0x%08x) : fail to bind(1) query (%d/%s)",
			PREFERENCE_ERROR_IO_ERROR,
			sqlite3_extended_errcode(pref_db),
			sqlite3_errmsg(pref_db));
		sqlite3_finalize(stmt);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_DONE) {
		LOGE("IO_ERROR(0x%08x): fail to delete data(%d/%s)",
			PREFERENCE_ERROR_IO_ERROR,
			sqlite3_extended_errcode(pref_db),
			sqlite3_errmsg(pref_db));
		sqlite3_finalize(stmt);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	sqlite3_finalize(stmt);
	if(buf) {
		sqlite3_free(buf);
		buf = NULL;
	}

	// if exist, remove changed cb
	 _remove_node(key);

	return PREFERENCE_ERROR_NONE;
}


int preference_remove_all(void)
{
	int ret;
	char *buf;
	char *errmsg;

	if (pref_db == NULL)
	{
		if (_initialize() != PREFERENCE_ERROR_NONE)
		{
			LOGE("IO_ERROR(0x%08x) : fail to initialize db", PREFERENCE_ERROR_IO_ERROR);
			return PREFERENCE_ERROR_IO_ERROR;
		}
	}

	/* insert data or update data if data already exist */
	buf = sqlite3_mprintf("DELETE FROM %s;", PREF_TBL_NAME);
	if (buf == NULL)
	{
		LOGE("IO_ERROR(0x%08x) : fail to create query string", PREFERENCE_ERROR_IO_ERROR);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	ret = sqlite3_exec(pref_db, buf, NULL, NULL, &errmsg);
	sqlite3_free(buf);
	if (ret != SQLITE_OK)
	{
		LOGE("IO_ERROR(0x%08x) : fail to delete data (%s)", PREFERENCE_ERROR_IO_ERROR, errmsg);
		sqlite3_free(errmsg);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	// if exist, remove changed cb
	_remove_all_node();

	return PREFERENCE_ERROR_NONE;
}


int preference_set_changed_cb(const char *key, preference_changed_cb callback, void *user_data)
{
	int ret;
	bool exist;

	ret = preference_is_existing(key, &exist);
	if (ret != PREFERENCE_ERROR_NONE)
	{
		return ret;
	}

	if (!exist)
	{
		LOGE("NO_KEY(0x%08x) : fail to find given key(%s)", PREFERENCE_ERROR_NO_KEY, key);
		return PREFERENCE_ERROR_NO_KEY;
	}

	if (!is_update_hook_registered)
	{
		sqlite3_update_hook(pref_db, _update_cb, NULL);
		is_update_hook_registered = true;
	}

	return _add_node(key, callback, user_data);
}

int preference_unset_changed_cb(const char *key)
{
	int ret;
	bool exist;

	ret = preference_is_existing(key, &exist);
	if (ret != PREFERENCE_ERROR_NONE)
	{
		return ret;
	}

	if (!exist)
	{
		LOGE("NO_KEY(0x%08x) : fail to find given key(%s)", PREFERENCE_ERROR_NO_KEY, key);
		return PREFERENCE_ERROR_NO_KEY;
	}

	return _remove_node(key);
}

int preference_foreach_item(preference_item_cb callback, void *user_data)
{
	int ret;
	char *buf;
	char **result;
	int rows;
	int columns;
	char *errmsg;
	int i;

	if (callback == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", PREFERENCE_ERROR_INVALID_PARAMETER);
		return PREFERENCE_ERROR_INVALID_PARAMETER;
	}

	if (pref_db == NULL)
	{
		if (_initialize() != PREFERENCE_ERROR_NONE)
		{
			LOGE("IO_ERROR(0x%08x) : fail to initialize db", PREFERENCE_ERROR_IO_ERROR);
			return PREFERENCE_ERROR_IO_ERROR;
		}
	}

	buf = sqlite3_mprintf("SELECT %s FROM %s;", PREF_F_KEY_NAME, PREF_TBL_NAME);
	if (buf == NULL)
	{
		LOGE("IO_ERROR(0x%08x) : fail to create query string", PREFERENCE_ERROR_IO_ERROR);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	ret = sqlite3_get_table(pref_db, buf, &result, &rows, &columns, &errmsg);
	sqlite3_free(buf);
	if (ret != SQLITE_OK)
	{
		LOGE("IO_ERROR(0x%08x) : fail to read data (%s)", PREFERENCE_ERROR_IO_ERROR, errmsg);
		sqlite3_free(errmsg);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	for (i = 1; i <= rows; i++)
	{
		if (callback(result[i], user_data) != true)
		{
			break;
		}
	}

	sqlite3_free_table(result);

	return PREFERENCE_ERROR_NONE;
}

