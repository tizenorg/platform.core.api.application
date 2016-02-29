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


#include <appcore-common.h>
#include <appcore-efl.h>
#include <aul.h>

#include "app_extension.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "CAPI_APPFW_APPLICATION"

void *app_get_preinitialized_window(const char *win_name)
{
	if (elm_win_precreated_object_get() == NULL)
		return NULL;

	appcore_set_preinit_window_name(win_name);
	return elm_win_precreated_object_get();
}

void *app_get_preinitialized_background(void)
{
	return elm_bg_precreated_object_get();
}

void *app_get_preinitialized_conformant(void)
{
	return elm_conformant_precreated_object_get();
}
