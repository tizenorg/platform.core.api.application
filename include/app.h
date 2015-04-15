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
#include <app_preference.h>
#include <app_i18n.h>

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
 * @brief Called when other application send the launch request to the application.
 *
 * @details When the application is launched, this callback function is called after the main loop of application starts up.
 * The passed app_control handle describes the launch request and contains the information about why the application is launched.
 * If the launch request is sent to the application on running or pause state,
 * this callback function can be called again to notify that the application is asked to be launched.
 *
 * The application could be explicitly launched by the user from the application launcher or be launched to perform the specific operation by the other application.
 * The application is responsible for handling each launch request and responding accordingly.
 * Using the app_control API, the application can get the information it needs to perform.
 * If the application is launched from the application launcher or explicitly launched by other application,
 * the passed app_control handle may include only the default operation (#APP_CONTROL_OPERATION_DEFAULT) without any data
 * For more information, see The @ref CAPI_APP_CONTROL_MODULE API description.
 *
 * @param[in]	app_control_h	The handle to the app_control
 * @param[in]	user_data	The user data passed from the callback registration function
 * @see app_efl_main()
 * @see #app_event_callback_s
 * @see @ref CAPI_APP_CONTROL_MODULE API
 */
typedef void (*app_control_cb) (app_control_h app_control, void *user_data);


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
 * @brief Enumeration for system events
 */
typedef enum
{
	APP_EVENT_LOW_MEMORY, /**< The low memory event */
	APP_EVENT_LOW_BATTERY, /**< The low battery event */
	APP_EVENT_LANGUAGE_CHANGED, /**< The system language changed event */
	APP_EVENT_DEVICE_ORIENTATION_CHANGED, /**< The device orientation changed event */
	APP_EVENT_REGION_FORMAT_CHANGED, /**< The region format changed event */
} app_event_type_e;


/**
 * @brief Enumeration for low memory status.
 */
typedef enum
{
	APP_EVENT_LOW_MEMORY_NORMAL = 0x01, /**< Normal status */
	APP_EVENT_LOW_MEMORY_SOFT_WARNING = 0x02, /**< Soft warning status */
	APP_EVENT_LOW_MEMORY_HARD_WARNING = 0x04, /**< Hard warning status */
} app_event_low_memory_status_e;


/**
 * @brief Enumeration for battery status.
 */
typedef enum
{
	APP_EVENT_LOW_BATTERY_POWER_OFF = 1, /**< The battery status is under 1% */
	APP_EVENT_LOW_BATTERY_CRITICAL_LOW, /**< The battery status is under 5% */
} app_event_low_battery_status_e;


/**
 * @brief The event handler that returned from add event handler function
 *
 * @see app_event_type_e
 * @see app_add_event_handler
 * @see app_remove_event_handler
 * @see app_event_info_h
 */
typedef struct app_event_handler* app_event_handler_h;


/**
 * @brief The system event information
 *
 * @see app_event_get_low_memory_status
 * @see app_event_get_low_battery_status
 * @see app_event_get_language
 * @see app_event_get_region_format
 * @see app_event_get_device_orientation
 */
typedef struct app_event_info* app_event_info_h;


/**
 * @brief The system event callback function
 *
 * @param[in] event_info The system event information
 * @param[in] user_data The user data passed from the add event handler function
 *
 * @see app_add_event_handler
 * @see app_event_info_h
 */
typedef void (*app_event_cb)(app_event_info_h event_info, void *user_data);


/**
 * @brief Gets the low memory status from given event info
 *
 * @param[in] event_info The system event info
 * @param[out] status The low memory status
 *
 * @return 0 on success, otherwise a negative error value
 * @retval #APP_ERROR_NONE Successful
 * @retval #APP_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_ERROR_INVALID_CONTEXT Invalid event context
 *
 * @see app_event_info_h
 * @see app_event_low_memory_status_e
 */
int app_event_get_low_memory_status(app_event_info_h event_info, app_event_low_memory_status_e *status);


/**
 * @brief Gets the low battery status from given event info
 *
 * @param[in] event_info The system event info
 * @param[out] status The low battery status
 *
 * @return 0 on success, otherwise a negative error value
 * @retval #APP_ERROR_NONE Successful
 * @retval #APP_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_ERROR_INVALID_CONTEXT Invalid event context
 *
 * @see app_event_info_h
 * @see app_event_low_battery_status_e
 */
int app_event_get_low_battery_status(app_event_info_h event_info, app_event_low_battery_status_e *status);


/**
 * @brief Gets the language from given event info
 *
 * @remarks @a lang must be released using free()
 * @param[in] event_info The system event info
 * @param[out] lang The language changed
 *
 * @return 0 on success, otherwise a negative error value
 * @retval #APP_ERROR_NONE Successful
 * @retval #APP_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_ERROR_INVALID_CONTEXT Invalid event context
 *
 * @see app_event_info_h
 */
int app_event_get_language(app_event_info_h event_info, char **lang);


/**
 * @brief Gets the region format from given event info
 *
 * @remarks @a region must be released using free()
 * @param[in] event_info The system event info
 * @param[out] region The region format changed
 *
 * @return 0 on success, otherwise a negative error value
 * @retval #APP_ERROR_NONE Successful
 * @retval #APP_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_ERROR_INVALID_CONTEXT Invalid event context
 *
 * @see app_event_info_h
 */
int app_event_get_region_format(app_event_info_h event_info, char **region);


/**
 * @brief Gets the device orientation from given event info
 *
 * @param[in] event_info The system event info
 * @param[out] orientation The device orientation changed
 *
 * @return 0 on success, otherwise a negative error value
 * @retval #APP_ERROR_NONE Successful
 * @retval #APP_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_ERROR_INVALID_CONTEXT Invalid event context
 *
 * @see app_event_info_h
 * @see app_device_orientation_e
 */
int app_event_get_device_orientation(app_event_info_h event_info, app_device_orientation_e *orientation);


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
 * @brief Gets the ID of the application.
 *
 * @remarks @a ID must be released with free() by you.
 *
 * @param [out] id The ID of the application
 *
 * @return 0 on success, otherwise a negative error value.
 *
 * @retval #APP_ERROR_NONE Successful
 * @retval #APP_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_ERROR_INVALID_CONTEXT The application is illegally launched, not launched by the launch system.
 * @retval #APP_ERROR_OUT_OF_MEMORY Out of memory
 */
int app_get_id(char **id);


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
 * @brief	Gets the absolute path to the application's data directory which is used to store private
 *			data of the application.
 * @details	An application can read and write its own files in the application's data directory.
 * @remarks	The returned path should be released.
 *
 * @return	The absolute path to the application's data directory, @n
 *			else @a null pointer if the memory is insufficient
 */
char *app_get_data_path(void);

/**
 * @brief	Gets the absolute path to the application's cache directory which is used to store
 *			temporary data of the application.
 * @details	An application can read and write its own files in the application's cache directory.
 * @remarks	The returned path should be released. @n
 *			The files stored in the application's cache directory can be removed by Setting
 *			application or platform while the application is running.
 *
 * @return	The absolute path to the application's cache directory, @n
 *			else @a null pointer if the memory is insufficient
 */
char *app_get_cache_path(void);

/**
 * @brief	Gets the absolute path to the application resource directory. The resource files
 *			are delivered with the application package.
 * @details	An application can only read its own files in the application's resource directory.
 * @remarks	The returned path should be released.
 *
 * @return	The absolute path to the application's resource directory, @n
 *			else @a null pointer if the memory is insufficient
 */
char *app_get_resource_path(void);

/**
 * @brief	Gets the absolute path to the application's shared data directory which is used to share
 *			data with other applications.
 * @details	An application can read and write its own files in the application's shared data
 *			directory and others can only read the files.
 * @remarks	The returned path should be released.
 *
 * @return	The absolute path to the application's shared data directory, @n
 *			else @a null pointer if the memory is insufficient
 */
char *app_get_shared_data_path(void);

/**
 * @brief	Gets the absolute path to the application's shared resource directory which is used to
 *			share resources with other applications.
 * @details	An application can read its own files in the application's shared resource directory
 *			and others can only read the files.
 * @remarks	The returned path should be released.
 *
 * @return	The absolute path to the application's shared resource directory, @n
 *			else @a null pointer if the memory is insufficient
 */
char *app_get_shared_resource_path(void);

/**
 * @brief	Gets the absolute path to the application's shared trusted directory which is used to share data
 *			with family of trusted applications
 * @details	An application can read and write its own files in the application's shared trusted directory
 *			and the family applications signed with the same certificate can read and write the files in the
 *			shared trusted directory.
 * @remarks	The returned path should be released.
 *
 * @return	The absolute path to the application's shared trusted directory, @n
 *			else @a null pointer if the memory is insufficient
 */
char *app_get_shared_trusted_path(void);

/**
 * @brief	Gets the absolute path to the application's external data directory which is used to
 *			store data of the application.
 * @details	An application can read and write its own files in the application's external data
 *			directory.
 * @remarks	The returned path should be released. @n
 *			The important files stored in the application's external data directory should be
 *			encrypted because they can be exported via the external sdcard.
 *
 * @return	The absolute path to the application's external data directory, @n
 *			else @a null pointer if the memory is insufficient
 */
char *app_get_external_data_path(void);

/**
 * @brief	Gets the absolute path to the application's external cache directory which is used to
 *			store temporary data of the application.
 * @details	An application can read and write its own files in the application's external cache
 *			directory.
 * @remarks The returned path should be released. @n
 *			The files stored in the application's external cache directory can be removed by
 *			Setting application while the application is running. @n
 *			The important files stored in the application's external cache directory should be
 *			encrypted because they can be exported via the external sdcard.
 *
 * @return	The absolute path to the application's external cache directory, @n
 *			else @a null pointer if the memory is insufficient
 */
char *app_get_external_cache_path(void);

/**
 * @brief	Gets the absolute path to the application's external shared data directory which is
 *			used to share data with other applications.
 * @details	An application can read and write its own files in the application's external shared
 *			data directory and others can only read the files.
 * @remarks	The specified @a path should be released.
 *
 * @return	The absolute path to the application's external shared data directory, @n
 *			else @a null pointer if the memory is insufficient
 */
char *app_get_external_shared_data_path(void);


/**
 * @brief Gets the current device orientation.
 *
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
 *
 * @see ui_app_main()
 * @see app_terminate_cb()
 */
void ui_app_exit(void);


/**
 * @brief Adds the system event handler
 *
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
