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

#include <string.h>

#include <vconf-internal-keys.h>
#include <app_common.h>
#include <app_private.h>

app_device_orientation_e app_convert_appcore_rm(enum appcore_rm rm)
{
	app_device_orientation_e dev_orientation;

	switch (rm)
	{
	case APPCORE_RM_PORTRAIT_NORMAL:
		dev_orientation = APP_DEVICE_ORIENTATION_0;
		break;

	case APPCORE_RM_PORTRAIT_REVERSE:
		dev_orientation = APP_DEVICE_ORIENTATION_180;
		break;

	case APPCORE_RM_LANDSCAPE_NORMAL:
		dev_orientation = APP_DEVICE_ORIENTATION_270;
		break;

	case APPCORE_RM_LANDSCAPE_REVERSE:
		dev_orientation = APP_DEVICE_ORIENTATION_90;
		break;

	default:
		dev_orientation = APP_DEVICE_ORIENTATION_0;
		break;
	}

	return dev_orientation;
}

static int _app_convert_low_memory(void *val)
{
	switch (*(int *)val) {
	case VCONFKEY_SYSMAN_LOW_MEMORY_NORMAL:
		return APP_EVENT_LOW_MEMORY_NORMAL;
	case VCONFKEY_SYSMAN_LOW_MEMORY_SOFT_WARNING:
		return APP_EVENT_LOW_MEMORY_SOFT_WARNING;
	case VCONFKEY_SYSMAN_LOW_MEMORY_HARD_WARNING:
		return APP_EVENT_LOW_MEMORY_HARD_WARNING;
	default:
		return -1;
	}
}

static int _app_convert_low_battery(void *val)
{
	switch (*(int *)val) {
	case VCONFKEY_SYSMAN_BAT_POWER_OFF:
		return APP_EVENT_LOW_BATTERY_POWER_OFF;
	case VCONFKEY_SYSMAN_BAT_CRITICAL_LOW:
		return APP_EVENT_LOW_BATTERY_CRITICAL_LOW;
	default:
		return -1;
	}
}

int app_event_get_low_memory_status(app_event_info_h event_info, app_event_low_memory_status_e *status)
{
	int ret;

	if (event_info == NULL || status == NULL)
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "null parameter");

	if (event_info->type != APP_EVENT_LOW_MEMORY)
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "event type mismatching");

	ret = _app_convert_low_memory(event_info->value);
	if (ret < 0)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "invalid event info");

	*status = ret;

	return APP_ERROR_NONE;
}

int app_event_get_low_battery_status(app_event_info_h event_info, app_event_low_battery_status_e *status)
{
	int ret;

	if (event_info == NULL || status == NULL)
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "null parameter");

	if (event_info->type != APP_EVENT_LOW_BATTERY)
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "event type mismatching");

	ret = _app_convert_low_battery(event_info->value);
	if (ret < 0)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "invalid event info");

	*status = ret;

	return APP_ERROR_NONE;
}

int app_event_get_language(app_event_info_h event_info, char **lang)
{
	if (event_info == NULL || lang == NULL)
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "null parameter");

	if (event_info->type != APP_EVENT_LANGUAGE_CHANGED)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "event type mismatching");

	*lang = strdup(event_info->value);

	return APP_ERROR_NONE;
}

int app_event_get_region_format(app_event_info_h event_info, char **region)
{
	if (event_info == NULL || region == NULL)
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "null parameter");

	if (event_info->type != APP_EVENT_REGION_FORMAT_CHANGED)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "event type mismatching");

	*region = strdup(event_info->value);

	return APP_ERROR_NONE;
}

int app_event_get_device_orientation(app_event_info_h event_info, app_device_orientation_e *orientation)
{
	if (event_info == NULL || orientation == NULL)
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "null parameter");

	if (event_info->type != APP_EVENT_DEVICE_ORIENTATION_CHANGED)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "event type mismatching");

	*orientation = app_convert_appcore_rm(*(enum appcore_rm *)(event_info->value));

	return APP_ERROR_NONE;
}
