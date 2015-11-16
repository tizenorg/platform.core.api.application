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
#include <unistd.h>
#include <time.h>

#include <bundle.h>
#include <aul.h>
#include <alarm.h>
#include <dlog.h>

#include <app_internal.h>
#include <app_alarm.h>
#include <app_control_internal.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "CAPI_APPFW_APPLICATION_ALARM"

typedef struct {
	alarm_registered_alarm_cb cb;
	void* user_data;
	bool* foreach_break;
} alarm_foreach_item_cb_context;

static int alarm_registered_alarm_cb_broker(int alarm_id, void *user_data)
{
	alarm_foreach_item_cb_context* foreach_cb_context = NULL;

	if (user_data == NULL)
		return 0;

	foreach_cb_context = (alarm_foreach_item_cb_context*)user_data;

	if (foreach_cb_context != NULL && *(foreach_cb_context->foreach_break) == false) {
		if (foreach_cb_context->cb(alarm_id, foreach_cb_context->user_data) == false)
			*(foreach_cb_context->foreach_break) = true;
	}

	return 0;
}

static int convert_error_code_to_alarm(const char* function, alarm_error_t alarm_error)
{
	switch (alarm_error) {
	case ERR_ALARM_INVALID_PARAM:
	case ERR_ALARM_INVALID_REPEAT:
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", function, ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
		break;
	case ERR_ALARM_INVALID_ID:
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", function, ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
		break;
	case ERR_ALARM_INVALID_TIME:
		LOGE("[%s] INVALID_TIME(0x%08x)", function, ALARM_ERROR_INVALID_TIME);
		return ALARM_ERROR_INVALID_TIME;
		break;
	case ERR_ALARM_INVALID_DATE:
		LOGE("[%s] INVALID_DATE(0x%08x)", function, ALARM_ERROR_INVALID_DATE);
		return ALARM_ERROR_INVALID_DATE;
		break;
	case ERR_ALARM_NO_SERVICE_NAME:
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", function, ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
		break;
	case ERR_ALARM_SYSTEM_FAIL:
		LOGE("[%s] CONNECTION_FAIL(0x%08x)", function, ALARM_ERROR_CONNECTION_FAIL);
		return ALARM_ERROR_CONNECTION_FAIL;
		break;
	case ERR_ALARM_NO_PERMISSION:
		LOGE("[%s] PERMISSION_DENIED(0x%08x)", function, ALARM_ERROR_PERMISSION_DENIED);
		return ALARM_ERROR_PERMISSION_DENIED;
		break;
	case ERR_ALARM_NOT_PERMITTED_APP:
		LOGE("[%s] NOT_PERMITTED_APP(0x%08x)", function, ALARM_ERROR_NOT_PERMITTED_APP);
		return ALARM_ERROR_NOT_PERMITTED_APP;
		break;
	case ALARMMGR_RESULT_SUCCESS:
		return ALARM_ERROR_NONE;
		break;
	default:
		return ALARM_ERROR_INVALID_PARAMETER;
	}
}

int alarm_get_scheduled_date(int alarm_id, struct tm* date)
{
	alarm_error_t result;
	time_t due_time = 0;

	if (date == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
	}

	result = alarmmgr_get_next_duetime(alarm_id, &due_time);
	if (result != ALARMMGR_RESULT_SUCCESS)
		return convert_error_code_to_alarm(__FUNCTION__, result);

	localtime_r(&due_time, date);

	return ALARM_ERROR_NONE;
}

int alarm_get_scheduled_period(int alarm_id, int* period)
{
	alarm_error_t result;
	alarm_entry_t *entry = NULL;
	alarm_repeat_mode_t mode;
	int value;

	if (period == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
	}

	entry = alarmmgr_create_alarm();

	result = alarmmgr_get_info(alarm_id, entry);
	if (result != ALARMMGR_RESULT_SUCCESS) {
		if (entry != NULL)
			alarmmgr_free_alarm(entry);

		return convert_error_code_to_alarm(__FUNCTION__, result);
	}

	result = alarmmgr_get_repeat_mode(entry, &mode, &value);
	if (result != ALARMMGR_RESULT_SUCCESS) {
		if (entry != NULL)
			alarmmgr_free_alarm(entry);

		return convert_error_code_to_alarm(__FUNCTION__, result);
	}

	result = alarmmgr_free_alarm(entry);
	if (result != ALARMMGR_RESULT_SUCCESS) {
		if (entry != NULL)
			alarmmgr_free_alarm(entry);

		return convert_error_code_to_alarm(__FUNCTION__, result);
	}

	(*period) = value;

	return ALARM_ERROR_NONE;
}

int alarm_schedule_after_delay(app_control_h app_control, int delay, int period, int *alarm_id)
{
	bundle *bundle_data;
	int result = 0;

	if (app_control == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
	}

	if (app_control_to_bundle(app_control, &bundle_data) != APP_CONTROL_ERROR_NONE) {
		LOGE("INVALID_PARAMETER(0x%08x)", ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
	}

	result = alarmmgr_add_alarm_appsvc(ALARM_TYPE_DEFAULT | ALARM_TYPE_INEXACT, delay, period, bundle_data, alarm_id);

	return  convert_error_code_to_alarm(__FUNCTION__, result);
}

int alarm_schedule_at_date(app_control_h app_control, struct tm *date, int period_in_second, int *alarm_id)
{
	alarm_date_t internal_time;
	alarm_entry_t* alarm_info;
	bundle *bundle_data;
	int result;

	if (app_control == NULL || date == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
	}

	if (app_control_to_bundle(app_control, &bundle_data) != APP_CONTROL_ERROR_NONE) {
		LOGE("INVALID_PARAMETER(0x%08x)", ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
	}

	alarm_info = alarmmgr_create_alarm();

	internal_time.year = date->tm_year + 1900;
	internal_time.month = date->tm_mon + 1;
	internal_time.day = date->tm_mday;

	internal_time.hour = date->tm_hour;
	internal_time.min = date->tm_min;
	internal_time.sec = date->tm_sec;

	result = alarmmgr_set_time(alarm_info, internal_time);

	if (result < 0) {
		alarmmgr_free_alarm(alarm_info);
		return convert_error_code_to_alarm(__FUNCTION__, result);
	}


	if (period_in_second > 0)
		result = alarmmgr_set_repeat_mode(alarm_info, ALARM_REPEAT_MODE_REPEAT, period_in_second);
	else
		result = alarmmgr_set_repeat_mode(alarm_info, ALARM_REPEAT_MODE_ONCE, period_in_second);

	if (result < 0) {
		alarmmgr_free_alarm(alarm_info);
		return convert_error_code_to_alarm(__FUNCTION__, result);
	}

	result = alarmmgr_set_type(alarm_info, ALARM_TYPE_DEFAULT);
	if (result < 0) {
		alarmmgr_free_alarm(alarm_info);
		return convert_error_code_to_alarm(__FUNCTION__, result);
	}

	result = alarmmgr_add_alarm_appsvc_with_localtime(alarm_info, bundle_data, alarm_id);
	if (result < 0) {
		alarmmgr_free_alarm(alarm_info);
		return convert_error_code_to_alarm(__FUNCTION__, result);
	}

	alarmmgr_free_alarm(alarm_info);
	return ALARM_ERROR_NONE;
}

int alarm_schedule_once_after_delay(app_control_h app_control, int delay, int *alarm_id)
{
	bundle *bundle_data;
	int result = 0;

	if (app_control == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
	}

	if (app_control_to_bundle(app_control, &bundle_data) != APP_CONTROL_ERROR_NONE) {
		LOGE("INVALID_PARAMETER(0x%08x)", ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
	}

	result = alarmmgr_add_alarm_appsvc(ALARM_TYPE_DEFAULT, delay, 0, bundle_data, alarm_id);

	return  convert_error_code_to_alarm(__FUNCTION__, result);
}

int alarm_schedule_once_at_date(app_control_h app_control, struct tm *date, int *alarm_id)
{
	alarm_date_t internal_time;
	alarm_entry_t* alarm_info;
	bundle *bundle_data;
	int result;

	if (app_control == NULL || date == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
	}

	if (app_control_to_bundle(app_control, &bundle_data) != APP_CONTROL_ERROR_NONE) {
		LOGE("INVALID_PARAMETER(0x%08x)", ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
	}

	alarm_info = alarmmgr_create_alarm();
	if (alarm_info == NULL) {
		LOGE("OUT_OF_MEMORY(0x%08x)", ALARM_ERROR_OUT_OF_MEMORY);
		return ALARM_ERROR_OUT_OF_MEMORY;
	}

	internal_time.year = date->tm_year + 1900;
	internal_time.month = date->tm_mon + 1;
	internal_time.day = date->tm_mday;

	internal_time.hour = date->tm_hour;
	internal_time.min = date->tm_min;
	internal_time.sec = date->tm_sec;

	result = alarmmgr_set_time(alarm_info, internal_time);

	if (result < 0) {
		alarmmgr_free_alarm(alarm_info);
		return convert_error_code_to_alarm(__FUNCTION__, result);
	}

	result = alarmmgr_set_repeat_mode(alarm_info, ALARM_REPEAT_MODE_ONCE, 0);
	if (result < 0) {
		alarmmgr_free_alarm(alarm_info);
		return convert_error_code_to_alarm(__FUNCTION__, result);
	}

	result = alarmmgr_set_type(alarm_info, ALARM_TYPE_DEFAULT);
	if (result < 0) {
		alarmmgr_free_alarm(alarm_info);
		return convert_error_code_to_alarm(__FUNCTION__, result);
	}

	result = alarmmgr_add_alarm_appsvc_with_localtime(alarm_info, bundle_data, alarm_id);
	if (result < 0) {
		alarmmgr_free_alarm(alarm_info);
		return convert_error_code_to_alarm(__FUNCTION__, result);
	}

	alarmmgr_free_alarm(alarm_info);
	return ALARM_ERROR_NONE;
}

int alarm_cancel(int alarm_id)
{
	int result;

	result = alarmmgr_remove_alarm(alarm_id);

	return convert_error_code_to_alarm(__FUNCTION__, result);
}

int alarm_cancel_all()
{
	int result;

	result = alarmmgr_remove_all();

	return convert_error_code_to_alarm(__FUNCTION__, result);
}

int alarm_foreach_registered_alarm(alarm_registered_alarm_cb callback, void* user_data)
{
	int result;
	bool foreach_break = false;

	if (callback == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
	}

	/* alarm_registered_alarm_cb_broker */
	alarm_foreach_item_cb_context foreach_cb_context = {
		.cb = callback,
		.user_data = user_data,
		.foreach_break = &foreach_break
	};

	result = alarmmgr_enum_alarm_ids(alarm_registered_alarm_cb_broker, &foreach_cb_context);

	return convert_error_code_to_alarm(__FUNCTION__, result);
}

int alarm_get_current_time(struct tm* date)
{
	time_t now;

	if (date == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
	}

	time(&now);
	localtime_r(&now, date);
	return ALARM_ERROR_NONE;
}

int alarm_schedule_with_recurrence_week_flag(app_control_h app_control, struct tm *date, int week_flag, int *alarm_id)
{
	alarm_date_t internal_time;
	alarm_entry_t* alarm_info;
	bundle *bundle_data;
	int result;

	if (app_control == NULL || date == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
	}

	if (app_control_to_bundle(app_control, &bundle_data) != APP_CONTROL_ERROR_NONE) {
		LOGE("INVALID_PARAMETER(0x%08x)", ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
	}

	alarm_info = alarmmgr_create_alarm();

	internal_time.year = date->tm_year + 1900;
	internal_time.month = date->tm_mon + 1;
	internal_time.day = date->tm_mday;

	internal_time.hour = date->tm_hour;
	internal_time.min = date->tm_min;
	internal_time.sec = date->tm_sec;

	result = alarmmgr_set_time(alarm_info, internal_time);
	if (result < 0) {
		alarmmgr_free_alarm(alarm_info);
		return convert_error_code_to_alarm(__FUNCTION__, result);
	}

	if (week_flag > 0)
		result = alarmmgr_set_repeat_mode(alarm_info, ALARM_REPEAT_MODE_WEEKLY, week_flag);

	if (result < 0) {
		alarmmgr_free_alarm(alarm_info);
		return convert_error_code_to_alarm(__FUNCTION__, result);
	}

	result = alarmmgr_set_type(alarm_info, ALARM_TYPE_DEFAULT);
	if (result < 0) {
		alarmmgr_free_alarm(alarm_info);
		return convert_error_code_to_alarm(__FUNCTION__, result);
	}

	result = alarmmgr_add_alarm_appsvc_with_localtime(alarm_info, bundle_data, alarm_id);
	alarmmgr_free_alarm(alarm_info);

	return convert_error_code_to_alarm(__FUNCTION__, result);
}

int alarm_get_scheduled_recurrence_week_flag(int alarm_id, int *week_flag)
{
	alarm_error_t result;
	alarm_entry_t *entry = NULL;
	alarm_repeat_mode_t mode;
	int value;

	if (week_flag == NULL) {
		LOGE("INVALID_PARAMETER(0x%08x)", ALARM_ERROR_INVALID_PARAMETER);
		return ALARM_ERROR_INVALID_PARAMETER;
	}

	entry = alarmmgr_create_alarm();

	result = alarmmgr_get_info(alarm_id, entry);
	if (result != ALARMMGR_RESULT_SUCCESS) {
		if (entry != NULL)
			alarmmgr_free_alarm(entry);

		return convert_error_code_to_alarm(__FUNCTION__, result);
	}

	result = alarmmgr_get_repeat_mode(entry, &mode, &value);

	if (mode != ALARM_REPEAT_MODE_WEEKLY) {
		if (entry != NULL)
			alarmmgr_free_alarm(entry);

		return ALARM_ERROR_INVALID_PARAMETER;
	}

	if (result != ALARMMGR_RESULT_SUCCESS) {
		if (entry != NULL)
			alarmmgr_free_alarm(entry);

		return convert_error_code_to_alarm(__FUNCTION__, result);
	}

	result = alarmmgr_free_alarm(entry);
	if (result != ALARMMGR_RESULT_SUCCESS) {
		if (entry != NULL)
			alarmmgr_free_alarm(entry);

		return convert_error_code_to_alarm(__FUNCTION__, result);
	}

	(*week_flag) = value;

	return ALARM_ERROR_NONE;
}

int alarm_get_app_control(int alarm_id, app_control_h *app_control)
{
	bundle *b = NULL;
	int error_code = 0;

	if (app_control == NULL)
		return ALARM_ERROR_INVALID_PARAMETER;

	b = alarmmgr_get_alarm_appsvc_info(alarm_id, &error_code);
	if (error_code != ALARMMGR_RESULT_SUCCESS)
		return convert_error_code_to_alarm(__FUNCTION__, error_code);

	if (b == NULL)
		return ALARM_ERROR_INVALID_PARAMETER;

	error_code = app_control_create_request(b, app_control);
	if (error_code != APP_CONTROL_ERROR_NONE) {
		bundle_free(b);
		return ALARM_ERROR_OUT_OF_MEMORY;
	}

	bundle_free(b);

	return ALARM_ERROR_NONE;
}
