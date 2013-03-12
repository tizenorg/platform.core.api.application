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


#ifndef __TIZEN_APPFW_STORAGE_PRIVATE_H__
#define __TIZEN_APPFW_STORAGE_PRIVATE_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef int (*storage_dev_get_state)(void);
typedef int (*storage_dev_set_state_cb)(void *data);
typedef void (*storage_dev_unset_state_cb)(void);
typedef int (*storage_dev_get_space)(unsigned long long *total, unsigned long long *available);

typedef struct storage_device_s
{
	storage_type_e type;
	char *path;
	storage_dev_get_state get_state;
	storage_dev_set_state_cb set_state_cb;
	storage_dev_unset_state_cb unset_state_cb;
	storage_dev_get_space get_space;
} *storage_device_h;

typedef struct storage_info_s
{
	int id;
	storage_device_h device;
	storage_state_e state;
	storage_state_changed_cb state_cb;
	void *state_cb_data;
} *storage_info_h;

void storage_dispatch_state_event(storage_state_e state, void* data);

int storage_statfs(const char *directory, unsigned long long *total, unsigned long long *available);

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_APPFW_STORAGE_PRIVATE_H__ */
