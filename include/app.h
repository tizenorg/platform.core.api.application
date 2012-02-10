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
#include <app_alarm.h>
#include <app_preference.h>
#include <app_storage.h>
#include <app_i18n.h>
#include <app_ui_notification.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CAPI_APPLICATION_MODULE
 * @{
 */


/**
 * @brief Enumerations of error code for Application.
 */
typedef enum
{
	APP_ERROR_NONE = TIZEN_ERROR_NONE, /**< Successful */
	APP_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */
	APP_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY, /**< Out of memory */
	APP_ERROR_INVALID_CONTEXT = TIZEN_ERROR_NOT_PERMITTED, /**< Invalid application context */
	APP_ERROR_NO_SUCH_FILE = TIZEN_ERROR_NO_SUCH_FILE, /**< No such file or directory */
	APP_ERROR_ALREADY_RUNNING = TIZEN_ERROR_ALREADY_IN_PROGRESS, /**< Application is already running */
} app_error_e;


/**
 * @brief Enumerations of the device orientation.
 */
typedef enum
{
	APP_DEVICE_ORIENTATION_0 = 0, /**< The device is oriented in natural position */
	APP_DEVICE_ORIENTATION_90 = 90, /**< The device's left side is at the top */
	APP_DEVICE_ORIENTATION_180 = 180, /**< The device is upside down */
	APP_DEVICE_ORIENTATION_270 = 270, /**<The device's right side is to the top */
} app_device_orientation_e;


/**
 * @brief Called at the start of the application.
 *
 * @details The callback function is called before the main loop of application starts.
 * In this callback you can initialize application resources like window creation, data structure, etc.
 * After this callback function returns @c true, the main loop starts up and app_service_cb() is subsequently called.
 * If this callback function returns @c false, the main loop doesn't start and app_terminate_cb() is subsequently called.
 * 
 * @param[in]	user_data	The user data passed from the callback registration function
 * @return @c true on success, otherwise @c false
 * @pre	app_efl_main() will invoke this callback function.
 * @see app_efl_main()
 * @see #app_event_callback_s
 */
typedef bool (*app_create_cb) (void *user_data);


/**
 * @brief   Called when the application is completely obscured by another application and becomes invisible.
 *
 * @details The application is not terminated and still running in paused state.
 *
 * @param[in]	user_data	The user data passed from the callback registration function
 * @see	app_efl_main()
 * @see	#app_event_callback_s
 */
typedef void (*app_pause_cb) (void *user_data);


/**
 * @brief   Called when the application becomes visible.
 *
 * @remarks This callback function is not called when the application moved from created state to running state.
 *
 * @param[in]	user_data	The user data passed from the callback registration function
 * @see	app_efl_main()
 * @see #app_event_callback_s
 */
typedef void (*app_resume_cb) (void *user_data);


/**
 * @brief   Called once after the main loop of application exits.
 * @details You should release the application's resources in this function.
 *
 * @param[in]	user_data	The user data passed from the callback registration function
 * @see	app_efl_main()
 * @see #app_event_callback_s
 */
typedef void (*app_terminate_cb) (void *user_data);


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
 * @brief Gets the name of the application package.
 *
 * @remarks @a package must be released with free() by you.
 *
 * @param [out] package The name of the application package
 *
 * @return 0 on success, otherwise a negative error value.
 *
 * @retval #APP_ERROR_NONE Successful
 * @retval #APP_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_ERROR_INVALID_CONTEXT The application is illegally launched, not launched by the launch system.
 * @retval #APP_ERROR_OUT_OF_MEMORY Out of memory
 */
int app_get_package(char **package);


/**
 * @brief Gets the localized name of the application.
 *
 * @remarks @a name must be released with free() by you.
 *
 * @param [out] name The name of the application
 *
 * @return 0 on success, otherwise a negative error value.
 *
 * @retval #APP_ERROR_NONE Successful
 * @retval #APP_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_ERROR_INVALID_CONTEXT The application is illegally launched, not launched by the launch system.
 * @retval #APP_ERROR_OUT_OF_MEMORY Out of memory
 */
int app_get_name(char **name);


/**
 * @brief Gets the version of the application package.
 *
 * @remarks @a version must be released with free() by you.
 *
 * @param [out] version The version of the application
 *
 * @return 0 on success, otherwise a negative error value.
 *
 * @retval #APP_ERROR_NONE Successful
 * @retval #APP_ERROR_INVALID_PARAMETER Invalid parameter 
 * @retval #APP_ERROR_INVALID_CONTEXT The application is illegally launched, not launched by the launch system.
 * @retval #APP_ERROR_OUT_OF_MEMORY Out of memory
 */
int app_get_version(char **version);


/**
 * @brief Gets the absolute path to the resource included in application package
 *
 * @details The application cannot write and modify any resource files.
 *
 * @remarks This function stores the absolute path into the @a buffer at most one less than @a size bytes 
 * and a null character is appended  in @a buffer after the path stored.
 *
 * @param [in] resource The resource's path relative to the resource directory of the application package (e.g. edje/app.edj or images/background.png)
 * @param [in] buffer The pre-allocated buffer where the absolute path to the resource is stored. 
 * @param [in] size The size of @a buffer in bytes
 * @return  @a buffer on success, otherwise NULL.
 */
char* app_get_resource(const char *resource, char *buffer, int size);


/**
 * @brief Gets the absolute path to the application's data directory.
 *
 * @details An application can read and write its own data files in the application's data directory.
 *
 * @remarks This function stores the absolute path into the @a buffer at most one less than @a size bytes 
 * and a null character is appended  in @a buffer after the path stored.
 *
 * @param [in] buffer The pre-allocated buffer where the absolute path to the application data directory
 * @param [in] size The size of @a buffer in bytes
 * @return  @a buffer on success, otherwise NULL.
 */
char* app_get_data_directory(char *buffer, int size);


/**
 * @brief Gets the current device orientation.
 *
 * @return The current device orientation
 */
app_device_orientation_e app_get_device_orientation(void);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_APPFW_APP_H__ */
