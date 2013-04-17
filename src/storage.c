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
#include <sys/vfs.h>

#include <aul.h>
#include <dlog.h>
#include <vconf.h>

#include <app_storage.h>
#include <app_storage_private.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "CAPI_APPFW_APPLICATION_STORAGE"

static int storage_initialize();
static int storage_register_device(storage_device_h device);
static int storage_get_storage(int id, storage_info_h* storage_info);

extern storage_device_h storage_internal_device();
extern storage_device_h storage_sdcard_device();
extern storage_device_h storage_usbhost_device();

extern int storage_internal_get_state();
extern int storage_sdcard_get_state();
extern int storage_usbhost_get_state();

#define STORAGE_MAX 3
static struct storage_info_s storage_info_table[STORAGE_MAX];
static int storage_num = 0;

static int storage_register_device(storage_device_h device)
{
	if (device == NULL)
	{
		return -1;
	}

	if (storage_num >= STORAGE_MAX)
	{
		LOGE("failed to register device : not enough device table");
		return -1;
	}

	storage_info_table[storage_num].id = storage_num;
	storage_info_table[storage_num].device = device;
	storage_info_table[storage_num].state = device->get_state();
	storage_info_table[storage_num].state_cb = NULL;
	storage_info_table[storage_num].state_cb_data = NULL;

	storage_num++;

	return 0;
}

static int storage_initialize()
{
	storage_device_h dev_internal;
	storage_device_h dev_sdcard;
	storage_device_h dev_usbhost;

	dev_internal = storage_internal_device();
	storage_register_device(dev_internal);

	dev_sdcard = storage_sdcard_device();
	storage_register_device(dev_sdcard);

	dev_usbhost = storage_usbhost_device();
	storage_register_device(dev_usbhost);

	return 0;
}

static int storage_get_storage(int id, storage_info_h* storage_info)
{
	int device_state = -1;

	if (storage_num < 1)
	{
		if (storage_initialize() != 0)
		{
			return STORAGE_ERROR_NOT_SUPPORTED;
		}
	}

	if (id < 0 || id >= storage_num)
	{
		return STORAGE_ERROR_NOT_SUPPORTED;
	}

	switch (id)
	{
	case 0:
		device_state = storage_internal_get_state();
		break;
	case 1:
		device_state = storage_sdcard_get_state();
		break;
	case 2:
		device_state = storage_usbhost_get_state();
		break;
	default:
		LOGE("Device statei is invalid");
		break;
	}
	storage_info_table[id].state = device_state;
	*storage_info = &(storage_info_table[id]);

	return STORAGE_ERROR_NONE;
}

int storage_foreach_device_supported(storage_device_supported_cb callback, void *user_data)
{
	int storage_id = 0;
	storage_info_h storage_info = NULL;
	bool foreach_next = false;

	if (callback == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x) : invalid callback", STORAGE_ERROR_INVALID_PARAMETER);
		return STORAGE_ERROR_INVALID_PARAMETER;
	}

	while (true)
	{
		if (storage_get_storage(storage_id, &storage_info) != 0)
		{
			break;
		}

		storage_id++;

		foreach_next = callback(storage_info->id, storage_info->device->type, storage_info->state, storage_info->device->path, user_data);

		if (foreach_next == false)
		{
			break;
		}
	}

	return STORAGE_ERROR_NONE;
}


int storage_get_root_directory(int storage, char **path)
{
	storage_info_h storage_info;

	if (path == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x) : invalid output param", STORAGE_ERROR_INVALID_PARAMETER);
		return STORAGE_ERROR_INVALID_PARAMETER;
	}

	if (storage_get_storage(storage, &storage_info) != 0)
	{
		LOGE("NOT_SUPPORTED(0x%08x) : storage(%d)", STORAGE_ERROR_NOT_SUPPORTED, storage);
		return STORAGE_ERROR_NOT_SUPPORTED;
	}

	*path = strdup(storage_info->device->path);

	return STORAGE_ERROR_NONE;
}


int storage_get_type(int storage, storage_type_e *type)
{
	storage_info_h storage_info;

	if (type == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x) : invalid output param", STORAGE_ERROR_INVALID_PARAMETER);
		return STORAGE_ERROR_INVALID_PARAMETER;
	}

	if (storage_get_storage(storage, &storage_info) != 0)
	{
		LOGE("NOT_SUPPORTED(0x%08x) : storage(%d)", STORAGE_ERROR_NOT_SUPPORTED, storage);
		return STORAGE_ERROR_NOT_SUPPORTED;
	}

	*type = storage_info->device->type;

	return STORAGE_ERROR_NONE;
}


int storage_get_state(int storage, storage_state_e *state)
{
	storage_info_h storage_info;
	storage_dev_get_state get_state;

	if (state == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x) : invalid output param", STORAGE_ERROR_INVALID_PARAMETER);
		return STORAGE_ERROR_INVALID_PARAMETER;
	}

	if (storage_get_storage(storage, &storage_info) != 0)
	{
		LOGE("NOT_SUPPORTED(0x%08x) : storage(%d)", STORAGE_ERROR_NOT_SUPPORTED, storage);
		return STORAGE_ERROR_NOT_SUPPORTED;
	}

	get_state = storage_info->device->get_state;

	if (get_state == NULL)
	{
		LOGE("NOT_SUPPORTED(0x%08x) : storage(%d)", STORAGE_ERROR_NOT_SUPPORTED, storage);
		return STORAGE_ERROR_NOT_SUPPORTED;
	}

	storage_info->state = get_state();

	*state = storage_info->state;

	return STORAGE_ERROR_NONE;
}


void storage_dispatch_state_event(storage_state_e state, void* data)
{
	storage_info_h storage_info;
	storage_state_changed_cb state_cb;

	storage_info = data;

	if (storage_info == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x) : invalid storage information", STORAGE_ERROR_INVALID_PARAMETER);
		return;
	}

	storage_info->state = state;
	state_cb = storage_info->state_cb;

	if (state_cb != NULL)
	{
		state_cb(storage_info->id, state, storage_info->state_cb_data);
	}
}


int storage_set_state_changed_cb(int storage, storage_state_changed_cb callback, void *user_data)
{
	storage_info_h storage_info;
	storage_dev_set_state_cb set_state_cb;

	if (callback == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x) : invalid callback", STORAGE_ERROR_INVALID_PARAMETER);
		return STORAGE_ERROR_INVALID_PARAMETER;
	}

	if (storage_get_storage(storage, &storage_info) != 0)
	{
		LOGE("NOT_SUPPORTED(0x%08x) : storage(%d)", STORAGE_ERROR_NOT_SUPPORTED, storage);
		return STORAGE_ERROR_NOT_SUPPORTED;
	}

	storage_info->state_cb = callback;
	storage_info->state_cb_data = user_data;

	set_state_cb = storage_info->device->set_state_cb;

	if (set_state_cb == NULL)
	{
		LOGE("NOT_SUPPORTED(0x%08x) : storage(%d)", STORAGE_ERROR_NOT_SUPPORTED, storage);
		return STORAGE_ERROR_NOT_SUPPORTED;
	}

	if (set_state_cb(storage_info) != 0)
	{
		LOGE("NOT_SUPPORTED(0x%08x) : storage(%d)", STORAGE_ERROR_NOT_SUPPORTED, storage);
		return STORAGE_ERROR_NOT_SUPPORTED;
	}

	return STORAGE_ERROR_NONE;
}


int storage_unset_state_changed_cb(int storage)
{
	storage_info_h storage_info;
	storage_dev_unset_state_cb unset_state_cb;

	if (storage_get_storage(storage, &storage_info) != 0)
	{
		LOGE("NOT_SUPPORTED(0x%08x) : storage(%d)", STORAGE_ERROR_NOT_SUPPORTED, storage);
		return STORAGE_ERROR_NOT_SUPPORTED;
	}

	storage_info->state_cb = NULL;
	unset_state_cb = storage_info->device->unset_state_cb;

	if (unset_state_cb != NULL)
	{
		unset_state_cb();
	}

	return STORAGE_ERROR_NONE;
}


int storage_get_total_space(int storage, unsigned long long *bytes)
{
	storage_info_h storage_info;
	storage_dev_get_space get_space;

	if (bytes == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x) : invalid output param", STORAGE_ERROR_INVALID_PARAMETER);
		return STORAGE_ERROR_INVALID_PARAMETER;
	}

	if (storage_get_storage(storage, &storage_info) != 0)
	{
		LOGE("NOT_SUPPORTED(0x%08x) : storage(%d)", STORAGE_ERROR_NOT_SUPPORTED, storage);
		return STORAGE_ERROR_NOT_SUPPORTED;
	}

	get_space = storage_info->device->get_space;

	if (get_space == NULL)
	{
		LOGE("NOT_SUPPORTED(0x%08x) : storage(%d)", STORAGE_ERROR_NOT_SUPPORTED, storage);
		return STORAGE_ERROR_NOT_SUPPORTED;
	}

	if (get_space(bytes, NULL) != 0)
	{
		LOGE("NOT_SUPPORTED(0x%08x) : storage(%d)", STORAGE_ERROR_NOT_SUPPORTED, storage);
		return STORAGE_ERROR_NOT_SUPPORTED;
	}

	return STORAGE_ERROR_NONE;
}

int storage_get_available_space(int storage, unsigned long long *bytes)
{
	storage_info_h storage_info;
	storage_dev_get_space get_space;

	if (bytes == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x) : invalid output param", STORAGE_ERROR_INVALID_PARAMETER);
		return STORAGE_ERROR_INVALID_PARAMETER;
	}

	if (storage_get_storage(storage, &storage_info) != 0)
	{
		LOGE("NOT_SUPPORTED(0x%08x) : storage(%d)", STORAGE_ERROR_NOT_SUPPORTED, storage);
		return STORAGE_ERROR_NOT_SUPPORTED;
	}

	get_space = storage_info->device->get_space;

	if (get_space == NULL)
	{
		LOGE("NOT_SUPPORTED(0x%08x) : storage(%d)", STORAGE_ERROR_NOT_SUPPORTED, storage);
		return STORAGE_ERROR_NOT_SUPPORTED;
	}

	if (get_space(NULL, bytes) != 0)
	{
		LOGE("NOT_SUPPORTED(0x%08x) : storage(%d)", STORAGE_ERROR_NOT_SUPPORTED, storage);
		return STORAGE_ERROR_NOT_SUPPORTED;
	}

	return STORAGE_ERROR_NONE;
}

int storage_statfs(const char *directory, unsigned long long *total, unsigned long long *available)
{
	struct statfs fs;

	if (statfs(directory, &fs) < 0)
	{
		LOGE("statfs returns error(%d) directory(%s)\n", errno, directory);
		return -1;
	}

	if (total != NULL)
	{
		*total = (unsigned long long)fs.f_bsize * (unsigned long long)fs.f_blocks;
	}

	if (available != NULL)
	{
		*available = (unsigned long long)fs.f_bsize * (unsigned long long)fs.f_bavail;
	}

	return 0;
}

