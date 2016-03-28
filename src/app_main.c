/*
 * Copyright (c) 2011 - 2016 Samsung Electronics Co., Ltd All Rights Reserved
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
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <bundle.h>
#include <appcore-common.h>
#include <appcore-efl.h>
#include <aul.h>
#include <dlog.h>

#include <Elementary.h>

#include <app_internal.h>
#include <app_control_internal.h>
#include <tizen_error.h>

#include "app_extension.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "CAPI_APPFW_APPLICATION"

typedef enum {
	APP_STATE_NOT_RUNNING, /* The application has been launched or was running but was terminated */
	APP_STATE_CREATING, /* The application is initializing the resources on app_create_cb callback */
	APP_STATE_RUNNING, /* The application is running in the foreground and background */
} app_state_e;

typedef struct {
	char *package;
	char *app_name;
	app_state_e state;
	app_event_callback_s *callback;
	void *data;
} app_context_s;

typedef app_context_s *app_context_h;

static int app_appcore_create(void *data);
static int app_appcore_pause(void *data);
static int app_appcore_resume(void *data);
static int app_appcore_terminate(void *data);
static int app_appcore_reset(bundle *appcore_bundle, void *data);

static int app_appcore_low_memory(void *event, void *data);
static int app_appcore_low_battery(void *event, void *data);
static int app_appcore_rotation_event(void *event, enum appcore_rm rm, void *data);
static int app_appcore_lang_changed(void *event, void *data);
static int app_appcore_region_changed(void *event, void *data);

static void app_set_appcore_event_cb(app_context_h app_context);
static void app_unset_appcore_event_cb(void);

int app_main(int argc, char **argv, app_event_callback_s *callback, void *user_data)
{
	return app_efl_main(&argc, &argv, callback, user_data);
}

int app_efl_main(int *argc, char ***argv, app_event_callback_s *callback, void *user_data)
{
	app_context_s app_context = {
		.package = NULL,
		.app_name = NULL,
		.state = APP_STATE_NOT_RUNNING,
		.callback = callback,
		.data = user_data
	};

	struct appcore_ops appcore_context = {
		.data = &app_context,
		.create = app_appcore_create,
		.terminate = app_appcore_terminate,
		.pause = app_appcore_pause,
		.resume = app_appcore_resume,
		.reset = app_appcore_reset,
	};

	if (argc == NULL || argv == NULL || callback == NULL)
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (callback->create == NULL)
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "app_create_cb() callback must be registered");

	if (app_context.state != APP_STATE_NOT_RUNNING)
		return app_error(APP_ERROR_ALREADY_RUNNING, __FUNCTION__, NULL);

	if (app_get_id(&(app_context.package)) != APP_ERROR_NONE)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "failed to get the package");

	if (app_get_package_app_name(app_context.package, &(app_context.app_name)) != APP_ERROR_NONE) {
		free(app_context.package);
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "failed to get the package's app name");
	}

	app_context.state = APP_STATE_CREATING;

	appcore_efl_main(app_context.app_name, argc, argv, &appcore_context);

	free(app_context.package);
	free(app_context.app_name);

	return APP_ERROR_NONE;
}

void app_exit(void)
{
	app_efl_exit();
}

void app_efl_exit(void)
{
	elm_exit();
}

int app_appcore_create(void *data)
{
	app_context_h app_context = data;
	app_create_cb create_cb;
	char locale_dir[TIZEN_PATH_MAX] = {0, };

	if (app_context == NULL)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);

	app_set_appcore_event_cb(app_context);

	snprintf(locale_dir, TIZEN_PATH_MAX, "%s/%s" PATH_FMT_RES_DIR
			PATH_FMT_LOCALE_DIR, PATH_FMT_APP_ROOT, app_context->package);
	if (access(locale_dir, R_OK) != 0) {
		snprintf(locale_dir, TIZEN_PATH_MAX, "%s/%s" PATH_FMT_RO_RES_DIR
				PATH_FMT_RO_LOCALE_DIR, PATH_FMT_RO_APP_ROOT, app_context->package);
	}
	appcore_set_i18n(app_context->app_name, locale_dir);

	create_cb = app_context->callback->create;
	if (create_cb == NULL || create_cb(app_context->data) == false)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "app_create_cb() returns false");

	app_context->state = APP_STATE_RUNNING;

	return APP_ERROR_NONE;
}

int app_appcore_terminate(void *data)
{
	app_context_h app_context = data;
	app_terminate_cb terminate_cb;

	if (app_context == NULL)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);

	terminate_cb = app_context->callback->terminate;

	if (terminate_cb != NULL)
		terminate_cb(app_context->data);

	app_unset_appcore_event_cb();

	app_finalizer_execute();

	return APP_ERROR_NONE;
}

int app_appcore_pause(void *data)
{
	app_context_h app_context = data;
	app_pause_cb pause_cb;

	if (app_context == NULL)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);

	pause_cb = app_context->callback->pause;
	if (pause_cb != NULL)
		pause_cb(app_context->data);

	return APP_ERROR_NONE;
}

int app_appcore_resume(void *data)
{
	app_context_h app_context = data;
	app_resume_cb resume_cb;

	if (app_context == NULL)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);

	resume_cb = app_context->callback->resume;
	if (resume_cb != NULL)
		resume_cb(app_context->data);

	return APP_ERROR_NONE;
}

int app_appcore_reset(bundle *appcore_bundle, void *data)
{
	app_context_h app_context = data;
	app_control_cb callback;
	app_control_h app_control;

	if (app_context == NULL)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);

	if (app_control_create_event(appcore_bundle, &app_control) != APP_ERROR_NONE)
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "failed to create a service handle from the bundle");

	callback = app_context->callback->app_control;
	if (callback != NULL)
		callback(app_control, app_context->data);

	app_control_destroy(app_control);

	return APP_ERROR_NONE;
}

int app_appcore_low_memory(void *event_info, void *data)
{
	app_context_h app_context = data;
	app_low_memory_cb low_memory_cb;

	if (app_context == NULL)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);

	low_memory_cb = app_context->callback->low_memory;
	if (low_memory_cb != NULL)
		low_memory_cb(app_context->data);

	return APP_ERROR_NONE;
}

int app_appcore_low_battery(void *event_info, void *data)
{
	app_context_h app_context = data;
	app_low_battery_cb low_battery_cb;

	if (app_context == NULL)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);

	low_battery_cb = app_context->callback->low_battery;
	if (low_battery_cb != NULL)
		low_battery_cb(app_context->data);

	return APP_ERROR_NONE;
}

int app_appcore_rotation_event(void *event_info, enum appcore_rm rm, void *data)
{
	app_context_h app_context = data;
	app_device_orientation_cb device_orientation_cb;

	if (app_context == NULL)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);

	device_orientation_cb = app_context->callback->device_orientation;
	if (device_orientation_cb != NULL) {
		app_device_orientation_e dev_orientation;

		dev_orientation = app_convert_appcore_rm(rm);

		device_orientation_cb(dev_orientation, app_context->data);
	}

	return APP_ERROR_NONE;
}

int app_appcore_lang_changed(void *event_info, void *data)
{
	app_context_h app_context = data;
	app_language_changed_cb lang_changed_cb;

	if (app_context == NULL)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);

	lang_changed_cb = app_context->callback->language_changed;
	if (lang_changed_cb != NULL)
		lang_changed_cb(app_context->data);

	return APP_ERROR_NONE;
}

int app_appcore_region_changed(void *event_info, void *data)
{
	app_context_h app_context = data;
	app_region_format_changed_cb region_changed_cb;

	if (app_context == NULL)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);

	region_changed_cb = app_context->callback->region_format_changed;
	if (region_changed_cb != NULL)
		region_changed_cb(app_context->data);

	return APP_ERROR_NONE;
}

void app_set_appcore_event_cb(app_context_h app_context)
{
	if (app_context->callback->low_memory != NULL)
		appcore_set_event_callback(APPCORE_EVENT_LOW_MEMORY, app_appcore_low_memory, app_context);

	if (app_context->callback->low_battery != NULL)
		appcore_set_event_callback(APPCORE_EVENT_LOW_BATTERY, app_appcore_low_battery, app_context);

	if (app_context->callback->device_orientation != NULL)
		appcore_set_rotation_cb(app_appcore_rotation_event, app_context);

	if (app_context->callback->language_changed != NULL)
		appcore_set_event_callback(APPCORE_EVENT_LANG_CHANGE, app_appcore_lang_changed, app_context);

	if (app_context->callback->region_format_changed != NULL)
		appcore_set_event_callback(APPCORE_EVENT_REGION_CHANGE, app_appcore_region_changed, app_context);
}

void app_unset_appcore_event_cb(void)
{
	appcore_set_event_callback(APPCORE_EVENT_LOW_MEMORY, NULL, NULL);
	appcore_set_event_callback(APPCORE_EVENT_LOW_BATTERY, NULL, NULL);
	appcore_unset_rotation_cb();
	appcore_set_event_callback(APPCORE_EVENT_LANG_CHANGE, NULL, NULL);
	appcore_set_event_callback(APPCORE_EVENT_REGION_CHANGE, NULL, NULL);
}

#define UI_APP_EVENT_MAX 6
static Eina_List *handler_list[UI_APP_EVENT_MAX] = {NULL, };
static int handler_initialized = 0;
static int appcore_initialized = 0;

struct ui_app_context {
	char *package;
	char *app_name;
	app_state_e state;
	ui_app_lifecycle_callback_s *callback;
	void *data;
};

static void _free_handler_list(void)
{
	int i;
	app_event_handler_h handler;

	for (i = 0; i < UI_APP_EVENT_MAX; i++) {
		EINA_LIST_FREE(handler_list[i], handler)
			if (handler)
				free(handler);
	}

	eina_shutdown();
}

static int _ui_app_appcore_low_memory(void *event_info, void *data)
{
	Eina_List *l;
	app_event_handler_h handler;
	struct app_event_info event;

	LOGI("_app_appcore_low_memory");

	event.type = APP_EVENT_LOW_MEMORY;
	event.value = event_info;

	EINA_LIST_FOREACH(handler_list[APP_EVENT_LOW_MEMORY], l, handler) {
		handler->cb(&event, handler->data);
	}

	return APP_ERROR_NONE;
}

static int _ui_app_appcore_low_battery(void *event_info, void *data)
{
	Eina_List *l;
	app_event_handler_h handler;
	struct app_event_info event;

	LOGI("_ui_app_appcore_low_battery");

	event.type = APP_EVENT_LOW_BATTERY;
	event.value = event_info;

	EINA_LIST_FOREACH(handler_list[APP_EVENT_LOW_BATTERY], l, handler) {
		handler->cb(&event, handler->data);
	}

	return APP_ERROR_NONE;
}

static int _ui_app_appcore_rotation_event(void *event_info, enum appcore_rm rm, void *data)
{
	Eina_List *l;
	app_event_handler_h handler;
	struct app_event_info event;

	LOGI("_ui_app_appcore_rotation_event");

	event.type = APP_EVENT_DEVICE_ORIENTATION_CHANGED;
	event.value = event_info;

	EINA_LIST_FOREACH(handler_list[APP_EVENT_DEVICE_ORIENTATION_CHANGED], l, handler) {
		handler->cb(&event, handler->data);
	}

	return APP_ERROR_NONE;
}

static int _ui_app_appcore_lang_changed(void *event_info, void *data)
{
	Eina_List *l;
	app_event_handler_h handler;
	struct app_event_info event;

	LOGI("_ui_app_appcore_lang_changed");

	event.type = APP_EVENT_LANGUAGE_CHANGED;
	event.value = event_info;

	EINA_LIST_FOREACH(handler_list[APP_EVENT_LANGUAGE_CHANGED], l, handler) {
		handler->cb(&event, handler->data);
	}

	return APP_ERROR_NONE;
}

static int _ui_app_appcore_region_changed(void *event_info, void *data)
{
	Eina_List *l;
	app_event_handler_h handler;
	struct app_event_info event;

	if (event_info == NULL) {
		LOGI("receive empty event, ignore it");
		return APP_ERROR_NONE;
	}

	LOGI("_ui_app_appcore_region_changed");

	event.type = APP_EVENT_REGION_FORMAT_CHANGED;
	event.value = event_info;

	EINA_LIST_FOREACH(handler_list[APP_EVENT_REGION_FORMAT_CHANGED], l, handler) {
		handler->cb(&event, handler->data);
	}

	return APP_ERROR_NONE;
}

static int _ui_app_appcore_suspended_state_changed(void *event_info, void *data)
{
	Eina_List *l;
	app_event_handler_h handler;
	struct app_event_info event;

	LOGI("_ui_app_appcore_suspended_state_changed");
	LOGI("[__SUSPEND__] suspended state: %d (0: suspend, 1: wake)", *(int *)event_info);

	event.type = APP_EVENT_SUSPENDED_STATE_CHANGED;
	event.value = event_info;

	EINA_LIST_FOREACH(handler_list[APP_EVENT_SUSPENDED_STATE_CHANGED], l, handler) {
		handler->cb(&event, handler->data);
	}

	return APP_ERROR_NONE;
}

static void _ui_app_appcore_set_event_cb(app_event_type_e event_type)
{
	switch (event_type) {
	case APP_EVENT_LOW_MEMORY:
		appcore_set_event_callback(APPCORE_EVENT_LOW_MEMORY, _ui_app_appcore_low_memory, NULL);
		break;
	case APP_EVENT_LOW_BATTERY:
		appcore_set_event_callback(APPCORE_EVENT_LOW_BATTERY, _ui_app_appcore_low_battery, NULL);
		break;
	case APP_EVENT_LANGUAGE_CHANGED:
		appcore_set_event_callback(APPCORE_EVENT_LANG_CHANGE, _ui_app_appcore_lang_changed, NULL);
		break;
	case APP_EVENT_DEVICE_ORIENTATION_CHANGED:
		appcore_set_rotation_cb(_ui_app_appcore_rotation_event, NULL);
		break;
	case APP_EVENT_REGION_FORMAT_CHANGED:
		appcore_set_event_callback(APPCORE_EVENT_REGION_CHANGE, _ui_app_appcore_region_changed, NULL);
		break;
	case APP_EVENT_SUSPENDED_STATE_CHANGED:
		LOGI("[__SUSPEND__]");
		appcore_set_event_callback(APPCORE_EVENT_SUSPENDED_STATE_CHANGE, _ui_app_appcore_suspended_state_changed, NULL);
		break;
	default:
		break;
	}
}

static void _ui_app_appcore_unset_event_cb(app_event_type_e event_type)
{
	switch (event_type) {
	case APP_EVENT_LOW_MEMORY:
		appcore_set_event_callback(APPCORE_EVENT_LOW_MEMORY, NULL, NULL);
		break;
	case APP_EVENT_LOW_BATTERY:
		appcore_set_event_callback(APPCORE_EVENT_LOW_BATTERY, NULL, NULL);
		break;
	case APP_EVENT_LANGUAGE_CHANGED:
		appcore_set_event_callback(APPCORE_EVENT_LANG_CHANGE, NULL, NULL);
		break;
	case APP_EVENT_DEVICE_ORIENTATION_CHANGED:
		appcore_unset_rotation_cb();
		break;
	case APP_EVENT_REGION_FORMAT_CHANGED:
		appcore_set_event_callback(APPCORE_EVENT_REGION_CHANGE, NULL, NULL);
		break;
	case APP_EVENT_SUSPENDED_STATE_CHANGED:
		LOGI("[__SUSPEND__]");
		appcore_set_event_callback(APPCORE_EVENT_SUSPENDED_STATE_CHANGE, NULL, NULL);
		break;
	default:
		break;
	}
}

static void _ui_app_set_appcore_event_cb(void)
{
	_ui_app_appcore_set_event_cb(APP_EVENT_LOW_MEMORY);
	_ui_app_appcore_set_event_cb(APP_EVENT_LANGUAGE_CHANGED);
	_ui_app_appcore_set_event_cb(APP_EVENT_REGION_FORMAT_CHANGED);

	if (eina_list_count(handler_list[APP_EVENT_LOW_BATTERY]) > 0)
		_ui_app_appcore_set_event_cb(APP_EVENT_LOW_BATTERY);
	if (eina_list_count(handler_list[APP_EVENT_DEVICE_ORIENTATION_CHANGED]) > 0)
		_ui_app_appcore_set_event_cb(APP_EVENT_DEVICE_ORIENTATION_CHANGED);
	if (eina_list_count(handler_list[APP_EVENT_SUSPENDED_STATE_CHANGED]) > 0)
		_ui_app_appcore_set_event_cb(APP_EVENT_SUSPENDED_STATE_CHANGED);
}

static void _ui_app_unset_appcore_event_cb(void)
{
	_ui_app_appcore_unset_event_cb(APP_EVENT_LOW_MEMORY);
	_ui_app_appcore_unset_event_cb(APP_EVENT_LANGUAGE_CHANGED);
	_ui_app_appcore_unset_event_cb(APP_EVENT_REGION_FORMAT_CHANGED);

	if (eina_list_count(handler_list[APP_EVENT_LOW_BATTERY]) > 0)
		_ui_app_appcore_unset_event_cb(APP_EVENT_LOW_BATTERY);
	if (eina_list_count(handler_list[APP_EVENT_DEVICE_ORIENTATION_CHANGED]) > 0)
		_ui_app_appcore_unset_event_cb(APP_EVENT_DEVICE_ORIENTATION_CHANGED);
	if (eina_list_count(handler_list[APP_EVENT_SUSPENDED_STATE_CHANGED]) > 0)
		_ui_app_appcore_unset_event_cb(APP_EVENT_SUSPENDED_STATE_CHANGED);
}

static int _ui_app_appcore_create(void *data)
{
	LOGI("app_appcore_create");
	struct ui_app_context *app_context = data;
	app_create_cb create_cb;

	if (app_context == NULL)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);

	appcore_initialized = 1;
	_ui_app_set_appcore_event_cb();

	create_cb = app_context->callback->create;

	if (create_cb == NULL || create_cb(app_context->data) == false)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "app_create_cb() returns false");

	app_context->state = APP_STATE_RUNNING;

	return APP_ERROR_NONE;
}

static int _ui_app_appcore_terminate(void *data)
{
	LOGI("app_appcore_terminate");
	struct ui_app_context *app_context = data;
	app_terminate_cb terminate_cb;

	if (app_context == NULL)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);

	terminate_cb = app_context->callback->terminate;

	if (terminate_cb != NULL)
		terminate_cb(app_context->data);

	_ui_app_unset_appcore_event_cb();

	app_finalizer_execute();

	if (handler_initialized) {
		_free_handler_list();
		handler_initialized = 0;
	}

	return APP_ERROR_NONE;
}

static int _ui_app_appcore_pause(void *data)
{
	LOGI("app_appcore_pause");
	struct ui_app_context *app_context = data;
	app_pause_cb pause_cb;

	if (app_context == NULL)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);

	pause_cb = app_context->callback->pause;

	if (pause_cb != NULL)
		pause_cb(app_context->data);

	return APP_ERROR_NONE;
}

static int _ui_app_appcore_resume(void *data)
{
	LOGI("app_appcore_resume");
	struct ui_app_context *app_context = data;
	app_resume_cb resume_cb;

	if (app_context == NULL)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);

	resume_cb = app_context->callback->resume;

	if (resume_cb != NULL)
		resume_cb(app_context->data);

	return APP_ERROR_NONE;
}


static int _ui_app_appcore_reset(bundle *appcore_bundle, void *data)
{
	LOGI("app_appcore_reset");
	struct ui_app_context *app_context = data;
	app_control_cb callback;
	app_control_h app_control;
	int ret;

	if (app_context == NULL)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);

	if (appcore_bundle) {
		if (app_control_create_event(appcore_bundle, &app_control) != APP_ERROR_NONE)
			return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "failed to create an app_control handle from the bundle");
	} else {
		ret = app_control_create(&app_control);
		if (ret != APP_ERROR_NONE)
			return app_error(APP_ERROR_OUT_OF_MEMORY, __FUNCTION__, "failed to create an app_control");
	}

	callback = app_context->callback->app_control;

	if (callback != NULL)
		callback(app_control, app_context->data);

	app_control_destroy(app_control);

	return APP_ERROR_NONE;
}

int ui_app_main(int argc, char **argv, ui_app_lifecycle_callback_s *callback, void *user_data)
{
	struct ui_app_context app_context = {
		.package = NULL,
		.app_name = NULL,
		.state = APP_STATE_NOT_RUNNING,
		.callback = callback,
		.data = user_data
	};

	struct appcore_ops appcore_context = {
		.data = &app_context,
		.create = _ui_app_appcore_create,
		.terminate = _ui_app_appcore_terminate,
		.pause = _ui_app_appcore_pause,
		.resume = _ui_app_appcore_resume,
		.reset = _ui_app_appcore_reset,
	};

	if (argc < 1 || argv == NULL || callback == NULL)
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (callback->create == NULL)
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "app_create_cb() callback must be registered");

	if (app_context.state != APP_STATE_NOT_RUNNING)
		return app_error(APP_ERROR_ALREADY_RUNNING, __FUNCTION__, NULL);

	if (app_get_id(&(app_context.package)) != APP_ERROR_NONE)
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "failed to get the package");

	if (app_get_package_app_name(app_context.package, &(app_context.app_name)) != APP_ERROR_NONE) {
		free(app_context.package);
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "failed to get the package's app name");
	}

	app_context.state = APP_STATE_CREATING;

	LOGI("app_efl_main");
	appcore_efl_main(app_context.app_name, &argc, &argv, &appcore_context);

	free(app_context.package);
	free(app_context.app_name);

	return APP_ERROR_NONE;
}

void ui_app_exit(void)
{
	app_efl_exit();
}

int ui_app_add_event_handler(app_event_handler_h *event_handler, app_event_type_e event_type, app_event_cb callback, void *user_data)
{
	app_event_handler_h handler;
	Eina_List *l_itr;

	if (!handler_initialized) {
		eina_init();
		handler_initialized = 1;
	}

	if (event_handler == NULL || callback == NULL)
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "null parameter");

	if (event_type < APP_EVENT_LOW_MEMORY || event_type > APP_EVENT_SUSPENDED_STATE_CHANGED)
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid event type");

	EINA_LIST_FOREACH(handler_list[event_type], l_itr, handler) {
		if (handler->cb == callback)
			return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "already registered");
	}

	handler = calloc(1, sizeof(struct app_event_handler));
	if (!handler)
		return app_error(APP_ERROR_OUT_OF_MEMORY, __FUNCTION__, "failed to create handler");

	handler->type = event_type;
	handler->cb = callback;
	handler->data = user_data;

	if (appcore_initialized && eina_list_count(handler_list[event_type]) == 0)
		_ui_app_appcore_set_event_cb(event_type);

	handler_list[event_type] = eina_list_append(handler_list[event_type], handler);

	*event_handler = handler;

	return APP_ERROR_NONE;
}

int ui_app_remove_event_handler(app_event_handler_h event_handler)
{
	app_event_handler_h handler;
	app_event_type_e type;
	Eina_List *l_itr;
	Eina_List *l_next;

	if (event_handler == NULL)
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "handler is null");

	if (!handler_initialized) {
		LOGI("handler list is not initialized");
		return APP_ERROR_NONE;
	}

	type = event_handler->type;
	if (type < APP_EVENT_LOW_MEMORY || type > APP_EVENT_SUSPENDED_STATE_CHANGED)
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid handler");

	EINA_LIST_FOREACH_SAFE(handler_list[type], l_itr, l_next, handler) {
		if (handler == event_handler) {
			free(handler);
			handler_list[type] = eina_list_remove_list(handler_list[type], l_itr);

			if (appcore_initialized && eina_list_count(handler_list[type]) == 0)
				_ui_app_appcore_unset_event_cb(type);

			return APP_ERROR_NONE;
		}
	}

	return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "cannot find such handler");
}

