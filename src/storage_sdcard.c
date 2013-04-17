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

#define SDCARD_PATH "/opt/storage/sdcard"

int storage_sdcard_get_state()
{
	int sdcard_state = 0;

	vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS, &sdcard_state);

	switch (sdcard_state)
	{
		case VCONFKEY_SYSMAN_MMC_REMOVED:
			return STORAGE_STATE_REMOVED;

		case VCONFKEY_SYSMAN_MMC_MOUNTED:
			return STORAGE_STATE_MOUNTED;

		case VCONFKEY_SYSMAN_MMC_INSERTED_NOT_MOUNTED:
			return STORAGE_STATE_UNMOUNTABLE;

		default:
			return STORAGE_STATE_REMOVED;
	}
}

static void storage_sdcard_state_cb_broker(keynode_t* key, void* data)
{
	storage_state_e state;
	state = storage_sdcard_get_state();

	storage_dispatch_state_event(state, data);
}

static int storage_sdcard_set_state_cb(void *data)
{
	vconf_notify_key_changed(VCONFKEY_SYSMAN_MMC_STATUS, storage_sdcard_state_cb_broker, data);
	return 0;
}

static void storage_sdcard_unset_state_cb()
{
	vconf_ignore_key_changed(VCONFKEY_SYSMAN_MMC_STATUS, storage_sdcard_state_cb_broker);
}

static int storage_sdcard_get_space(unsigned long long *total, unsigned long long *available)
{
	storage_state_e state;
	state = storage_sdcard_get_state();

	if (state < STORAGE_STATE_MOUNTED)
	{
		if (total != NULL)
		{
			*total = 0;
		}

		if (available != NULL)
		{
			*available = 0;
		}

		return 0;
	}
	else
	{
		return storage_statfs(SDCARD_PATH, total, available);
	}
}

storage_device_h storage_sdcard_device()
{
	storage_device_h device;

	device = calloc(1, sizeof(struct storage_device_s));

	if (device == NULL)
	{
		LOGE("OUT_OF_MEMORY(0x%08x)", STORAGE_ERROR_OUT_OF_MEMORY);
		return NULL;
	}

	device->type = STORAGE_TYPE_EXTERNAL;
	device->path = SDCARD_PATH;
	device->get_state = storage_sdcard_get_state;
	device->set_state_cb = storage_sdcard_set_state_cb;
	device->unset_state_cb = storage_sdcard_unset_state_cb;
	device->get_space = storage_sdcard_get_space;

	return device;
}

