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

#define LOG_TAG "TIZEN_N_APPLICATION"

#define TERMINATE_TASK_MAX 8

typedef enum {
	APP_STATE_NOT_RUNNING, // The application has been launched or was running but was terminated
	APP_STATE_CREATING, // The application is initializing the resources on app_create_cb callback
	APP_STATE_RUNNING, // The application is running in the foreground and background
} app_state_e;

typedef struct app_context_s {
	struct appcore_ops appcore;
	char *package;
	char *project_name;
	app_event_callback_s *callbacks;
	app_state_e state;
	void *user_data;
	app_terminate_task_t terminate_task[TERMINATE_TASK_MAX];
} *app_context_h;

static int app_cb_broker_appcore_create(void *data);
static int app_cb_broker_appcore_pause(void *data);
static int app_cb_broker_appcore_resume(void *data);
static int app_cb_broker_appcore_terminate(void *data);
static int app_cb_broker_appcore_reset(bundle *appcore_bundle, void *data);

static int app_cb_broker_appcore_low_memory(void *data);
static int app_cb_broker_appcore_low_battery(void *data);
static int app_cb_broker_appcore_rotation_event(enum appcore_rm rm, void *data);
static int app_cb_broker_appcore_lang_changed(void *data);
static int app_cb_broker_appcore_region_changed(void *data);

static void app_set_appcore_event_cb(void);
static void app_unset_appcore_event_cb(void);

static struct app_context_s app_context = {
	.appcore.create = app_cb_broker_appcore_create,
	.appcore.terminate = app_cb_broker_appcore_terminate,
	.appcore.pause = app_cb_broker_appcore_pause,
	.appcore.resume = app_cb_broker_appcore_resume,
	.appcore.reset = app_cb_broker_appcore_reset,
	.package = NULL,
	.callbacks = NULL,
	.state = APP_STATE_NOT_RUNNING,
	.user_data = NULL
};


static void app_reset_app_context()
{
	int i;

	if (app_context.package != NULL)
	{
		free(app_context.package);
	}

	if (app_context.project_name != NULL)
	{
		free(app_context.project_name);
	}

	app_context.callbacks = NULL;
	app_context.user_data = NULL;
	app_context.state = APP_STATE_NOT_RUNNING;

	for (i=0; i<TERMINATE_TASK_MAX; i++)
	{
		app_context.terminate_task[i].cb = NULL;
		app_context.terminate_task[i].data = NULL;
	}
}


int app_efl_main(int *argc, char ***argv, app_event_callback_s *callback, void *user_data)
{
	char *package = NULL;;
	char *project_name = NULL;

	if (argc == NULL || argv == NULL || callback == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, APP_ERROR_INVALID_PARAMETER);
		return APP_ERROR_INVALID_PARAMETER;
	}

	if (callback->create == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, APP_ERROR_INVALID_PARAMETER);
		return APP_ERROR_INVALID_PARAMETER;
	}

	if (app_context.state != APP_STATE_NOT_RUNNING)
	{
		LOGE("[%s] ALREADY_RUNNING(0x%08x)", __FUNCTION__, APP_ERROR_ALREADY_RUNNING);
		return APP_ERROR_ALREADY_RUNNING;
	}

	if (app_get_package(&package) != 0)
	{
		LOGE("[%s] INVALID_CONTEXT(0x%08x)", __FUNCTION__, APP_ERROR_INVALID_CONTEXT);
		return APP_ERROR_INVALID_CONTEXT;
	}

	app_context.package = package;
	
	if (app_get_project_name(package, &project_name) != 0)
	{
		LOGE("[%s] INVALID_CONTEXT(0x%08x)", __FUNCTION__, APP_ERROR_INVALID_CONTEXT);
		return APP_ERROR_INVALID_CONTEXT;
	}

	app_context.project_name = project_name;

	app_context.state = APP_STATE_CREATING;
	app_context.callbacks = callback;
	app_context.user_data = user_data;

	appcore_efl_main(app_context.project_name, argc, argv, &(app_context.appcore));

	app_reset_app_context();

	return APP_ERROR_NONE;
}

void app_efl_exit(void)
{
	elm_exit();
}

static void app_set_appcore_event_cb()
{
	if (app_context.callbacks->low_memory != NULL)
	{
		appcore_set_event_callback(APPCORE_EVENT_LOW_MEMORY, app_cb_broker_appcore_low_memory, NULL);
	}

	if (app_context.callbacks->low_battery != NULL)
	{
		appcore_set_event_callback(APPCORE_EVENT_LOW_BATTERY, app_cb_broker_appcore_low_battery, NULL);
	}

	if (app_context.callbacks->device_orientation != NULL)
	{
		appcore_set_rotation_cb(app_cb_broker_appcore_rotation_event, NULL);
	}

	if (app_context.callbacks->language_changed != NULL)
	{
		appcore_set_event_callback(APPCORE_EVENT_LANG_CHANGE, app_cb_broker_appcore_lang_changed, NULL);
	}

	if (app_context.callbacks->region_format_changed != NULL)
	{
		appcore_set_event_callback(APPCORE_EVENT_REGION_CHANGE, app_cb_broker_appcore_region_changed, NULL);
	}
}

static void app_unset_appcore_event_cb(void)
{
	appcore_set_event_callback(APPCORE_EVENT_LOW_MEMORY, NULL, NULL);
	appcore_set_event_callback(APPCORE_EVENT_LOW_BATTERY, NULL, NULL);
	appcore_unset_rotation_cb();
	appcore_set_event_callback(APPCORE_EVENT_LANG_CHANGE, NULL, NULL);
	appcore_set_event_callback(APPCORE_EVENT_REGION_CHANGE, NULL, NULL);
}


int app_cb_broker_appcore_create(void *data)
{
	app_create_cb create_cb;
	char locale_dir[TIZEN_PATH_MAX] = {0, };

	app_set_appcore_event_cb();

	snprintf(locale_dir, TIZEN_PATH_MAX, PATH_FMT_LOCALE_DIR, app_context.package);
	appcore_set_i18n(app_context.project_name, locale_dir);

	create_cb = app_context.callbacks->create;

	if (create_cb != NULL && create_cb(app_context.user_data) == true)
	{
		app_context.state = APP_STATE_RUNNING;
		return 0;
	}

	return -1;
}

int app_cb_broker_appcore_terminate(void *data)
{
	app_terminate_cb terminate_cb;
	int i;

	terminate_cb = app_context.callbacks->terminate;

	if (terminate_cb != NULL)
	{
		terminate_cb(app_context.user_data);
	}

	app_unset_appcore_event_cb();	

	for (i=0; i<TERMINATE_TASK_MAX; i++)
	{
		if (app_context.terminate_task[i].cb != NULL)
		{	
			app_terminate_task_cb task_cb;

			task_cb = app_context.terminate_task[i].cb;
			
			if (task_cb != NULL)
			{
				task_cb(app_context.terminate_task[i].data);
			}
		}
	}

	return 0;
}

int app_cb_broker_appcore_pause(void *data)
{
	app_pause_cb pause_cb;

	pause_cb = app_context.callbacks->pause;

	if (pause_cb != NULL)
	{
		pause_cb(app_context.user_data);
	}

	return 0;
}

int app_cb_broker_appcore_resume(void *data)
{
	app_resume_cb resume_cb;

	resume_cb = app_context.callbacks->resume;

	if (resume_cb != NULL)
	{
		resume_cb(app_context.user_data);
	}

	return 0;
}


int app_cb_broker_appcore_reset(bundle *appcore_bundle, void *data)
{
	app_service_cb service_cb;
	service_h service;

	if (service_create_event(appcore_bundle, &service) != 0)
	{
		LOGE("[%s] fail to create service handle from bundle", __FUNCTION__);
		return -1;
	}

	service_cb = app_context.callbacks->service;

	if (service_cb != NULL)
	{
		service_cb(service, app_context.user_data);
	}

	service_destroy(service);

	return 0;
}


int app_cb_broker_appcore_low_memory(void *data)
{
	app_low_memory_cb low_memory_cb;

	low_memory_cb = app_context.callbacks->low_memory;

	if (low_memory_cb != NULL)
	{
		low_memory_cb(app_context.user_data);
	}

	return 0;
}

int app_cb_broker_appcore_low_battery(void *data)
{
	app_low_battery_cb low_battery_cb;

	low_battery_cb = app_context.callbacks->low_battery;

	if (low_battery_cb != NULL)
	{
		low_battery_cb(app_context.user_data);
	}

	return 0;
}

int app_cb_broker_appcore_rotation_event(enum appcore_rm rm, void *data)
{
	app_device_orientation_cb device_orientation_cb;

	device_orientation_cb = app_context.callbacks->device_orientation;

	if (device_orientation_cb != NULL)
	{
		app_device_orientation_e dev_orientation;

		dev_orientation = app_convert_appcore_rm(rm);

		device_orientation_cb(dev_orientation, app_context.user_data);
	}

	return 0;
}

int app_cb_broker_appcore_lang_changed(void *data)
{
	app_language_changed_cb lang_changed_cb;

	lang_changed_cb = app_context.callbacks->language_changed;

	if (lang_changed_cb != NULL)
	{
		lang_changed_cb(app_context.user_data);
	}

	return 0;
}

int app_cb_broker_appcore_region_changed(void *data)
{
	app_region_format_changed_cb region_changed_cb;

	region_changed_cb = app_context.callbacks->region_format_changed;

	if (region_changed_cb != NULL)
	{
		region_changed_cb(app_context.user_data);
	}

	return 0;
}


int app_add_terminate_task(app_terminate_task_cb callback, void *data)
{
	int i;

	for (i=0; i<TERMINATE_TASK_MAX; i++)
	{
		if (app_context.terminate_task[i].cb == NULL)
		{
			app_context.terminate_task[i].cb = callback;
			app_context.terminate_task[i].data = data;
			return 0;
		}
	}

	LOGE("[%s] fail to add terminate-task", __FUNCTION__);		

	return -1;
}
