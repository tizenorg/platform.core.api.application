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


#ifndef __TIZEN_APPFW_PREFERENCE_INTERNAL_H__
#define __TIZEN_APPFW_PREFERENCE_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "app_preference_log.h"
#include "linux/limits.h"

#define BUF_LEN			(4096)
#define PREF_DIR	".pref/"

#define PREFERENCE_KEY_PATH_LEN        1024
#define ERR_LEN 		128

#define PREF_DB_NAME		".pref.db"
#define PREF_TBL_NAME		"pref"
#define PREF_F_KEY_NAME		"pref_key"
#define PREF_F_TYPE_NAME	"pref_type"
#define PREF_F_DATA_NAME	"pref_data"

/* ASCII VALUE */
#define PREF_KEYNAME_C_PAD '='
#define PREF_KEYNAME_C_PLUS '+'
#define PREF_KEYNAME_C_SLASH '/'

#define PREF_KEYNAME_C_DOT '.'
#define PREF_KEYNAME_C_UNDERSCORE '_'
#define PREF_KEYNAME_C_HYPHEN '-'

/**
 * @brief Definition for PREFERENCE_ERROR_WRONG_PREFIX.
 */
#define PREFERENCE_ERROR_WRONG_PREFIX    -2

/**
 * @brief Definition for PREFERENCE_ERROR_WRONG_TYPE.
 */
#define PREFERENCE_ERROR_WRONG_TYPE      -3

/**
 * @brief Definition for PREFERENCE_ERROR_FILE_OPEN.
 */
#define PREFERENCE_ERROR_FILE_OPEN       -21

/**
 * @brief Definition for PREFERENCE_ERROR_FILE_FREAD.
 */
#define PREFERENCE_ERROR_FILE_FREAD      -22

/**
 * @brief Definition for PREFERENCE_ERROR_FILE_FGETS.
 */
#define PREFERENCE_ERROR_FILE_FGETS      -23

/**
 * @brief Definition for PREFERENCE_ERROR_FILE_WRITE.
 */
#define PREFERENCE_ERROR_FILE_WRITE      -24

/**
 * @brief Definition for PREFERENCE_ERROR_FILE_SYNC.
 */
#define PREFERENCE_ERROR_FILE_SYNC       -25

/**
 * @brief Definition for PREFERENCE_ERROR_FILE_CHMOD.
 */
#define PREFERENCE_ERROR_FILE_CHMOD      -28

/**
 * @brief Definition for PREFERENCE_ERROR_FILE_LOCK.
 */
#define PREFERENCE_ERROR_FILE_LOCK       -29

typedef enum
{
	PREFERENCE_TYPE_NONE = 0,
	PREFERENCE_TYPE_STRING,
	PREFERENCE_TYPE_INT,
	PREFERENCE_TYPE_DOUBLE,
	PREFERENCE_TYPE_BOOLEAN,
} preference_type_e;

typedef struct _pref_changed_cb_node_t{
	char *key;
	preference_changed_cb cb;
	void *user_data;
	struct _pref_changed_cb_node_t *prev;
	struct _pref_changed_cb_node_t *next;
} pref_changed_cb_node_t;

typedef struct _keynode_t {
    char *keyname;           /**< Keyname for keynode */
    int type;                /**< Keynode type */
    union {
        int i;               /**< Integer type */
        int b;               /**< Bool type */
        double d;            /**< Double type */
        char *s;             /**< String type */
    } value;                 /**< Value for keynode */
    struct _keynode_t *next; /**< Next keynode */
} keynode_t;

/**
 * @brief The structure type for opaque type. It must be used via accessor functions.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef struct _keylist_t {
    int num;           /**< Number of list */
    keynode_t *head;   /**< Head node */
    keynode_t *cursor; /**< Cursor node */
} keylist_t;


int _preference_kdb_add_notify
	(keynode_t *keynode, preference_changed_cb cb, void *data);
int _preference_kdb_del_notify
	(keynode_t *keynode);

int _preference_get_key_path(keynode_t *keynode, char *path);
int _preference_get_key(keynode_t *keynode);

int _preference_keynode_set_keyname(keynode_t *keynode, const char *keyname);
inline void _preference_keynode_set_null(keynode_t *keynode);
inline keynode_t *_preference_keynode_new(void);
inline void _preference_keynode_free(keynode_t *keynode);


#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_APPFW_PREFERENCE_INTERNAL_H__ */
