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


#ifndef __TIZEN_APPFW_PREFERENCE_PRIVATE_H__
#define __TIZEN_APPFW_PREFERENCE_PRIVATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define PREF_DB_NAME		".pref.db"
#define PREF_TBL_NAME		"pref"
#define PREF_F_KEY_NAME		"pref_key"
#define PREF_F_TYPE_NAME	"pref_type"
#define PREF_F_DATA_NAME	"pref_data"
#define BUF_LEN			(4096)

typedef enum
{
	PREFERENCE_TYPE_INT = 1,
	PREFERENCE_TYPE_BOOLEAN,
	PREFERENCE_TYPE_DOUBLE,
	PREFERENCE_TYPE_STRING
} preference_type_e;

typedef struct _pref_changed_cb_node_t{
	char *key;
	preference_changed_cb cb;
	void *user_data;
	struct _pref_changed_cb_node_t *prev;
	struct _pref_changed_cb_node_t *next;
} pref_changed_cb_node_t;

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_APPFW_PREFERENCE_PRIVATE_H__ */
