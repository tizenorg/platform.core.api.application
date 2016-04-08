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


#ifndef __TIZEN_APPFW_APP_EXTENSION_H__
#define __TIZEN_APPFW_APP_EXTENSION_H__

#include <app_control.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief	Gets the preinitialized window object.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks	This API only supports BASIC type window.
 *
 * @param[in] win_name The name to be set for the preinitialized window
 *
 * @return	A @a window object on success,
 *		otherwise @c NULL
 */
void *app_get_preinitialized_window(const char *win_name);

/**
 * @brief	Gets the preinitialized background object added to the preinitialized window.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks	This API should be called after calling app_get_preinitizlized_window().
 *
 * @return	A @a background object on success,
 *		otherwise @c NULL
 */
void *app_get_preinitialized_background(void);

/**
 * @brief	Gets the preinitialized conformant widget added to the preinitialized window.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks	This API should be called after calling app_get_preinitizlized_window().
 *
 * @return	A conformant object on success,
 *		otherwise @c NULL
 */
void *app_get_preinitialized_conformant(void);

/**
 * @platform
 * @brief Sets the ID of default application associated with operation, mime-type and uri.
 *
 * @since_tizen 3.0
 * @privlevel platform
 * @privilege %http://tizen.org/privilege/systemsettings.admin
 * @param[in] app_control The app_control handle
 * @param[in] app_id The ID of the application
 *
 * @return 0 on success, otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_PERMISSION_DENIED Permission denied
 * @retval #APP_CONTROL_ERROR_IO_ERROR IO error
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 *
 */
int app_control_set_defapp(app_control_h app_control, const char *app_id);

/**
 * @platform
 * @brief Unsets default application control setting of an application.
 *
 * @details When an user call this API, all the default application settings for the app_id are unset.
 *
 * @since_tizen 3.0
 * @privlevel platform
 * @privilege %http://tizen.org/privilege/systemsettings.admin
 * @param[in] app_id The ID of the application
 *
 * @return 0 on success, otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_PERMISSION_DENIED Permission denied
 * @retval #APP_CONTROL_ERROR_IO_ERROR IO error
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 *
 */
int app_control_unset_defapp(const char *app_id);
#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_APPFW_APP_EXTENSION_H__ */

