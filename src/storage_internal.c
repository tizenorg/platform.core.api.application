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

#define INTERNAL_MEMORY_PATH "/opt/usr/media"

int storage_internal_get_state()
{
	return STORAGE_STATE_MOUNTED;
}

static int storage_internal_set_state_cb(void *data)
{
	return 0;
}

static void storage_internal_unset_state_cb()
{
	// empty function
}

int storage_internal_get_space(unsigned long long *total, unsigned long long *available)
{
	return storage_statfs(INTERNAL_MEMORY_PATH, total, available);
}

storage_device_h storage_internal_device()
{
	storage_device_h device;

	device = calloc(1, sizeof(struct storage_device_s));

	if (device == NULL)
	{
		LOGE("OUT_OF_MEMORY(0x%08x)", STORAGE_ERROR_OUT_OF_MEMORY);
		return NULL;
	}

	device->type = STORAGE_TYPE_INTERNAL;
	device->path = INTERNAL_MEMORY_PATH;
	device->get_state = storage_internal_get_state;
	device->set_state_cb = storage_internal_set_state_cb;
	device->unset_state_cb = storage_internal_unset_state_cb;
	device->get_space = storage_internal_get_space;

	return device;
}

