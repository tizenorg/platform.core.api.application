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

#include <app_private.h>
#include <app_service_private.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "CAPI_APPFW_APPLICATION"

typedef enum {
	APP_STATE_NOT_RUNNING, // The application has been launched or was running but was terminated
	APP_STATE_CREATING, // The application is initializing the resources on app_create_cb callback
	APP_STATE_RUNNING, // The application is running in the foreground and background
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

static int app_appcore_low_memory(void *data);
static int app_appcore_low_battery(void *data);
static int app_appcore_rotation_event(enum appcore_rm rm, void *data);
static int app_appcore_lang_changed(void *data);
static int app_appcore_region_changed(void *data);

static void app_set_appcore_event_cb(app_context_h app_context);
static void app_unset_appcore_event_cb(void);


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
	{
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (callback->create == NULL)
	{
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "app_create_cb() callback must be registered");
	}

	if (app_context.state != APP_STATE_NOT_RUNNING)
	{
		return app_error(APP_ERROR_ALREADY_RUNNING, __FUNCTION__, NULL);
	}

	if (app_get_id(&(app_context.package)) != APP_ERROR_NONE)
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "failed to get the package");
	}
	
	if (app_get_package_app_name(app_context.package, &(app_context.app_name)) != APP_ERROR_NONE)
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "failed to get the package's app name");
	}

	app_context.state = APP_STATE_CREATING;

	appcore_efl_main(app_context.app_name, argc, argv, &appcore_context);

	free(app_context.package);
	free(app_context.app_name);

	return APP_ERROR_NONE;
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
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);
	}

	app_set_appcore_event_cb(app_context);

	snprintf(locale_dir, TIZEN_PATH_MAX, PATH_FMT_LOCALE_DIR, app_context->package);
	if (access(locale_dir, R_OK) != 0) {
		snprintf(locale_dir, TIZEN_PATH_MAX, PATH_FMT_RO_LOCALE_DIR, app_context->package);
	}
	appcore_set_i18n(app_context->app_name, locale_dir);

	create_cb = app_context->callback->create;

	if (create_cb == NULL || create_cb(app_context->data) == false)
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "app_create_cb() returns false");
	}

	app_context->state = APP_STATE_RUNNING;

	return APP_ERROR_NONE;
}

int app_appcore_terminate(void *data)
{
	app_context_h app_context = data;
	app_terminate_cb terminate_cb;

	if (app_context == NULL)
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);
	}

	terminate_cb = app_context->callback->terminate;

	if (terminate_cb != NULL)
	{
		terminate_cb(app_context->data);
	}

	app_unset_appcore_event_cb();	

	app_finalizer_execute();

	return APP_ERROR_NONE;
}

int app_appcore_pause(void *data)
{
	app_context_h app_context = data;
	app_pause_cb pause_cb;

	if (app_context == NULL)
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);
	}

	pause_cb = app_context->callback->pause;

	if (pause_cb != NULL)
	{
		pause_cb(app_context->data);
	}

	return APP_ERROR_NONE;
}

int app_appcore_resume(void *data)
{
	app_context_h app_context = data;
	app_resume_cb resume_cb;

	if (app_context == NULL)
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);
	}

	resume_cb = app_context->callback->resume;

	if (resume_cb != NULL)
	{
		resume_cb(app_context->data);
	}

	return APP_ERROR_NONE;
}


int app_appcore_reset(bundle *appcore_bundle, void *data)
{
	app_context_h app_context = data;
	app_service_cb service_cb;
	service_h service;

	if (app_context == NULL)
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);
	}

	if (service_create_event(appcore_bundle, &service) != APP_ERROR_NONE)
	{
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "failed to create a service handle from the bundle");
	}

	service_cb = app_context->callback->service;

	if (service_cb != NULL)
	{
		service_cb(service, app_context->data);
	}

	service_destroy(service);

	return APP_ERROR_NONE;
}


int app_appcore_low_memory(void *data)
{
	app_context_h app_context = data;
	app_low_memory_cb low_memory_cb;

	if (app_context == NULL)
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);
	}

	low_memory_cb = app_context->callback->low_memory;

	if (low_memory_cb != NULL)
	{
		low_memory_cb(app_context->data);
	}

	return APP_ERROR_NONE;
}

int app_appcore_low_battery(void *data)
{
	app_context_h app_context = data;
	app_low_battery_cb low_battery_cb;

	if (app_context == NULL)
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);
	}

	low_battery_cb = app_context->callback->low_battery;

	if (low_battery_cb != NULL)
	{
		low_battery_cb(app_context->data);
	}

	return APP_ERROR_NONE;
}

int app_appcore_rotation_event(enum appcore_rm rm, void *data)
{
	app_context_h app_context = data;
	app_device_orientation_cb device_orientation_cb;

	if (app_context == NULL)
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);
	}

	device_orientation_cb = app_context->callback->device_orientation;

	if (device_orientation_cb != NULL)
	{
		app_device_orientation_e dev_orientation;

		dev_orientation = app_convert_appcore_rm(rm);

		device_orientation_cb(dev_orientation, app_context->data);
	}

	return APP_ERROR_NONE;
}

int app_appcore_lang_changed(void *data)
{
	app_context_h app_context = data;
	app_language_changed_cb lang_changed_cb;

	if (app_context == NULL)
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);
	}

	lang_changed_cb = app_context->callback->language_changed;

	if (lang_changed_cb != NULL)
	{
		lang_changed_cb(app_context->data);
	}

	return APP_ERROR_NONE;
}

int app_appcore_region_changed(void *data)
{
	app_context_h app_context = data;
	app_region_format_changed_cb region_changed_cb;

	if (app_context == NULL)
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);
	}

	region_changed_cb = app_context->callback->region_format_changed;

	if (region_changed_cb != NULL)
	{
		region_changed_cb(app_context->data);
	}

	return APP_ERROR_NONE;
}


void app_set_appcore_event_cb(app_context_h app_context)
{
	if (app_context->callback->low_memory != NULL)
	{
		appcore_set_event_callback(APPCORE_EVENT_LOW_MEMORY, app_appcore_low_memory, app_context);
	}

	if (app_context->callback->low_battery != NULL)
	{
		appcore_set_event_callback(APPCORE_EVENT_LOW_BATTERY, app_appcore_low_battery, app_context);
	}

	if (app_context->callback->device_orientation != NULL)
	{
		appcore_set_rotation_cb(app_appcore_rotation_event, app_context);
	}

	if (app_context->callback->language_changed != NULL)
	{
		appcore_set_event_callback(APPCORE_EVENT_LANG_CHANGE, app_appcore_lang_changed, app_context);
	}

	if (app_context->callback->region_format_changed != NULL)
	{
		appcore_set_event_callback(APPCORE_EVENT_REGION_CHANGE, app_appcore_region_changed, app_context);
	}
}

void app_unset_appcore_event_cb(void)
{
	appcore_set_event_callback(APPCORE_EVENT_LOW_MEMORY, NULL, NULL);
	appcore_set_event_callback(APPCORE_EVENT_LOW_BATTERY, NULL, NULL);
	appcore_unset_rotation_cb();
	appcore_set_event_callback(APPCORE_EVENT_LANG_CHANGE, NULL, NULL);
	appcore_set_event_callback(APPCORE_EVENT_REGION_CHANGE, NULL, NULL);
}
