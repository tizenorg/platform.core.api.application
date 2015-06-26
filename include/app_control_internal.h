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

int app_control_to_bundle(app_control_h app_control, bundle **data);

/**
 * @brief Sets the window ID of the application.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] app_control The app_control handle
 * @param[in] id The window ID of the caller application (if the @a id is not positive, it clears the previous value)
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 * @see app_control_get_window()
 */
int app_control_set_window(app_control_h app_control, unsigned int id);

/**
 * @brief Gets the window ID of the application.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] app_control The app_control handle
 * @param[out] id The window ID of the caller application
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 * @see app_control_set_app_id()
 */
int app_control_get_window(app_control_h app_control, unsigned int *id);

/**
 * @brief Requests the specified callee window to be transient for the caller window.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks The @a callee_id window is transient for the top-level caller window and should be handled accordingly.
 * @param[in] app_control The app_control handle
 * @param[in] callee_id The callee window ID
 * @param[in] cbfunc The callback function to be called when the transient is requested
 * @param[in] data A data pointer to pass to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value.
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 */
int app_control_request_transient_app(app_control_h app_control, unsigned int callee_id, app_control_host_res_fn cbfunc, void *data);

/**
 * @brief Sends the launch request with uid.
 *
 * @details The operation is mandatory information for the launch request. \n
 *          If the operation is not specified, #APP_CONTROL_OPERATION_DEFAULT is used by default.
 *          If the operation is #APP_CONTROL_OPERATION_DEFAULT, the application ID is mandatory to explicitly launch the application.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel	public
 * @privilege	%http://tizen.org/privilege/appmanager.launch
 * @remarks The function returns #APP_CONTROL_ERROR_LAUNCH_REJECTED if the operation value is #APP_CONTROL_OPERATION_LAUNCH_ON_EVENT which is only for handling the event from the platform or other application, refer to @a Event module.
 * @param[in] app_control The app_control handle
 * @param[in] callback The callback function to be called when the reply is delivered
 * @param[in] user_data The user data to be passed to the callback function
 * @param[in] uid The user id to be launched
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #APP_CONTROL_ERROR_APP_NOT_FOUND The application to run the given launch request is not found
 * @retval #APP_CONTROL_ERROR_LAUNCH_REJECTED The application cannot be launched in current context
 * @retval #APP_CONTROL_ERROR_LAUNCH_FAILED Failed to launch the application
 * @retval #APP_CONTROL_ERROR_TIMED_OUT Failed due to timeout. The application that handles @a app_control may be busy
 * @retval #APP_CONTROL_ERROR_PERMISSION_DENIED Permission denied
 * @post If the launch request is sent for the result, the result will come back through app_control_reply_cb() from the callee application.
 * @see app_control_reply_to_launch_request()
 * @see app_control_reply_cb()
 */
int app_control_usr_send_launch_request(app_control_h app_control, app_control_reply_cb callback, void *user_data, uid_t uid);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_APPFW_APP_CONTROL_INTERNAL_H__ */
