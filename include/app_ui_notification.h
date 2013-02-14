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


#ifndef __TIZEN_APPFW_UI_NOTIFICATION_H__
#define __TIZEN_APPFW_UI_NOTIFICATION_H__

#include <tizen.h>
#include <time.h>
#include <app_service.h>

#ifdef __cplusplus
extern "C"
{
#endif

 /**
 * @addtogroup CAPI_UI_NOTIFICATION_MODULE
 * @{
 */

/**
 * @brief Notification handle.
 */
typedef struct ui_notification_s *ui_notification_h;

/**
 * @brief Enumerations of error code for notification.
 */
typedef enum {
	UI_NOTIFICATION_ERROR_NONE = TIZEN_ERROR_NONE, /**< Successful */
	UI_NOTIFICATION_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */
	UI_NOTIFICATION_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY, /**< Out of memory */
	UI_NOTIFICATION_ERROR_DB_FAILED = TIZEN_ERROR_APPLICATION_CLASS | 0x31, /**< DB operation failed */
	UI_NOTIFICATION_ERROR_NO_SUCH_FILE = TIZEN_ERROR_NO_SUCH_FILE, /**< No such file */
	UI_NOTIFICATION_ERROR_INVALID_STATE = TIZEN_ERROR_APPLICATION_CLASS | 0x32, /**< Invalid state */
} ui_notification_error_e;

/**
 * @brief Enumeration of progress type for ongoing notification
 */
typedef enum {
	UI_NOTIFICATION_PROGRESS_TYPE_SIZE, /**< Size in bytes */
	UI_NOTIFICATION_PROGRESS_TYPE_PERCENTAGE, /**< Percentage (between 0.0 and 1.0) */
} ui_notification_progress_type_e;

/**
* @brief Called to retrieve the notifications posted.
* @remarks You should not free @a notification returned by this function.
* @param[in] notification The notification handle
* @param[in] user_data The user data passed from the foreach function
* @return @c true to continue with the next iteration of the loop, \n @c false to break out of the loop.
* @pre ui_notification_foreach_notification_posted() will invoke this callback.
* @see ui_notification_foreach_notification_posted()
* @see ui_notification_clone()
*/
typedef bool (*ui_notification_cb)(ui_notification_h notification, void *user_data);

/**
 * @brief Creates a notification handle.
 * @remarks The @a notification must be released with ui_notification_destroy() by you.
 * @param[in] ongoing A boolean value that sets whether this is an ongoing notification.
 * @param[out] notification A UI notification handle to be newly created on success
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see ui_notification_destroy()
 */
int ui_notification_create(bool ongoing, ui_notification_h *notification);

/**
 * @brief Destroys the notification handle and releases all its resources.
 * @param[in] notification The notification handle
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see ui_notification_create()
 */
int ui_notification_destroy(ui_notification_h notification);

/**
 * @brief Gets the identifier of the notification unique within the application
 * @remarks The system assigns an unique identifier to the notification when it is posted. \n
 * This function returns #UI_NOTIFICATION_ERROR_INVALID_STATE if the notification was not posted.
 * @param[in] notification The notification handle
 * @param[out] id The identifier for the notification unique within the application.
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #UI_NOTIFICATION_ERROR_INVALID_STATE The notification was not posted.
 * @see ui_notification_post()
 */
int ui_notification_get_id(ui_notification_h notification, int *id);

/**
 * @brief Checks whether the notification is ongoing or not
 * @param[in] notification The notification handle
 * @param[out] ongoing A boolean value that sets whether this is an ongoing notification.
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see ui_notification_create()
 */
int ui_notification_is_ongoing(ui_notification_h notification, bool *ongoing);

/**
 * @brief Sets the full path of the icon image to display in the notification.
 * @remarks The @a path should be the absolute path. If the icon is not set, the icon of the application will be displayed. \n
 * This function should be called before posting or updating the notification (see ui_notification_post(), ui_notification_update()).
 * @param[in] notification The notification handle
 * @param[in] path The absolute path to the specified icon \n
 *     If the @a path is NULL, it clears the previous value.
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see ui_notification_get_icon()
 */
int ui_notification_set_icon(ui_notification_h notification, const char *path);

/**
 * @brief Gets the absolute path to the icon to display in the notification.
 * @remarks The @a path must be released with free() by you.
 * @param[in] notification The notification handle
 * @param[out] path The absolute path to the icon
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see ui_notification_set_icon()
 */
int ui_notification_get_icon(ui_notification_h notification, char **path);

/**
 * @brief Sets the time that the notification occurred.
 * @remarks This function should be called before posting or updating the notification (see ui_notification_post(), ui_notification_update()).
 * @param[in] notification The notification handle
 * @param[in] time The time that the notification occurred \n
 *     If the @a time is NULL, it clears the previous value.
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see ui_notification_get_time()
 */
int ui_notification_set_time(ui_notification_h notification, struct tm *time);

/**
 * @brief Gets the time that the notification occured.
 * @param[in] notification The notification handle
 * @param[out] time The time that the notification occured
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see ui_notification_set_time()
 */
int ui_notification_get_time(ui_notification_h notification, struct tm **time);

/**
 * @brief Sets the title to display in the notification.
 * @remarks If the title is not set, the name of the application will be displayed. \n
 * This function should be called before posting or updating the notification (see ui_notification_post(), ui_notification_update()).
 * @param[in] notification The notification handle
 * @param[in] title The title to display in the notification \n
 *     If the @a title is NULL, it clears the previous value.
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see ui_notification_get_title()
 */
int ui_notification_set_title(ui_notification_h notification, const char *title);

/**
 * @brief Gets the title to display in the notification.
 * @remarks The @a title must be released with free() by you.
 * @param[in] notification The notification handle
 * @param[out] title The title to display in the notification
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see ui_notification_set_title()
 */
int ui_notification_get_title(ui_notification_h notification, char **title);

/**
 * @brief Sets the content to display in the notification
 * @remarks This function should be called before posting or updating the notification (see ui_notification_post(), ui_notification_update()).
 * @param[in] notification The notification handle
 * @param[in] content The content to display in the notification \n
 *     If the @a content is NULL, it clears the previous value.
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see ui_notification_get_content()
 */
int ui_notification_set_content(ui_notification_h notification, const char *content);

/**
 * @brief Gets the content to display in the notification
 * @remarks The @a content must be released with free() by you.
 * @param[in] notification The notification handle
 * @param[out] content The content to display in the notification
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see ui_notification_set_content()
 */
int ui_notification_get_content(ui_notification_h notification, char **content);

/**
 * @brief Sets the path of sound file to play when the notification is shown.
 * @remarks The @a path should be the absolute path. \n
 * The sound file is only supported wave file format. \n
 * This function should be called before posting or updating the notification (see ui_notification_post(), ui_notification_update()).
 * @param[in] notification The notification handle
 * @param[in] path The path of sound file to play when the notification is shown \n
 *     If the @a path is NULL, it clears the previous value.
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see ui_notification_get_sound()
 */
int ui_notification_set_sound(ui_notification_h notification, const char *path);

/**
 * @brief Gets the path of sound file to play when the notification is shown.
 * @remarks The @a path must be released with free() by you.
 * @param[in] notification The notification handle
 * @param[out] path The path of sound file to play when the notification is shown \n
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see ui_notification_set_sound()
 */
int ui_notification_get_sound(ui_notification_h notification, char **path);

/**
 * @brief Sets whether to use vibration when the notification is shown.
 * @remarks This function should be called before posting or updating the notification (see ui_notification_post(), ui_notification_update()).
 * @param[in] notification The notification handle
 * @param[in] value A boolean value that sets whether to use vibration.
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see ui_notification_get_vibration()
 */
int ui_notification_set_vibration(ui_notification_h notification, bool value);

/**
 * @brief Gets whether to use vibration when the notification is shown.
 * @param[in] notification The notification handle
 * @param[out] value A boolean value that sets whether to use vibration.
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see ui_notification_set_vibration()
 */
int ui_notification_get_vibration(ui_notification_h notification, bool *value);

/**
 * @brief Sets the service to launch when the notification is selected from the notification tray.
 * @details When the notification is selected from the notification tray, the application which is described by the specified service is launched. \n
 * If you want to launch the current application, use the explicit launch of the @ref CAPI_SERVICE_MODULE API
 * @remarks If the service is not set, the selected notification will be cleared from both the notification tray and the status bar without any action. \n
 * This function should be called before posting or updating the notification (see ui_notification_post(), ui_notification_update()).
 * @param[in] notification The notification handle
 * @param[in] service The service handle to launch when the notification is selected \n
 *     If the @a service is NULL, it clears the previous value.
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see ui_notification_get_service()
 * @see service_create()
 */
int ui_notification_set_service(ui_notification_h notification, service_h service);

/**
 * @brief Gets the service to launch when the notification is selected from the notification tray
 * @remarks The @a service must be released with service_destroy() by you.
 * @param[in] notification The notification handle
 * @param[out] service The service handle to launch when the notification is selected
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see ui_notification_set_service()
 */
int ui_notification_get_service(ui_notification_h notification, service_h *service);

/**
 * @brief Posts the notification to display in the notification tray and the status bar
 * @param[in] notification The notification handle
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_DB_FAILED DB failed
 * @retval #UI_NOTIFICATION_ERROR_NO_SUCH_FILE DB No such icon file
 * @retval #UI_NOTIFICATION_ERROR_INVALID_STATE The notification was already posted
 * @post The posted notification can be canceled or updated.
 * @see ui_notification_cancel()
 * @see ui_notification_cancel_all()
 * @see ui_notification_update()
 * @see ui_notification_update_progress()
 * @see ui_notification_foreach_notification_posted()
 */
int ui_notification_post(ui_notification_h notification);

/**
 * @brief Cancels the previously posted notification.
 * @details The previously posted notification is removed from the notification tray and the status bar.
 * @remarks The canceled @a notification is not be released automatically, must be released with ui_notification_destroy() by you.
 * @param[in] notification The notification handle
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_INVALID_STATE The notification was not posted or the notification was either cleared or canceled.
 * @pre The notification must be posted before canceling it.
 * @see ui_notification_post()
 * @see ui_notification_cancel_all()
 */
int ui_notification_cancel(ui_notification_h notification);

/**
 * @brief Cancels all previously posted notifications by the current application.
 * @details All previously posted notifications are removed from the notification tray and the status bar.
 * @remarks The notifications posted by other applications are not canceled from the notification tray and the status bar.
 * @see ui_notification_post()
 * @see ui_notification_cancel()
 */
void ui_notification_cancel_all(void);

/**
 * @brief Cancels selected type of previously posted notifications by the current application.
 * @details Selected type of previously posted notifications are removed from the notification tray and the status bar.
 * @remarks The notifications posted by other applications are not cancelled from the notification tray and the status bar.
 * @param[in] ongoing A boolean value that indicates whether the notification type is ongoing to cancel.
 * @see ui_notification_post()
 * @see ui_notification_cancel()
 * @see ui_notification_cancel_all()
 */
void ui_notification_cancel_all_by_type(bool ongoing);

/**
 * @brief Cancels selected type of previously posted notifications by the given application.
 * @details Selected type of previously posted notifications are removed from the notification tray and the status bar.
 * @remark This function is @b deprecated. Use app_manager_app_context_cb() instead.
 * @param[in] package The package name of the application to calcel the posted notifications.
 * @param[in] ongoing A boolean value that indicates whether the notification type is ongoing to cancel.
 * @see ui_notification_post()
 * @see ui_notification_cancel()
 * @see ui_notification_cancel_all()
 */
void ui_notification_cancel_all_by_package(const char *package, bool ongoing);

/**
 * @brief Cancels selected type of previously posted notifications by the given application ID.
 * @details Selected type of previously posted notifications are removed from the notification tray and the status bar.
 * @param[in] id The ID of the application to calcel the posted notifications.
 * @param[in] ongoing A boolean value that indicates whether the notification type is ongoing to cancel.
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @see ui_notification_post()
 * @see ui_notification_cancel()
 * @see ui_notification_cancel_all()
 */
int ui_notification_cancel_all_by_app_id(const char *app_id, bool ongoing);

/**
 * @brief Updates the notification posted.
 * @remarks You cannot update the notification which was cleared or canceled.
 * @param[in] notification The notification handle
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_DB_FAILED DB failed
 * @retval #UI_NOTIFICATION_ERROR_NO_SUCH_FILE DB No such icon file
 * @retval #UI_NOTIFICATION_ERROR_INVALID_STATE The notification was not posted or the notification was either cleared or canceled.
 * @pre The notification must be posted before updating it.
 * @see ui_notification_post()
 * @see ui_notification_update_progress()
 */
int ui_notification_update(ui_notification_h notification);

/**
 * @brief Updates the progress to the specified value
 * @remarks You cannot update the notification which was cleared or canceled.
 * @param[in] notification The notification handle \n
 *	It must be ongoing notification. \n
 *	If not, #UI_NOTIFICATION_ERROR_INVALID_PARAMETER will occur
 * @param[in] type The progress type
 * @param[in] value The value of the progress \n
 *    The @a value must be greater than or equal to zero. \n
 *    if @a type is #UI_NOTIFICATION_PROGRESS_TYPE_SIZE, it must be in bytes. \n
 *    If @a type is #UI_NOTIFICATION_PROGRESS_TYPE_PERCENTAGE, It must be a floating-point value between 0.0 and 1.0.
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_INVALID_STATE The notification was not posted or the notification was canceled.
 * @pre The notification must be posted before updating the progress to the specified value
 * @see ui_notification_create()
 * @see ui_notification_post()
 * @see ui_notification_update()
 * @see #ui_notification_progress_type_e
 */
int ui_notification_update_progress(ui_notification_h notification, ui_notification_progress_type_e type, double value);

/**
 * @brief Retrieves all posted notifications.
 * @details This function calls ui_notification_cb() once for each notification which was posted and is being shown. \n
 * If ui_notification_cb() callback function returns false, then iteration will be finished.
 *
 * @param [in] ongoing A boolean value that sets whether the type is an ongoing notification.
 * @param [in] callback The iteration callback function
 * @param [in] user_data The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @post This function invokes ui_notification_cb().
 * @see ui_notification_cb()
 */
int ui_notification_foreach_notification_posted(bool ongoing, ui_notification_cb callback, void *user_data);

/**
 * @brief Creates and returns a copy of the given notification handle.
 *
 * @remarks A newly created notification handle should be destroyed by calling ui_notification_destroy() if it is no longer needed.
 *
 * @param [out] clone If successful, a newly created notification handle will be returned.
 * @param [in] service The notification handle
 * @return 0 on success, otherwise a negative error value.
 * @retval #UI_NOTIFICATION_ERROR_NONE Successful
 * @retval #UI_NOTIFICATION_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #UI_NOTIFICATION_ERROR_OUT_OF_MEMORY Out of memory
 * @see ui_notification_create()
 * @see ui_notification_destroy()
 */
int ui_notification_clone(ui_notification_h *clone, ui_notification_h notification);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_APPFW_UI_NOTIFICATION_H__ */
