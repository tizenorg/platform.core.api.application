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


#ifndef __TIZEN_APPFW_APP_PRIVATE_H__
#define __TIZEN_APPFW_APP_PRIVATE_H__

#include <appcore-common.h>

// GNU gettext macro is already defined at appcore-common.h
#ifdef _ 
#undef _
#endif

#include <app.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TIZEN_PATH_MAX 1024

#define PATH_FMT_APP_ROOT "/opt/apps"
#define PATH_FMT_RES_DIR PATH_FMT_APP_ROOT "/%s/res"
#define PATH_FMT_LOCALE_DIR PATH_FMT_RES_DIR "/locale"
#define PATH_FMT_DATA_DIR PATH_FMT_APP_ROOT "/%s/data"

typedef void (*app_terminate_task_cb) (void *data);

typedef struct app_terminate_task_s
{
	app_terminate_task_cb cb;
	void *data;
} app_terminate_task_t;

int app_add_terminate_task(app_terminate_task_cb callback, void *data);

app_device_orientation_e app_convert_appcore_rm(enum appcore_rm rm);

int app_get_project_name(const char *package, char **name);

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_APPFW_APP_PRIVATE_H__ */
