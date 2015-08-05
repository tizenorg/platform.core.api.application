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


#ifndef __TIZEN_APPFW_APP_H__
#define __TIZEN_APPFW_APP_H__

#include <tizen.h>
#include <app_service.h>
#include <app_control.h>
#include <app_alarm.h>
#include <app_common.h>
#include <app_preference.h>
#include <app_i18n.h>
#include <app_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CAPI_APPLICATION_MODULE
 * @{
 */


/**
 * @brief Called when the application starts.
 *
 * @details The callback function is called before the main loop of the application starts.
 *          In this callback, you can initialize application resources like window creation, data structure, and so on.
 *          After this callback function returns @c true, the main loop starts up and app_control_cb() is subsequently called.
 *          If this callback function returns @c false, the main loop doesn't start and app_terminate_cb() is subsequently called.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] user_data	The user data passed from the callback registration function
 * @return @c true on success,
 *         otherwise @c false
 * @pre	ui_app_main() will invoke this callback function.
 * @see ui_app_main()
 * @see #ui_app_lifecycle_callback_s
 */
typedef bool (*app_create_cb) (void *user_data);


/**
 * @brief Called when the application is completely obscured by another application and becomes invisible.
 *
 * @details The application is not terminated and still running in the paused state.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] user_data	The user data passed from the callback registration function
 * @see	ui_app_main()
 * @see	#ui_app_lifecycle_callback_s
 */
typedef void (*app_pause_cb) (void *user_data);


/**
 * @brief Called when the application becomes visible.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] user_data	The user data passed from the callback registration function
 * @see	ui_app_main()
 * @see #ui_app_lifecycle_callback_s
 */
typedef void (*app_resume_cb) (void *user_data);


/**
 * @brief Called when the application's main loop exits.
 * @details You should release the application's resources in this function.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] user_data	The user data passed from the callback registration function
 * @see	ui_app_main()
 * @see #ui_app_lifecycle_callback_s
 */
typedef void (*app_terminate_cb) (void *user_data);


/**
 * @brief Called when another application sends a launch request to the application.
 *
 * @details When the application is launched, this callback function is called after the main loop of the application starts up.
 *          The passed app_control handle describes the launch request and contains the information about why the application is launched.
 *          If the launch request is sent to the application in the running or pause state,
 *          this callback function can be called again to notify that the application has been asked to launch.
 *
 *          The application could be explicitly launched by the user from the application launcher or be launched to perform the specific operation by another application.
 *          The application is responsible for handling each launch request and responding appropriately.
 *          Using the App Control API, the application can get information about what is to be performed.
 *          If the application is launched from the application launcher or explicitly launched by another application,
 *          the passed app_control handle may include only the default operation (#APP_CONTROL_OPERATION_DEFAULT) without any data.
 *          For more information, see The @ref CAPI_APP_CONTROL_MODULE API description.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] app_control The handle to the app_control
 * @param[in] user_data	The user data passed from the callback registration function
 * @see ui_app_main()
 * @see #ui_app_lifecycle_callback_s
 * @see @ref CAPI_APP_CONTROL_MODULE API
 */
typedef void (*app_control_cb) (app_control_h app_control, void *user_data);


/**
 * @brief Called when other application send the launch request to the application.
 *
 * @details When the application is launched, this callback function is called after the main loop of application starts up.
 * The passed service handle describes the launch request and contains the information about why the application is launched.
 * If the launch request is sent to the application on running or pause state,
 * this callback function can be called again to notify that the application is asked to be launched.
 * 
 * The application could be explicitly launched by the user from the application launcher or be launched to perform the specific operation by other application.
 * The application is responsible for handling the each launch request and responding appropriately.
 * Using the Service API, the application can get the information what has to perform.
 * If the application is launched from the application launcher or explicitly launched by other application,
 * the passed service handle may include only the default operation (#SERVICE_OPERATION_DEFAULT) without any data
 * For more information, see The @ref CAPI_SERVICE_MODULE API description.
 *
 * @param[in]	service	The handle to the service
 * @param[in]	user_data	The user data passed from the callback registration function
 * @see app_efl_main()
 * @see #app_event_callback_s
 * @see @ref CAPI_SERVICE_MODULE API
 */
typedef void (*app_service_cb) (service_h service, void *user_data);


/**
 * @brief   Called when the system memory is running low.
 *
 * @details 
 * When low memory event is dispatched, the application should immediately save state and release resources to save as much memory as possible. \n
 * If enough memory is not reclaimed during low memory conditions, the system will terminate some of the applications to reclaim the memory.
 *
 * @param[in]	user_data	The user data passed from the callback registration function
 * @see	app_efl_main()
 * @see #app_event_callback_s
 */
typedef void (*app_low_memory_cb) (void *user_data);


/**
 * @brief   Called when the battery power is running low.
 * @details When the battery level falls below 5%, it is called.
 *
 * @param[in]	user_data	The user data passed from the callback registration function
 * @see	app_efl_main()
 * @see #app_event_callback_s
 */
typedef void (*app_low_battery_cb) (void *user_data);


/**
 * @brief   Called when the orientation of device changes.
 *
 * @param[in]	orientation	The orientation of device
 * @param[in]	user_data	The user data passed from the callback registration function
 * @see	app_efl_main()
 * @see #app_event_callback_s
 */
typedef void (*app_device_orientation_cb) (app_device_orientation_e orientation, void *user_data);


/**
 * @brief   Called when language setting changes.
 *
 * @param   [in] user_data The user data passed from the callback registration function
 * @see	app_efl_main()
 * @see #app_event_callback_s
 */
typedef void (*app_language_changed_cb) (void *user_data);


/**
 * @brief   Called when region format setting changes.
 *
 * @param   [in] user_data The user data passed from the callback registration function
 * @see	app_efl_main()
 * @see #app_event_callback_s
 */
typedef void (*app_region_format_changed_cb) (void *user_data);


/**
 * @brief The structure type to contain the set of callback functions for handling application events. 
 * @details It is one of the input parameters of the app_efl_main() function.
 *
 * @see app_efl_main()
 * @see app_create_cb()
 * @see app_pause_cb()
 * @see app_resume_cb()
 * @see app_terminate_cb()
 * @see app_service_cb()
 * @see app_low_memory_cb()
 * @see app_low_battery_cb()
 * @see app_device_orientation_cb()
 * @see app_language_changed_cb()
 * @see app_region_format_changed_cb()
 */
typedef struct
{
	app_create_cb create; /**< This callback function is called at the start of the application. */
	app_terminate_cb terminate; /**< This callback function is called once after the main loop of application exits. */
	app_pause_cb pause; /**< This callback function is called each time the application is completely obscured by another application and becomes invisible to the user. */
	app_resume_cb resume; /**< This callback function is called each time the application becomes visible to the user. */
	app_service_cb service; /**< This callback function is called when other application send the launch request to the application. */
	app_low_memory_cb low_memory; /**< The registered callback function is called when the system runs low on memory. */
	app_low_battery_cb low_battery; /**< The registered callback function is called when battery is low. */
	app_device_orientation_cb device_orientation; /**< The registered callback function is called when the orientation of device changes */
	app_language_changed_cb language_changed; /**< The registered callback function is called when language setting changes. */
	app_region_format_changed_cb region_format_changed; /**< The registered callback function is called when region format setting is changes. */
} app_event_callback_s;

/**
 * @brief The structure type containing the set of callback functions for handling application lifecycle events.
 * @details It is one of the input parameters of the ui_app_main() function.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @see ui_app_main()
 * @see app_create_cb()
 * @see app_pause_cb()
 * @see app_resume_cb()
 * @see app_terminate_cb()
 * @see app_control_cb()
 */
typedef struct
{
	app_create_cb create; /**< This callback function is called at the start of the application. */
	app_terminate_cb terminate; /**< This callback function is called once after the main loop of the application exits. */
	app_pause_cb pause; /**< This callback function is called each time the application is completely obscured by another application and becomes invisible to the user. */
	app_resume_cb resume; /**< This callback function is called each time the application becomes visible to the user. */
	app_control_cb app_control; /**< This callback function is called when another application sends the launch request to the application. */
} ui_app_lifecycle_callback_s;


/**
 * @brief Runs the main loop of application until app_efl_exit() is called
 *
 * @details This function is the main entry point of the Tizen application.
 * The app_create_cb() callback function is called to initialize the application before the main loop of application starts up.
 * After the app_create_cb() callback function returns true, the main loop starts up and the app_service_cb() callback function is subsequently called.
 * If the app_create_cb() callback function returns false, the main loop doesn't start up and app_terminate_cb() callback function is called
 *
 * @param [in] argc The argument count
 * @param [in] argv The argument vector
 * @param [in] callback The set of callback functions to handle application events
 * @param [in] user_data The user data to be passed to the callback functions
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #APP_ERROR_NONE Successful
 * @retval #APP_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_ERROR_INVALID_CONTEXT The application is illegally launched, not launched by the launch system.
 * @retval #APP_ERROR_ALREADY_RUNNING The main loop already starts
 *
 * @see app_create_cb()
 * @see app_terminate_cb()
 * @see app_pause_cb()
 * @see app_resume_cb()
 * @see app_service_cb()
 * @see app_low_memory_cb()
 * @see app_low_battery_cb()
 * @see app_device_orientation_cb()
 * @see app_language_changed_cb()
 * @see app_region_format_changed_cb()
 * @see app_efl_exit()
 * @see #app_event_callback_s
 */
int app_efl_main(int *argc, char ***argv, app_event_callback_s *callback, void *user_data);


/**
 * @brief Exits the main loop of application.
 *
 * @details The main loop of application stops and app_terminate_cb() is invoked
 * @see app_efl_main()
 * @see app_terminate_cb() 
 */
void app_efl_exit(void);


/**
 * @brief Gets the current device orientation.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @return The current device orientation
 */
app_device_orientation_e app_get_device_orientation(void);


/**
 * @brief Sets whether reclaiming system cache is enabled in the pause state.
 *
 * @details If the reclaiming system cache is enabled, the system caches are released as possible when the application's state changes to the pause state.
 *
 * @remarks The reclaiming system cache is enabled by default
 *
 * @param [in] enable whether reclaiming system cache is enabled
 */
void app_set_reclaiming_system_cache_on_pause(bool enable);

/**
 * @brief Runs the application's main loop until ui_app_exit() is called.
 *
 * @details This function is the main entry point of the Tizen application.
 *          The app_create_cb() callback function is called to initialize the application before the main loop of application starts up.
 *          After the app_create_cb() callback function returns true, the main loop starts up and the app_control_cb() callback function is subsequently called.
 *          If the app_create_cb() callback function returns false, the main loop doesn't start up and app_terminate_cb() callback function is called.
 *          This main loop supports event handling for the Ecore Main Loop.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] argc The argument count
 * @param[in] argv The argument vector
 * @param[in] callback The set of callback functions to handle application lifecycle events
 * @param[in] user_data The user data to be passed to the callback functions
 *
 * @return 0 on success, otherwise a negative error value
 * @retval #APP_ERROR_NONE Successful
 * @retval #APP_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_ERROR_INVALID_CONTEXT The application is illegally launched, not launched by the launch system
 * @retval #APP_ERROR_ALREADY_RUNNING The main loop already starts
 *
 * @see app_create_cb()
 * @see app_terminate_cb()
 * @see app_pause_cb()
 * @see app_resume_cb()
 * @see app_control_cb()
 * @see ui_app_exit()
 * @see #ui_app_lifecycle_callback_s
 */
int ui_app_main(int argc, char **argv, ui_app_lifecycle_callback_s *callback, void *user_data);


/**
 * @brief Exits the main loop of application.
 *
 * @details The main loop of application stops and app_terminate_cb() is invoked.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @see ui_app_main()
 * @see app_terminate_cb()
 */
void ui_app_exit(void);


/**
 * @brief Adds the system event handler
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[out] event_handler The event handler
 * @param[in] event_type The system event type
 * @param[in] callback The callback function
 * @param[in] user_data The user data to be passed to the callback functions
 *
 * @return 0 on success, otherwise a negative error value
 * @retval #APP_ERROR_NONE Successful
 * @retval #APP_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_ERROR_OUT_OF_MEMORY Out of memory
 *
 * @see app_event_type_e
 * @see app_event_cb
 * @see ui_app_remove_event_handler
 */
int ui_app_add_event_handler(app_event_handler_h *event_handler, app_event_type_e event_type, app_event_cb callback, void *user_data);


/**
 * @brief Removes registered event handler
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] event_handler The event handler
 *
 * @return 0 on success, otherwise a negative error value
 * @retval #APP_ERROR_NONE Successful
 * @retval #APP_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @see ui_app_add_event_handler
 */
int ui_app_remove_event_handler(app_event_handler_h event_handler);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* __TIZEN_APPFW_APP_H__ */
