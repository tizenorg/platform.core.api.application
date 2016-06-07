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


#ifndef __TIZEN_APPFW_APP_INTERNAL_H__
#define __TIZEN_APPFW_APP_INTERNAL_H__

#include <app.h>
#include <appcore-common.h>
#include <tzplatform_config.h>

/* GNU gettext macro is already defined at appcore-common.h */
#ifdef _
#undef _
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define TIZEN_PATH_MAX 1024

#define PATH_FMT_APP_ROOT tzplatform_getenv(TZ_USER_APP)
#define PATH_FMT_RES_DIR "/res"
#define PATH_FMT_LOCALE_DIR "/locale"
#define PATH_FMT_DATA_DIR "/data"

#define PATH_FMT_RO_APP_ROOT tzplatform_getenv(TZ_SYS_RO_APP)
#define PATH_FMT_RO_RES_DIR "/res"
#define PATH_FMT_RO_LOCALE_DIR "/local"

struct app_event_handler {
	app_event_type_e type;
	app_event_cb cb;
	void *data;
};

struct app_event_info {
	app_event_type_e type;
	void *value;
};

app_device_orientation_e app_convert_appcore_rm(enum appcore_rm rm);

typedef void (*app_finalizer_cb) (void *data);

int app_error(app_error_e error, const char *function, const char *description);

app_device_orientation_e app_convert_appcore_rm(enum appcore_rm rm);

int app_get_package_app_name(const char *package, char **name);

int app_finalizer_add(app_finalizer_cb callback, void *data);

int app_finalizer_remove(app_finalizer_cb callback);

void app_finalizer_execute(void);

int app_get_package(char **package);

/**
 * @internal
 * @brief Called when the system memory is running low.
 *
 * @details When a low memory event is dispatched, the application should immediately save state and release resources to save as much memory as possible. \n
 *          If enough memory is not reclaimed during low memory conditions, the system will terminate some of the applications to reclaim the memory.
 *
 * @since_tizen 2.3
 * @param[in] user_data	The user data passed from the callback registration function
 * @see	app_main()
 * @see #app_event_callback_s
 */
typedef void (*app_low_memory_cb) (void *user_data);

/**
 * @internal
 * @brief Called when the battery power is running low.
 * @details When the battery level falls below 5%, it is called.
 *
 * @since_tizen 2.3
 * @param[in] user_data	The user data passed from the callback registration function
 * @see	app_main()
 * @see #app_event_callback_s
 */
typedef void (*app_low_battery_cb) (void *user_data);

/**
 * @internal
 * @brief Called when the orientation of the device changes.
 *
 * @since_tizen 2.3
 * @param[in] orientation The orientation of the device
 * @param[in] user_data	The user data passed from the callback registration function
 * @see	app_main()
 * @see #app_event_callback_s
 */
typedef void (*app_device_orientation_cb) (app_device_orientation_e orientation, void *user_data);

/**
 * @internal
 * @brief Called when language setting changes.
 *
 * @since_tizen 2.3
 * @param[in] user_data The user data passed from the callback registration function
 * @see	app_main()
 * @see #app_event_callback_s
 */
typedef void (*app_language_changed_cb) (void *user_data);

/**
 * @internal
 * @brief Called when region format setting changes.
 *
 * @since_tizen 2.3
 * @param[in] user_data The user data passed from the callback registration function
 * @see	app_main()
 * @see #app_event_callback_s
 */
typedef void (*app_region_format_changed_cb) (void *user_data);

/**
 * @internal
 * @brief The structure type containing the set of callback functions for handling application events.
 * @details It is one of the input parameters of the app_main() function.
 *
 * @since_tizen 2.3
 * @see app_main()
 * @see app_create_cb()
 * @see app_pause_cb()
 * @see app_resume_cb()
 * @see app_terminate_cb()
 * @see app_control_cb()
 * @see app_low_memory_cb()
 * @see app_low_battery_cb()
 * @see app_device_orientation_cb()
 * @see app_language_changed_cb()
 * @see app_region_format_changed_cb()
 */
typedef struct {
	app_create_cb create; /**< This callback function is called at the start of the application. */
	app_terminate_cb terminate; /**< This callback function is called once after the main loop of the application exits. */
	app_pause_cb pause; /**< This callback function is called each time the application is completely obscured by another application and becomes invisible to the user. */
	app_resume_cb resume; /**< This callback function is called each time the application becomes visible to the user. */
	app_control_cb app_control; /**< This callback function is called when another application sends the launch request to the application. */
	app_low_memory_cb low_memory; /**< The registered callback function is called when the system runs low on memory. */
	app_low_battery_cb low_battery; /**< The registered callback function is called when the battery is low. */
	app_device_orientation_cb device_orientation; /**< The registered callback function is called when the orientation of the device changes */
	app_language_changed_cb language_changed; /**< The registered callback function is called when language setting changes. */
	app_region_format_changed_cb region_format_changed; /**< The registered callback function is called when region format setting changes. */
} app_event_callback_s;

/**
 * @internal
 * @brief Runs the application's main loop until app_exit() is called.
 *
 * @details This function is the main entry point of the Tizen application.
 *          The app_create_cb() callback function is called to initialize the application before the main loop of application starts up.
 *          After the app_create_cb() callback function returns true, the main loop starts up and the app_control_cb() callback function is subsequently called.
 *          If the app_create_cb() callback function returns false, the main loop doesn't start up and app_terminate_cb() callback function is called.
 *
 * @since_tizen 2.3
 * @param[in] argc The argument count
 * @param[in] argv The argument vector
 * @param[in] callback The set of callback functions to handle application events
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
 * @see app_low_memory_cb()
 * @see app_low_battery_cb()
 * @see app_device_orientation_cb()
 * @see app_language_changed_cb()
 * @see app_region_format_changed_cb()
 * @see app_exit()
 * @see #app_event_callback_s
 */
int app_main(int argc, char **argv, app_event_callback_s *callback, void *user_data);

/**
 * @internal
 * @brief Runs the application's main loop until app_efl_exit() is called.
 *
 * @details This function is the main entry point of the Tizen application.
 *          The app_create_cb() callback function is called to initialize the application before the main loop of the application starts up.
 *          After the app_create_cb() callback function returns @c true, the main loop starts up and the app_control_cb() callback function is subsequently called.
 *          If the app_create_cb() callback function returns @c false, the main loop doesn't start up and the app_terminate_cb() callback function is called.
 *
 * @since_tizen 2.3
 * @param[in] argc The argument count
 * @param[in] argv The argument vector
 * @param[in] callback The set of callback functions to handle application events
 * @param[in] user_data The user data to be passed to the callback functions
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_ERROR_NONE Successful
 * @retval #APP_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_ERROR_INVALID_CONTEXT The application is illegally launched, not launched by the launch system
 * @retval #APP_ERROR_ALREADY_RUNNING The main loop has already started
 *
 * @see app_create_cb()
 * @see app_terminate_cb()
 * @see app_pause_cb()
 * @see app_resume_cb()
 * @see app_control_cb()
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
 * @internal
 * @brief Exits the main loop of application.
 *
 * @details The main loop of application stops and app_terminate_cb() is invoked.
 * @since_tizen 2.3
 * @see app_main()
 * @see app_terminate_cb()
 */
void app_exit(void);

/**
 * @internal
 * @brief Exits the main loop of the application.
 *
 * @details The main loop of the application stops and app_terminate_cb() is invoked.
 * @since_tizen 2.3
 * @see app_efl_main()
 * @see app_terminate_cb()
 */
void app_efl_exit(void);

int ui_app_init(int argc, char **argv, ui_app_lifecycle_callback_s *callback, void *user_data);

void ui_app_fini(void);

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_APPFW_APP_INTERNAL_H__ */

