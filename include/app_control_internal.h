/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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

#ifndef __TIZEN_APPFW_APP_CONTROL_INTERNAL_H__
#define __TIZEN_APPFW_APP_CONTROL_INTERNAL_H__

#include <bundle.h>

#include <app_control.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file app_control_internal.h
 */

/**
 * @addtogroup CAPI_APP_CONTROL_MODULE
 * @{
 */

/**
 * @brief Replaces all data in the app_control with the bundle
 *
 * @remarks This function clears all data in the app_control and adds all key-value pairs in the bundle into the app_control
 * @param [in] app_control The app_control handle
 * @param [in] data The bundle handle
 * @return 0 on success, otherwise a negative error value.
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	app_control_export_as_bundle()
 * @code
 *
 * #include <bundle.h>
 * #include <app_control.h>
 *
 * app_control_h app_control = NULL;
 * app_control_create(&app_control);
 * app_control_import_from_bundle(app_control, b);
 *
 * @endcode
 *
 */
int app_control_import_from_bundle(app_control_h app_control, bundle *data);

/**
 * @brief Returns a new bundle containing all data contained int the app_control
 *
 * @remarks The @a data must be released with bundle_free() by you.
 * @param [in] app_control The app_control handle
 * @param [out] data The bundle handle
 * @return 0 on success, otherwise a negative error value.
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	app_control_import_from_bundle()
 * @code
 *
 * #include <bundle.h>
 * #include <app_control.h>
 *
 * bundle* b = NULL;
 * app_control_export_as_bundle(app_control, &b);
 *
 * @endcode
 */
int app_control_export_as_bundle(app_control_h app_control, bundle **data);

int app_control_create_request(bundle *data, app_control_h *app_control);

int app_control_create_event(bundle *data, app_control_h *app_control);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_APPFW_APP_CONTROL_INTERNAL_H__ */
