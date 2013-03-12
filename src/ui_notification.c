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
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <dlog.h>
#include <notification.h>

#include <app.h>
#include <app_service_private.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "CAPI_APPFW_APPLICATION_UI_NOTIFICATION"

struct ui_notification_s {
	notification_h raw_handle;
	bool ongoing;
	bool posted;
	bool removed;
	char *icon;
	struct tm *time;
	char *title;
	char *content;
	service_h service;
	char *sound;
	bool vibration;
};

static int ui_notification_error_handler(int error, const char *func, const char *on_error)
{
	int retcode;
	char *error_msg;

	switch (error)
	{
	case NOTIFICATION_ERROR_NONE:
		retcode = UI_NOTIFICATION_ERROR_NONE;
		break;

	case NOTIFICATION_ERROR_INVALID_DATA:
		retcode = UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
		error_msg = "INVALID_PARAMETER";
		break;

	case NOTIFICATION_ERROR_NO_MEMORY:
		retcode = UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		error_msg = "OUT_OF_MEMORY";
		break;

	case NOTIFICATION_ERROR_FROM_DB:
		retcode = UI_NOTIFICATION_ERROR_DB_FAILED;
		error_msg = "DB_FAILED";
		break;

	case NOTIFICATION_ERROR_ALREADY_EXIST_ID:
	case NOTIFICATION_ERROR_NOT_EXIST_ID:
		retcode = UI_NOTIFICATION_ERROR_INVALID_STATE;
		error_msg = "INVALID_STATE";
		break;

	default:
		retcode = UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
		error_msg = "INVALID_PARAMETER";
	}

	if (retcode != UI_NOTIFICATION_ERROR_NONE)
	{
		LOGE("[%s] %s(0x%08x) : %s", func, error_msg, retcode, on_error);
	}

	return retcode;
}


int ui_notification_create(bool ongoing, ui_notification_h *notification)
{
	ui_notification_h notification_out;

	if (notification == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x) : invalid output param", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	notification_out = (ui_notification_h)calloc(1, sizeof(struct ui_notification_s));

	if (notification_out == NULL)
	{
		LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	notification_out->raw_handle = NULL;
	notification_out->ongoing = ongoing;
	notification_out->posted = false;
	notification_out->removed = false;
	notification_out->icon = NULL;
	notification_out->time = NULL;
	notification_out->title = NULL;
	notification_out->content = NULL;
	notification_out->service = NULL;
	notification_out->sound = NULL;
	notification_out->vibration = false;

	*notification = notification_out;

	return UI_NOTIFICATION_ERROR_NONE;
}

static int ui_notification_construct(bool ongoing, notification_h raw_handle, ui_notification_h *notification)
{
	int retcode;
	ui_notification_h notification_out;
	char *icon;
	time_t time;
	char *title;
	char *content;
	bundle *service_data;
	const char *sound = NULL;
	notification_sound_type_e sound_type;
	notification_vibration_type_e vib_type;

	if (notification == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x) : invalid output param", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	notification_out = (ui_notification_h)calloc(1, sizeof(struct ui_notification_s));

	if (notification_out == NULL)
	{
		LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	retcode = ui_notification_error_handler(notification_clone(raw_handle, &(notification_out->raw_handle)),\
				 __FUNCTION__, "failed to clone the notification handle");

	if (retcode != NOTIFICATION_ERROR_NONE)
	{
		free(notification_out);
		return retcode;
	}

	notification_out->ongoing = ongoing;

	notification_out->posted = true;

	notification_out->removed = false;

	if (!notification_get_image(raw_handle, NOTIFICATION_IMAGE_TYPE_ICON, &icon) && icon)
	{
		notification_out->icon = strdup(icon);
	}

	if (!notification_get_time(raw_handle, &time))
	{
		notification_out->time = malloc(sizeof(struct tm));

		if (notification_out->time == NULL)
		{
			ui_notification_destroy(notification_out);
			LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}

		localtime_r(&time, notification_out->time);
	}

	if (!notification_get_text(raw_handle, NOTIFICATION_TEXT_TYPE_TITLE, &title) && title)
	{
		notification_out->title = strdup(title);
	}

	if (!notification_get_text(raw_handle, NOTIFICATION_TEXT_TYPE_CONTENT, &content) && content)
	{
		notification_out->content = strdup(content);
	}

	if (!notification_get_sound(raw_handle, &sound_type, &sound) && sound)
	{
		notification_out->sound = strdup(sound);
	}

	if (!notification_get_vibration(raw_handle, &vib_type, NULL))
	{
		if (vib_type == NOTIFICATION_VIBRATION_TYPE_DEFAULT)
		{
			notification_out->vibration = true;
		}
	}

	if (!notification_get_execute_option(raw_handle, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, &service_data))
	{
		service_h service;

		if (!service_create_request(service_data, &service))
		{
			notification_out->service = service;
		}
	}

	*notification = notification_out;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_destroy(ui_notification_h notification)
{
	if (notification == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x) : invalid handle", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->raw_handle)
		notification_free(notification->raw_handle);

	if (notification->icon)
		free(notification->icon);

	if (notification->time)
		free(notification->time);

	if (notification->title)
		free(notification->title);

	if (notification->content)
		free(notification->content);

	if (notification->sound)
		free(notification->sound);

	if (notification->service)
		service_destroy(notification->service);

	free(notification);

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_get_id(ui_notification_h notification, int *id)
{
        notification_h raw_handle = NULL;

        if (notification == NULL || id == NULL)
        {
                LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
                return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
        }
        if (notification->raw_handle == NULL)
        {
                LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
                return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
        }

        raw_handle = notification->raw_handle;
        if (notification_get_id(raw_handle, NULL, id) != NOTIFICATION_ERROR_NONE) {
                LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
                return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
        }

        return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_clone(ui_notification_h *clone, ui_notification_h notification)
{
	ui_notification_h notification_out;
	int retcode;

	if (clone == NULL || notification == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	notification_out = (ui_notification_h)calloc(1, sizeof(struct ui_notification_s));

	if (notification_out == NULL)
	{
		LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	if (notification->raw_handle != NULL)
	{
		retcode = notification_clone(notification->raw_handle, &(notification_out->raw_handle));

		if (retcode)
		{
			free(notification_out);
			return ui_notification_error_handler(retcode, __FUNCTION__, "failed to clone the handle");
		}
	}

	notification_out->ongoing = notification->ongoing;

	notification_out->posted = notification->posted;

	notification_out->removed = notification->removed;

	if (notification->icon)
	{
		notification_out->icon = strdup(notification->icon);
	}

	if (notification->time)
	{
		notification_out->time = malloc(sizeof(struct tm));
		if (notification_out->time != NULL)
		{
			memcpy(notification_out->time, notification->time, sizeof(struct tm));
		}
	}

	if (notification->title)
	{
		notification_out->title = strdup(notification->title);
	}

	if (notification->content)
	{
		notification_out->content = strdup(notification->content);
	}

	if (notification->sound)
	{
		notification_out->sound = strdup(notification->sound);
	}

	notification_out->vibration = notification->vibration;

	if (notification->service)
	{
		service_clone(&(notification_out->service), notification->service);
	}

	*clone = notification_out;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_is_ongoing(ui_notification_h notification, bool *ongoing)
{
	if (notification == NULL || ongoing == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*ongoing = notification->ongoing;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_set_icon(ui_notification_h notification, const char *path)
{
	char *path_dup = NULL;

	if (notification == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (path != NULL)
	{
		path_dup = strdup(path);

		if (path_dup == NULL)
		{
			LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}
	}

	if (notification->icon != NULL)
	{
		free(notification->icon);
	}

	notification->icon = path_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_get_icon(ui_notification_h notification, char **path)
{
	char *path_dup = NULL;

	if (notification == NULL || path == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->icon != NULL)
	{
		path_dup = strdup(notification->icon);

		if (path_dup == NULL)
		{
			LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}
	}

	*path = path_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_set_time(ui_notification_h notification, struct tm *time)
{
	struct tm *time_dup = NULL;

	if (notification == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (time != NULL)
	{
		time_dup = malloc(sizeof(struct tm));

		if (time_dup == NULL)
		{
			LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}

		memcpy(time_dup, time, sizeof(struct tm));
	}

	if (notification->time != NULL)
	{
		free(notification->time);
	}

	notification->time = time_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_get_time(ui_notification_h notification, struct tm **time)
{
	struct tm *time_dup = NULL;

	if (notification == NULL || time == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->time != NULL)
	{
		time_dup = malloc(sizeof(struct tm));

		if (time_dup == NULL)
		{
			LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}

		memcpy(time_dup, notification->time, sizeof(struct tm));
	}

	*time = time_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_set_title(ui_notification_h notification, const char *title)
{
	char *title_dup = NULL;

	if (notification == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (title != NULL)
	{
		title_dup = strdup(title);

		if (title_dup == NULL)
		{
			LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}
	}

	if (notification->title != NULL)
	{
		free(notification->title);
	}

	notification->title = title_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_get_title(ui_notification_h notification, char **title)
{
	char *title_dup = NULL;

	if (notification == NULL || title == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->title != NULL)
	{
		title_dup = strdup(notification->title);

		if (title_dup == NULL)
		{
			LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}
	}

	*title = title_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}


int ui_notification_set_content(ui_notification_h notification, const char *content)
{
	char *content_dup = NULL;

	if (notification == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (content != NULL)
	{
		content_dup = strdup(content);

		if (content_dup == NULL)
		{
			LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}
	}

	if (notification->content != NULL)
	{
		free(notification->content);
	}

	notification->content = content_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_get_content(ui_notification_h notification, char **content)
{
	char *content_dup = NULL;

	if (notification == NULL || content == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->content != NULL)
	{
		content_dup = strdup(notification->content);

		if (content_dup == NULL)
		{
			LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}
	}

	*content = content_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}


int ui_notification_set_service(ui_notification_h notification, service_h service)
{
	int retcode;
	service_h service_dup = NULL;

	if (notification == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (service != NULL)
	{
		retcode = service_clone(&service_dup, service);

		if (retcode != SERVICE_ERROR_NONE)
		{
			if (retcode == SERVICE_ERROR_OUT_OF_MEMORY)
			{
				LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
				return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
			}
			else
			{
				LOGE("INVALID_PARAMETER(0x%08x) : invalid service handle", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
				return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
			}
		}
	}

	if (notification->service != NULL)
	{
		service_destroy(notification->service);
	}

	notification->service = service_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_get_service(ui_notification_h notification, service_h *service)
{
	int retcode;
	service_h service_dup = NULL;

	if (notification == NULL || service == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->service != NULL)
	{
		retcode = service_clone(&service_dup, notification->service);

		if (retcode != SERVICE_ERROR_NONE)
		{
			if (retcode == SERVICE_ERROR_OUT_OF_MEMORY)
			{
				LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
				return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
			}
			else
			{
				LOGE("INVALID_PARAMETER(0x%08x) : invalid service handle", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
				return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
			}
		}
	}

	*service = service_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_set_sound(ui_notification_h notification, const char *path)
{
	char *path_dup = NULL;

	if (notification == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (path != NULL)
	{
		path_dup = strdup(path);

		if (path_dup == NULL)
		{
			LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}
	}

	if (notification->sound != NULL)
	{
		free(notification->sound);
	}

	notification->sound = path_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_get_sound(ui_notification_h notification, char **path)
{
	char *path_dup = NULL;

	if (notification == NULL || path == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->sound != NULL)
	{
		path_dup = strdup(notification->sound);

		if (path_dup == NULL)
		{
			LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
			*path = NULL;

			return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}
	}

	*path = path_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_set_vibration(ui_notification_h notification, bool value)
{
	if (notification == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	notification->vibration = value;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_get_vibration(ui_notification_h notification, bool *value)
{
	if (notification == NULL || value == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	*value = notification->vibration;

	return UI_NOTIFICATION_ERROR_NONE;
}

static int ui_notification_build_attributes(ui_notification_h notification)
{
	bundle *service_data;

	if (notification == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x) : invalid handle", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->icon != NULL)
	{
		struct stat st;

		if (stat(notification->icon, &st) < 0)
		{
			LOGE("NO_SUCH_FILE(0x%08x) : invalid icon", UI_NOTIFICATION_ERROR_NO_SUCH_FILE);
			return UI_NOTIFICATION_ERROR_NO_SUCH_FILE;
		}

		notification_set_image(notification->raw_handle, NOTIFICATION_IMAGE_TYPE_ICON, notification->icon);
	}

	if (notification->time != NULL)
	{
		notification_set_time(notification->raw_handle, mktime(notification->time));
	}

	if (notification->title != NULL)
	{
		notification_set_text(notification->raw_handle, NOTIFICATION_TEXT_TYPE_TITLE, notification->title, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	}

	if (notification->content != NULL)
	{
		notification_set_text(notification->raw_handle, NOTIFICATION_TEXT_TYPE_CONTENT, notification->content, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	}

	if (notification->service != NULL && service_to_bundle(notification->service, &service_data) == SERVICE_ERROR_NONE)
	{
		notification_set_property(notification->raw_handle, 0);
		notification_set_execute_option(notification->raw_handle, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, service_data);
	}
	else
	{
		notification_set_property(notification->raw_handle, NOTIFICATION_PROP_DISABLE_APP_LAUNCH);
	}

	if (notification->sound != NULL)
	{
		struct stat st;

		if (stat(notification->sound, &st) < 0)
		{
			LOGE("NO_SUCH_FILE(0x%08x) : invalid sound file", UI_NOTIFICATION_ERROR_NO_SUCH_FILE);
			return UI_NOTIFICATION_ERROR_NO_SUCH_FILE;
		}
		notification_set_sound(notification->raw_handle, NOTIFICATION_SOUND_TYPE_USER_DATA, notification->sound);
	}

	if (notification->vibration)
	{
		notification_set_vibration(notification->raw_handle, NOTIFICATION_VIBRATION_TYPE_DEFAULT, NULL);
	}

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_post(ui_notification_h notification)
{
	int retcode;

	if (notification == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->posted == true)
	{
		LOGE("INVALID_STATE(0x%08x) : the notification was already posted", UI_NOTIFICATION_ERROR_INVALID_STATE);
		return UI_NOTIFICATION_ERROR_INVALID_STATE;
	}

	if (notification->ongoing == true)
	{
		notification->raw_handle = notification_new(NOTIFICATION_TYPE_ONGOING, NOTIFICATION_GROUP_ID_DEFAULT, NOTIFICATION_PRIV_ID_NONE);
	}
	else
	{
		notification->raw_handle = notification_new(NOTIFICATION_TYPE_NOTI, NOTIFICATION_GROUP_ID_DEFAULT, NOTIFICATION_PRIV_ID_NONE);
	}

	if (notification->raw_handle == NULL)
	{
		LOGE("OUT_OF_MEMORY(0x%08x)", UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	retcode = ui_notification_build_attributes(notification);

	if (retcode != UI_NOTIFICATION_ERROR_NONE)
	{
		return retcode;
	}

	retcode = ui_notification_error_handler(notification_insert(notification->raw_handle, NULL), __FUNCTION__, "failed to post a notification");

	if (retcode == UI_NOTIFICATION_ERROR_NONE)
	{
		notification->posted = true;
	}

	return retcode;
}

int ui_notification_update(ui_notification_h notification)
{
	int retcode;

	if (notification == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->posted == false)
	{
		LOGE("INVALID_STATE(0x%08x) : the notification was not posted", UI_NOTIFICATION_ERROR_INVALID_STATE);
		return UI_NOTIFICATION_ERROR_INVALID_STATE;
	}

	if (notification->removed == true)
	{
		LOGE("INVALID_STATE(0x%08x) : the notification was canceled or cleared", UI_NOTIFICATION_ERROR_INVALID_STATE);
		return UI_NOTIFICATION_ERROR_INVALID_STATE;
	}

	retcode = ui_notification_build_attributes(notification);

	if (retcode != UI_NOTIFICATION_ERROR_NONE)
	{
		return retcode;
	}

	retcode = ui_notification_error_handler(notification_update(notification->raw_handle), __FUNCTION__, "failed to post a notification");

	if (retcode == UI_NOTIFICATION_ERROR_INVALID_STATE)
	{
		notification->removed = true;
	}

	return retcode;
}

int  ui_notification_update_progress(ui_notification_h notification, ui_notification_progress_type_e type, double value)
{
	int retcode;

	if (notification == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->raw_handle == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x) : invalid handle", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->posted == false)
	{
		LOGE("INVALID_STATE(0x%08x) : the notification was not posted", UI_NOTIFICATION_ERROR_INVALID_STATE);
		return UI_NOTIFICATION_ERROR_INVALID_STATE;
	}

	if (notification->removed == true)
	{
		LOGE("INVALID_STATE(0x%08x) : the notification was canceled or cleared", UI_NOTIFICATION_ERROR_INVALID_STATE);
		return UI_NOTIFICATION_ERROR_INVALID_STATE;
	}

	if (value < 0)
	{
		LOGE("INVALID_PARAMETER(0x%08x) : the value must be greater than or equal to zero.", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	switch (type)
	{
	case UI_NOTIFICATION_PROGRESS_TYPE_SIZE:
		retcode = ui_notification_error_handler(
			notification_update_size(notification->raw_handle, NOTIFICATION_PRIV_ID_NONE, value),
			__FUNCTION__, "failed to update the progress");
		break;

	case UI_NOTIFICATION_PROGRESS_TYPE_PERCENTAGE:
		retcode = ui_notification_error_handler(
			notification_update_progress(notification->raw_handle, NOTIFICATION_PRIV_ID_NONE, value),
			__FUNCTION__, "failed to update the progress");
		break;

	default:
		LOGE("INVALID_PARAMETER(0x%08x) : invalid progress type", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (retcode == UI_NOTIFICATION_ERROR_INVALID_STATE)
	{
		notification->removed = true;
	}

	return retcode;
}

int ui_notification_cancel(ui_notification_h notification)
{
	int retcode;

	if (notification == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->raw_handle == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x) : invalid handle", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->posted == false)
	{
		LOGE("INVALID_STATE(0x%08x) : the notification was not posted", UI_NOTIFICATION_ERROR_INVALID_STATE);
		return UI_NOTIFICATION_ERROR_INVALID_STATE;
	}

	if (notification->removed == true)
	{
		LOGE("INVALID_STATE(0x%08x) : the notification was canceled or cleared", UI_NOTIFICATION_ERROR_INVALID_STATE);
		return UI_NOTIFICATION_ERROR_INVALID_STATE;
	}

	retcode = ui_notification_error_handler(notification_delete(notification->raw_handle), __FUNCTION__, "failed to cancel the notification");

	if (retcode == UI_NOTIFICATION_ERROR_NONE)
	{
		notification->removed = true;
	}

	return retcode;
}

void ui_notification_cancel_all(void)
{
	notification_delete_all_by_type(NULL, NOTIFICATION_TYPE_NONE);
}

void ui_notification_cancel_all_by_type(bool ongoing)
{
	notification_type_e type = NOTIFICATION_TYPE_NONE;

	if (ongoing)
		type = NOTIFICATION_TYPE_ONGOING;
	else
		type = NOTIFICATION_TYPE_NOTI;

	notification_delete_all_by_type(NULL, type);
}

void ui_notification_cancel_all_by_package(const char *package, bool ongoing)
{
	notification_type_e type = NOTIFICATION_TYPE_NONE;

	if (ongoing)
		type = NOTIFICATION_TYPE_ONGOING;
	else
		type = NOTIFICATION_TYPE_NOTI;

	notification_delete_all_by_type(package, type);
}

int ui_notification_cancel_all_by_app_id(const char *app_id, bool ongoing)
{
	notification_type_e type = NOTIFICATION_TYPE_NONE;

	if (app_id == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (ongoing)
		type = NOTIFICATION_TYPE_ONGOING;
	else
		type = NOTIFICATION_TYPE_NOTI;

	notification_delete_all_by_type(app_id, type);

	return UI_NOTIFICATION_ERROR_NONE;
}

static bool ui_notification_package_equal(notification_h handle)
{
	char *package = NULL;
	char *handle_package = NULL;
	char cmdline[512] = {0,};
	char buf[64] = {0,};

	if (notification_get_pkgname(handle, &handle_package))
	{
		return false;
	}

	if (app_get_package(&package))
	{
		int ret = 0;
		int fd = -1;
		int pid = getpid();

		snprintf(buf, sizeof(buf), "/proc/%d/cmdline", pid);

		fd = open(buf, O_RDONLY);
		if (fd < 0) {
			return false;
		}

		ret = read(fd, cmdline, sizeof(cmdline) - 1);
		if (ret <= 0) {
			close(fd);
			return false;
		}

		cmdline[ret] = 0;
		close(fd);

		if (strlen(cmdline) == strlen(handle_package))
		{
			if (!strncmp(cmdline, handle_package, strlen(cmdline)))
			{
				return true;
			}
		}
	}
	else
	{
		if (strlen(package) == strlen(handle_package))
		{
				if (!strncmp(package, handle_package, strlen(package)))
				{
					return true;
				}
		}
	}

	return false;
}

int ui_notification_foreach_notification_posted(bool ongoing, ui_notification_cb callback, void *user_data)
{
	notification_list_h raw_handle_list;
	notification_h raw_handle;
	notification_type_e notification_type = ongoing ? NOTIFICATION_TYPE_ONGOING : NOTIFICATION_TYPE_NOTI;
	ui_notification_h notification = NULL;
	bool iterate_next = true;

	if (callback == NULL)
	{
		LOGE("INVALID_PARAMETER(0x%08x)", UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification_get_grouping_list(notification_type, -1, &raw_handle_list))
	{
		LOGE("DB_FAILED(0x%08x) : failed to get a notification list", UI_NOTIFICATION_ERROR_DB_FAILED);
		return UI_NOTIFICATION_ERROR_DB_FAILED;
	}

	while (raw_handle_list != NULL)
	{
		raw_handle = notification_list_get_data(raw_handle_list);

		if (raw_handle != NULL && ui_notification_package_equal(raw_handle))
		{
			if (!ui_notification_construct(ongoing, raw_handle, &notification))
			{
				iterate_next = callback(notification, user_data);

				ui_notification_destroy(notification);

				if (iterate_next == false)
				{
					break;
				}
			}
		}

		raw_handle_list = notification_list_get_next(raw_handle_list);
	}

	notification_free_list(raw_handle_list);

	return UI_NOTIFICATION_ERROR_NONE;
}

