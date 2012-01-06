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

#include <dlog.h>
#include <notification.h>

#include <app_service.h>
#include <app_service_private.h>
#include <app_ui_notification.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "TIZEN_N_UI_NOTIFICATION"

struct ui_notification_s {
	notification_h core;
	bool ongoing;
	bool posted;
	char *icon;
	struct tm *time;
	char *title;
	char *content;
	service_h service;
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
		retcode = UI_NOTIFICATION_ERROR_ALREADY_POSTED;
		error_msg = "ALREADY_POSTED";
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
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid output param", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	notification_out = (ui_notification_h)malloc(sizeof(struct ui_notification_s));

	if (notification_out == NULL)
	{
		LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	notification_out->ongoing = ongoing;
	notification_out->posted = false;
	notification_out->icon = NULL;
	notification_out->time = NULL;
	notification_out->title = NULL;
	notification_out->content = NULL;
	notification_out->service = NULL;
	
	*notification = notification_out;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_destroy(ui_notification_h notification)
{
	if (notification == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->core)
		notification_free(notification->core);

	if (notification->icon)
		free(notification->icon);

	if (notification->time)
		free(notification->time);

	if (notification->title)
		free(notification->title);

	if (notification->content)
		free(notification->content);

	if (notification->service)
		service_destroy(notification->service);

	free(notification);

	return UI_NOTIFICATION_ERROR_NONE;
}


int ui_notification_set_icon(ui_notification_h notification, const char *path)
{
	char *path_dup;

	if (notification == NULL || path == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->posted == true)
	{
		LOGE("[%s] ALREADY_POSTED(0x%08x) : unable to change the notification", __FUNCTION__, UI_NOTIFICATION_ERROR_ALREADY_POSTED);
		return UI_NOTIFICATION_ERROR_ALREADY_POSTED;
	}

	path_dup = strdup(path);

	if (path_dup == NULL)
	{
		LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
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
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->icon != NULL)
	{
		path_dup = strdup(notification->icon);

		if (path_dup == NULL)
		{
			LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}
	}

	*path = path_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_set_time(ui_notification_h notification, struct tm *time)
{
	struct tm *time_dup;

	if (notification == NULL || time == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->posted == true)
	{
		LOGE("[%s] ALREADY_POSTED(0x%08x) : unable to change the notification", __FUNCTION__, UI_NOTIFICATION_ERROR_ALREADY_POSTED);
		return UI_NOTIFICATION_ERROR_ALREADY_POSTED;
	}

	time_dup = malloc(sizeof(struct tm));

	if (time_dup == NULL)
	{
		LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	memcpy(time_dup, time, sizeof(struct tm));

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
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->time != NULL)
	{
		time_dup = malloc(sizeof(struct tm));

		if (time_dup == NULL)
		{
			LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}
	
		memcpy(time_dup, notification->time, sizeof(struct tm));
	}

	*time = time_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}

int ui_notification_set_title(ui_notification_h notification, const char *title)
{
	char *title_dup;

	if (notification == NULL || title == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->posted == true)
	{
		LOGE("[%s] ALREADY_POSTED(0x%08x) : unable to change the notification", __FUNCTION__, UI_NOTIFICATION_ERROR_ALREADY_POSTED);
		return UI_NOTIFICATION_ERROR_ALREADY_POSTED;
	}

	title_dup = strdup(title);

	if (title_dup == NULL)
	{
		LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
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
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->title != NULL)
	{
		title_dup = strdup(notification->title);

		if (title_dup == NULL)
		{
			LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}
	}

	*title = title_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}


int ui_notification_set_content(ui_notification_h notification, const char *content)
{
	char *content_dup;

	if (notification == NULL || content == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->posted == true)
	{
		LOGE("[%s] ALREADY_POSTED(0x%08x) : unable to change the notification", __FUNCTION__, UI_NOTIFICATION_ERROR_ALREADY_POSTED);
		return UI_NOTIFICATION_ERROR_ALREADY_POSTED;
	}

	content_dup = strdup(content);

	if (content_dup == NULL)
	{
		LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
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
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->content != NULL)
	{
		content_dup = strdup(notification->content);

		if (content_dup == NULL)
		{
			LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}
	}

	*content = content_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}


int ui_notification_set_service(ui_notification_h notification, service_h service)
{
	int retcode;
	service_h service_dup;

	if (notification == NULL || service == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->posted == true)
	{
		LOGE("[%s] ALREADY_POSTED(0x%08x) : unable to change the notification", __FUNCTION__, UI_NOTIFICATION_ERROR_ALREADY_POSTED);
		return UI_NOTIFICATION_ERROR_ALREADY_POSTED;
	}

	retcode = service_clone(&service_dup, service);

	if (retcode != SERVICE_ERROR_NONE)
	{
		if (retcode == SERVICE_ERROR_OUT_OF_MEMORY)
		{
			LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
			return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
		}
		else
		{
			LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid service handle", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
			return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
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
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->service != NULL)
	{
		retcode = service_clone(&service_dup, notification->service);

		if (retcode != SERVICE_ERROR_NONE)
		{
			if (retcode == SERVICE_ERROR_OUT_OF_MEMORY)
			{
				LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
				return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
			}
			else
			{
				LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid service handle", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
				return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
			}
		}
	}

	*service = service_dup;

	return UI_NOTIFICATION_ERROR_NONE;
}


int ui_notification_post(ui_notification_h notification)
{
	int retcode;
	notification_h core;
	bundle *service_data;

	if (notification == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->posted == true)
	{
		LOGE("[%s] ALREADY_POSTED(0x%08x) : unable to change the notification", __FUNCTION__, UI_NOTIFICATION_ERROR_ALREADY_POSTED);
		return UI_NOTIFICATION_ERROR_ALREADY_POSTED;
	}

	// STEP 1: core handle
	if (notification->ongoing == true)
	{
		core = notification_new(NOTIFICATION_TYPE_ONGOING, NOTIFICATION_GROUP_ID_DEFAULT, NOTIFICATION_PRIV_ID_NONE);
	}
	else
	{
		core = notification_new(NOTIFICATION_TYPE_NOTI, NOTIFICATION_GROUP_ID_DEFAULT, NOTIFICATION_PRIV_ID_NONE);
	}

	if (core == NULL)
	{
		LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_OUT_OF_MEMORY);
		return UI_NOTIFICATION_ERROR_OUT_OF_MEMORY;
	}

	notification->core = core;

	// STEP 2: icon
	if (notification->icon != NULL)
	{
		struct stat st;
	
		if (stat(notification->icon, &st) < 0)
		{
			LOGE("[%s] NO_SUCH_FILE(0x%08x) : invalid icon", __FUNCTION__, UI_NOTIFICATION_ERROR_NO_SUCH_FILE);
			return UI_NOTIFICATION_ERROR_NO_SUCH_FILE;
		}

		notification_set_image(core, NOTIFICATION_IMAGE_TYPE_ICON, notification->icon);
	}

	// STEP 3: time
	if (notification->time != NULL)
	{
		notification_set_time(core, mktime(notification->time));
	}

	// STEP 4: title
	if (notification->title != NULL)
	{
		notification_set_text(core, NOTIFICATION_TEXT_TYPE_TITLE, notification->title, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	}

	// STEP 5: content
	if (notification->content != NULL)
	{
		notification_set_text(core, NOTIFICATION_TEXT_TYPE_CONTENT, notification->content, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	}

	// STEP 6: service
	if (notification->service != NULL && service_to_bundle(notification->service, &service_data) == SERVICE_ERROR_NONE)
	{
		notification_set_execute_option(core, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, service_data);
	}
	else
	{
		notification_set_property(core, NOTIFICATION_PROP_DISABLE_APP_LAUNCH);
	}

	// STEP 7: insert
	retcode = ui_notification_error_handler(notification_insert(core, NULL), __FUNCTION__, "failed to post a notification");

	if (retcode == UI_NOTIFICATION_ERROR_NONE)
	{
		notification->posted = true;
	}

	return retcode;
}

int ui_notification_update_progress(ui_notification_h notification, ui_notification_progress_type_e type, double value)
{
	int retcode;

	if (notification == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->core == NULL || notification->posted == false)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	switch (type)
	{
	case UI_NOTIFICATION_PROGRESS_TYPE_SIZE:
		retcode = ui_notification_error_handler(
			notification_update_size(notification->core, NOTIFICATION_PRIV_ID_NONE, value),
			__FUNCTION__, "failed to update the progress");
		break;
		
	case UI_NOTIFICATION_PROGRESS_TYPE_PERCENTAGE:
		retcode = ui_notification_error_handler(
			notification_update_progress(notification->core, NOTIFICATION_PRIV_ID_NONE, value),
			__FUNCTION__, "failed to update the progress");
		break;

	default:
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid progress type", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	return retcode;
}

int ui_notification_cancel(ui_notification_h notification)
{
	int retcode;

	if (notification == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	if (notification->core == NULL || notification->posted == false)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, UI_NOTIFICATION_ERROR_INVALID_PARAMETER);
		return UI_NOTIFICATION_ERROR_INVALID_PARAMETER;
	}

	retcode = ui_notification_error_handler(
		notification_delete(notification->core),
		__FUNCTION__, "failed to cancel the notification");

	return retcode;
}

int ui_notification_cancel_all(void)
{
	return ui_notification_error_handler(
		notification_delete_all_by_type(NULL, NOTIFICATION_TYPE_NONE),
		__FUNCTION__, "failed to cancel the notification");
}

