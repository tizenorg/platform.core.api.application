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


#ifndef __TIZEN_APPFW_APP_CONTROL_H__
#define __TIZEN_APPFW_APP_CONTROL_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file app_control.h
 */

/**
 * @addtogroup CAPI_APP_CONTROL_MODULE
 * @{
 */


typedef struct _bundle_t bundle;
typedef unsigned char bundle_raw;

/**
 * @brief App Control handle.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef struct app_control_s* app_control_h;


/**
 * @brief Enumeration for App Control Error.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef enum
{
	APP_CONTROL_ERROR_NONE = TIZEN_ERROR_NONE, /**< Successful */
	APP_CONTROL_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */
	APP_CONTROL_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY, /**< Out of memory */
	APP_CONTROL_ERROR_APP_NOT_FOUND = TIZEN_ERROR_APPLICATION | 0x21, /**< The application is not found */
	APP_CONTROL_ERROR_KEY_NOT_FOUND = TIZEN_ERROR_KEY_NOT_AVAILABLE, /**< Specified key is not found */
	APP_CONTROL_ERROR_KEY_REJECTED = TIZEN_ERROR_KEY_REJECTED, /**< Key is not available */
	APP_CONTROL_ERROR_INVALID_DATA_TYPE = TIZEN_ERROR_APPLICATION | 0x22, /**< Invalid data type */
	APP_CONTROL_ERROR_LAUNCH_REJECTED = TIZEN_ERROR_APPLICATION | 0x23, /**< The application cannot be launched now*/
	APP_CONTROL_ERROR_PERMISSION_DENIED = TIZEN_ERROR_PERMISSION_DENIED,	/**< Permission denied */
	APP_CONTROL_ERROR_LAUNCH_FAILED = TIZEN_ERROR_APPLICATION | 0x24, /**< Internal launch error */
	APP_CONTROL_ERROR_TIMED_OUT = TIZEN_ERROR_TIMED_OUT, /**< Time out */
	APP_CONTROL_ERROR_IO_ERROR = TIZEN_ERROR_IO_ERROR	/**< IO error */
} app_control_error_e;


/**
 * @brief Enumeration for App Control Result.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef enum
{
	APP_CONTROL_RESULT_SUCCEEDED = 0, /**< Operation succeeded */
	APP_CONTROL_RESULT_FAILED = -1, /**< Operation failed by the callee */
	APP_CONTROL_RESULT_CANCELED = -2, /**< Operation canceled by the framework */
} app_control_result_e;

/**
 * @brief Enumeration for App Control Launch Mode.
 * @since_tizen 2.4
 */
typedef enum {
	APP_CONTROL_LAUNCH_MODE_SINGLE = 0,
	APP_CONTROL_LAUNCH_MODE_GROUP,
} app_control_launch_mode_e;

/**
 * @brief Definition for the app_control operation: main operation for an explicit launch.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_OPERATION_MAIN "http://tizen.org/appcontrol/operation/main"


/**
 * @brief Definition for the app_control operation: default operation for an explicit launch.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_OPERATION_DEFAULT "http://tizen.org/appcontrol/operation/default"


/**
 * @brief Definition for the app_control operation: provides an explicit editable access to the given data.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_OPERATION_EDIT "http://tizen.org/appcontrol/operation/edit"


/**
 * @brief Definition for the app_control operation: displays the data.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_OPERATION_VIEW "http://tizen.org/appcontrol/operation/view"


/**
 * @brief Definition for the app_control operation: picks an item from the data, returning what is selected.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_OPERATION_PICK "http://tizen.org/appcontrol/operation/pick"


/**
 * @brief Definition for the app_control operation: creates content, returning what is created.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_OPERATION_CREATE_CONTENT "http://tizen.org/appcontrol/operation/create_content"


/**
 * @brief Definition for the app_control operation: performs a call to someone specified by the data.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/call
 * @remarks When you request this operation, you must declare this privilege.
 */
#define APP_CONTROL_OPERATION_CALL "http://tizen.org/appcontrol/operation/call"


/**
 * @brief Definition for the app_control operation: delivers some data to someone else.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_OPERATION_SEND "http://tizen.org/appcontrol/operation/send"


/**
 * @brief Definition for the app_control operation: delivers text data to someone else.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_OPERATION_SEND_TEXT "http://tizen.org/appcontrol/operation/send_text"


/**
 * @brief Definition for the app_control operation: shares an item with someone else.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_OPERATION_SHARE "http://tizen.org/appcontrol/operation/share"


/**
 * @brief Definition for the app_control operation: shares multiple items with someone else.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_OPERATION_MULTI_SHARE "http://tizen.org/appcontrol/operation/multi_share"


/**
 * @brief Definition for the app_control operation: shares text data with someone else.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_OPERATION_SHARE_TEXT "http://tizen.org/appcontrol/operation/share_text"


/**
 * @brief Definition for the app_control operation: dials a number as specified by the data.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_OPERATION_DIAL "http://tizen.org/appcontrol/operation/dial"


/**
 * @brief Definition for the app_control operation: performs a search.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_OPERATION_SEARCH "http://tizen.org/appcontrol/operation/search"


/**
 * @brief Definition for the app_control operation: downloads an item.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_OPERATION_DOWNLOAD "http://tizen.org/appcontrol/operation/download"


/**
 * @brief Definition for the app_control operation: prints content.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_OPERATION_PRINT "http://tizen.org/appcontrol/operation/print"

/**
 * @brief Definition for the app_control operation: composes.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_OPERATION_COMPOSE "http://tizen.org/appcontrol/operation/compose"

/**
 * @brief Definition for the app_control operation: can be launched by interested event.
 * @since_tizen 2.4
 * @remarks This operation is for handling event from the platform or other application. This operation can not be requested via app_control_send_launch_request().
 * @remarks Refer to "Launch on Event" section of Event module.
 */
#define APP_CONTROL_OPERATION_LAUNCH_ON_EVENT "http://tizen.org/appcontrol/operation/launch_on_event"

/**
 * @brief Definition for app_control optional data: the subject of a message.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_DATA_SUBJECT "http://tizen.org/appcontrol/data/subject"


/**
 * @brief Definition for app_control optional data: e-mail addresses.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_DATA_TO "http://tizen.org/appcontrol/data/to"


/**
 * @brief Definition for app_control optional data: e-mail addresses that should be carbon copied.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_DATA_CC "http://tizen.org/appcontrol/data/cc"


/**
 * @brief Definition for app_control optional data: e-mail addresses that should be blind carbon copied.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_DATA_BCC "http://tizen.org/appcontrol/data/bcc"


/**
 * @brief Definition for app_control optional data: the content of the data is associated with #APP_CONTROL_OPERATION_SEND.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_DATA_TEXT "http://tizen.org/appcontrol/data/text"


/**
 * @brief Definition for app_control optional data: the title of the data.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_DATA_TITLE "http://tizen.org/appcontrol/data/title"


/**
 * @brief Definition for app_control optional data: the path of a selected item.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_DATA_SELECTED "http://tizen.org/appcontrol/data/selected"


/**
 * @brief Definition for app_control optional data: multiple item path to deliver.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_DATA_PATH "http://tizen.org/appcontrol/data/path"


/**
 * @brief Definition for app_control optional data: the selection type.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
#define APP_CONTROL_DATA_SELECTION_MODE "http://tizen.org/appcontrol/data/selection_mode"


/**
 * @brief Called when the reply of the launch request is delivered.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @remarks The @a request and @a reply must not be deallocated by the application.
 *
 * @param[in] request The app_control handle of the launch request that has been sent
 * @param[in] reply The app_control handle in which the results of the callee are contained
 * @param[in] result The result code of the launch request
 * @param[in] user_data	The user data passed from the callback registration function
 * @pre When the callee replies to the launch request, this callback will be invoked.
 * @see app_control_send_launch_request()
 * @see app_control_reply_to_launch_request()
 */
typedef void (*app_control_reply_cb) (app_control_h request, app_control_h reply, app_control_result_e result, void *user_data);


/**
 * @brief Called to retrieve the extra data contained in the app_control.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @remarks The @a key must not be deallocated by the application.
 *
 * @param[in] app_control  The app_control handle
 * @param[in] key The key of the value contained in the app_control
 * @param[in] user_data The user data passed from the foreach function
 * @return @c true to continue with the next iteration of the loop,
 *         otherwise @c false to break out of the loop
 * @pre	app_control_foreach_extra_data() will invoke this callback.
 * @see	app_control_foreach_extra_data()
 */
typedef bool (*app_control_extra_data_cb)(app_control_h app_control, const char *key, void *user_data);


/**
 * @brief Called once for each matched application that can be launched to handle the given app_control request.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] app_control  The app_control handle
 * @param[in] package The package name of the application that can handle the launch request of the given app_control
 * @param[in] user_data The user data passed from the foreach function
 * @return @c true to continue with the next iteration of the loop,
 *         otherwise @c false to break out of the loop
 * @pre	app_control_foreach_app_matched() will invoke this callback.
 * @see app_control_foreach_app_matched()
 */
typedef bool (*app_control_app_matched_cb)(app_control_h app_control, const char *appid, void *user_data);


typedef int (*app_control_host_res_fn)(void *data);

/**
 * @brief Creates an app_control handle.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks The @a app_control must be released using app_control_destroy().
 * @param[out] app_control The app_control handle to be newly created on success
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 * @see	app_control_destroy()
 */
int app_control_create(app_control_h *app_control);


/**
 * @brief Destroys the app_control handle and releases all its resources.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] app_control The app_control handle
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 * @see	app_control_create()
 */
int app_control_destroy(app_control_h app_control);


/**
 * @brief Sets the operation to be performed.
 *
 * @details The @a operation is the mandatory information for the launch request.
 *          If the operation is not specified, #APP_CONTROL_OPERATION_DEFAULT is used for the launch request.
 *          If the operation is #APP_CONTROL_OPERATION_DEFAULT, the package information is mandatory to explicitly launch the application.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] app_control The app_control handle
 * @param[in] operation The operation to be performed (if the @a operation is @c NULL, it clears the previous value)
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @see app_control_get_operation()
 * @see APP_CONTROL_OPERATION_DEFAULT
 * @see APP_CONTROL_OPERATION_EDIT
 * @see APP_CONTROL_OPERATION_VIEW
 * @see APP_CONTROL_OPERATION_PICK
 * @see APP_CONTROL_OPERATION_CREATE_CONTENT
 * @see APP_CONTROL_OPERATION_CALL
 * @see APP_CONTROL_OPERATION_SEND
 * @see APP_CONTROL_OPERATION_SEND_TEXT
 * @see APP_CONTROL_OPERATION_DIAL
 * @see APP_CONTROL_OPERATION_SEARCH
 */
int app_control_set_operation(app_control_h app_control, const char *operation);


/**
 * @brief Gets the operation to be performed.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks The @a operation must be released using free().
 * @param[in] app_control The app_control handle
 * @param[out] operation The operation to be performed
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 * @see	app_control_set_operation()
 */
int app_control_get_operation(app_control_h app_control, char **operation);


/**
 * @brief Sets the URI of the data.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] app_control The app_control handle
 * @param[in] uri The URI of the data this app_control is operating on (if the @a uri is @c NULL, it clears the previous value)
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	app_control_get_uri()
 */
int app_control_set_uri(app_control_h app_control, const char *uri);


/**
 * @brief Gets the URI of the data.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks The @a uri must be released using free().
 * @param[in] app_control The app_control handle
 * @param[out] uri The URI of the data this app_control is operating on
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 * @see	app_control_set_uri()
 */
int app_control_get_uri(app_control_h app_control, char **uri);


/**
 * @brief Sets the explicit MIME type of the data.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] app_control The app_control handle
 * @param[in] mime The explicit MIME type of the data this app_control is operating on (if the @a mime is @c NULL, it clears the previous value)
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	app_control_get_mime()
 */
int app_control_set_mime(app_control_h app_control, const char *mime);


/**
 * @brief Gets the explicit MIME type of the data.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks The @a uri must be released using free().
 * @param[in] app_control The app_control handle
 * @param[out] mime The explicit MIME type of the data this app_control is operating on
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 * @see	app_control_set_mime()
 */
int app_control_get_mime(app_control_h app_control, char **mime);


/**
 * @brief Sets the explicit category.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] app_control The app_control handle
 * @param[in] category The explicit category (if the @a category is @c NULL, it clears the previous value)
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	app_control_get_category()
 */
int app_control_set_category(app_control_h app_control, const char *category);


/**
 * @brief Gets the explicit category.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks The @a category must be released using free().
 * @param[in] app_control The app_control handle
 * @param[out] category The explicit category
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 * @see	app_control_set_category()
 */
int app_control_get_category(app_control_h app_control, char **category);


/**
 * @brief Sets the ID of the application to explicitly launch.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] app_control The app_control handle
 * @param[in] app_id The ID of the application to explicitly launch (if the @a app_id is @c NULL, it clears the previous value)
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 * @see	app_control_get_app_id()
 */
int app_control_set_app_id(app_control_h app_control, const char *app_id);


/**
 * @brief Gets the ID of the application to explicitly launch.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks The @a app_id must be released with free().
 * @param[in] app_control The app_control handle
 * @param[out] app_id The ID of the application to explicitly launch
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 * @see	app_control_set_app_id()
 */
int app_control_get_app_id(app_control_h app_control, char **app_id);

/**
 * @brief Adds extra data to the app_control.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks The function replaces any existing value for the given key.
 * @remarks The function returns #APP_CONTROL_ERROR_INVALID_PARAMETER if @a key or @a value is a zero-length string.
 * @remarks The function returns #APP_CONTROL_ERROR_KEY_REJECTED if the application tries to use the same key with system-defined key.
 * @param[in] app_control The app_control handle
 * @param[in] key The name of the extra data
 * @param[in] value The value associated with the given key
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_KEY_REJECTED Key not available
 * @see app_control_add_extra_data_array()
 * @see app_control_remove_extra_data()
 * @see app_control_get_extra_data()
 */
int app_control_add_extra_data(app_control_h app_control, const char *key, const char *value);


/**
 * @brief Adds the extra data array to the app_control.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks The function replaces any existing value for the given key.
 * @remarks The function returns #APP_CONTROL_ERROR_INVALID_PARAMETER if @a key is a zero-length string.
 * @remarks The function returns #APP_CONTROL_ERROR_KEY_REJECTED if the application tries to use the same key with system-defined key.
 * @param[in] app_control The app_control handle
 * @param[in] key The name of the extra data
 * @param[in] value The array value associated with the given key
 * @param[in] length The length of the array
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_KEY_REJECTED Key not available
 * @see app_control_add_extra_data()
 * @see app_control_remove_extra_data()
 * @see app_control_get_extra_data()
 */
int app_control_add_extra_data_array(app_control_h app_control, const char *key, const char* value[], int length);


/**
 * @brief Removes the extra data from the app_control.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] app_control The app_control handle
 * @param[in] key The name of the extra data
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_KEY_NOT_FOUND Specified key not found
 * @see app_control_add_extra_data()
 * @see app_control_add_extra_data_array()
 * @see app_control_get_extra_data()
 */
int app_control_remove_extra_data(app_control_h app_control, const char *key);


/**
 * @brief Gets the extra data from the app_control.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks The @a value must be released using free().
 * @remarks The function returns #APP_CONTROL_ERROR_INVALID_DATA_TYPE if @a value is of array data type.
 * @param[in] app_control The app_control handle
 * @param[in] key The name of the extra data
 * @param[out] value The value associated with the given key
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_KEY_NOT_FOUND Specified key not found
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #APP_CONTROL_ERROR_INVALID_DATA_TYPE Invalid data type
 * @see app_control_add_extra_data()
 * @see app_control_add_extra_data_array()
 * @see app_control_get_extra_data()
 * @see app_control_remove_extra_data()
 * @see app_control_foreach_extra_data()
 */
int app_control_get_extra_data(app_control_h app_control, const char *key, char **value);


/**
 * @brief Gets the extra data array from the app_control.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks The @a value must be released using free().
 * @remarks The function returns #APP_CONTROL_ERROR_INVALID_DATA_TYPE if @a value is not of array data type.
 * @param[in] app_control The app_control handle
 * @param[in] key The name of the extra data
 * @param[out] value The array value associated with the given key
 * @param[out] length The length of the array
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_KEY_NOT_FOUND Specified key not found
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #APP_CONTROL_ERROR_INVALID_DATA_TYPE Invalid data type
 * @see app_control_add_extra_data()
 * @see app_control_add_extra_data_array()
 * @see app_control_remove_extra_data()
 * @see app_control_foreach_extra_data()
 */
int app_control_get_extra_data_array(app_control_h app_control, const char *key, char ***value, int *length);


/**
 * @brief Checks whether the extra data associated with the given @a key is of array data type.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] app_control The app_control handle
 * @param[in] key The name of the extra data
 * @param[out] array If @c true the extra data is of array data type,
 *                   otherwise @c false
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @see app_control_add_extra_data()
 * @see app_control_add_extra_data_array()
 * @see app_control_remove_extra_data()
 * @see app_control_foreach_extra_data()
 */
int app_control_is_extra_data_array(app_control_h app_control, const char *key, bool *array);


/**
 * @brief Retrieves all extra data contained in app_control.
 * @details This function calls app_control_extra_data_cb() once for each key-value pair for extra data contained in app_control. \n
 *          If the app_control_extra_data_cb() callback function returns @c false, then iteration will be finished.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] app_control The app_control handle
 * @param[in] callback The iteration callback function
 * @param[in] user_data The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @post This function invokes app_control_extra_data_cb().
 * @see app_control_extra_data_cb()
 */
int app_control_foreach_extra_data(app_control_h app_control, app_control_extra_data_cb callback, void *user_data);


/**
 * @brief Retrieves all applications that can be launched to handle the given app_control request.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] app_control The app_control handle
 * @param[in] callback The iteration callback function
 * @param[in] user_data The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Success
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @post This function invokes app_control_app_matched_cb().
 * @see app_control_app_matched_cb()
 */
int app_control_foreach_app_matched(app_control_h app_control, app_control_app_matched_cb callback, void *user_data);


/**
 * @brief Sends the launch request.
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
int app_control_send_launch_request(app_control_h app_control, app_control_reply_cb callback, void *user_data);


/**
 * @brief Sends the terminate request to the application that is launched by app_control. This API is only effective for some applications that are provided by default for handling platform default app_controls. You are not allowed to terminate other general applications using this API.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] app_control The app_control handle
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_PERMISSION_DENIED Permission denied
 * @see app_control_send_launch_request()
 */
int app_control_send_terminate_request(app_control_h app_control);


/**
 * @brief Replies to the launch request sent by the caller.
 * @details If the caller application sent the launch request to receive the result, the callee application can return the result back to the caller.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @param[in] reply The app_control handle in which the results of the callee are contained
 * @param[in] request The app_control handle sent by the caller
 * @param[in] result  The result code of the launch request
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 * @see app_control_send_launch_request()
 */
int app_control_reply_to_launch_request(app_control_h reply, app_control_h request, app_control_result_e result);


/**
 * @brief Creates and returns a copy of the given app_control handle.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks A newly created app_control should be destroyed by calling app_control_destroy() if it is no longer needed.
 *
 * @param[out] clone If successful, a newly created app_control handle will be returned
 * @param[in] app_control The app_control handle
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 * @see	app_control_destroy()
 */
int app_control_clone(app_control_h *clone, app_control_h app_control);


/**
 * @brief Gets the application ID of the caller from the launch request.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks The @a app_control must be the launch request from app_control_cb().
 * @remarks This function returns #APP_CONTROL_ERROR_INVALID_PARAMETER if the given app_control is not the launch request.
 * @remarks The @a id must be released using free().
 * @param[in] app_control The app_control handle from app_control_cb()
 * @param[out] id The application ID of the caller
 * @return @a 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 */
int app_control_get_caller(app_control_h app_control, char **id);


/**
 * @brief Checks whether the caller is requesting a reply from the launch request.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @remarks The @a app_control must be the launch request from app_control_cb().
 * @remarks This function returns #APP_CONTROL_ERROR_INVALID_PARAMETER if the given app_control is not the launch request.
 * @param[in] app_control The app_control handle from app_control_cb()
 * @param[out] requested If @c true a reply is requested by the caller,
 *                       otherwise @c false
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #APP_CONTROL_ERROR_OUT_OF_MEMORY Out of memory
 */
int app_control_is_reply_requested(app_control_h app_control, bool *requested);

/**
 * @brief Sets the launch mode of the application.
 *
 * @since_tizen 2.4
 * @param[in] app_control The app_control handle
 * @param[in] launch_mode The launch mode of app
 *
 * @return 0 on success, otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @see app_control_set_launch_mode()
 * @see app_control_launch_mode_e
 */
int app_control_set_launch_mode(app_control_h app_control,
		app_control_launch_mode_e mode);

/**
 * @brief Gets the launch mode of the application.
 *
 * @since_tizen 2.4
 * @param[in] app_control The app_control handle
 * @param[out] launch_mode The launch mode of app
 *
 * @return 0 on success, otherwise a negative error value
 * @retval #APP_CONTROL_ERROR_NONE Successful
 * @retval #APP_CONTROL_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	app_control_get_launch_mode()
 * @see app_control_launch_mode_e
 */
int app_control_get_launch_mode(app_control_h app_control,
		app_control_launch_mode_e *mode);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_APPFW_APP_CONTROL_H__ */
