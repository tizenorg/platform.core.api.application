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


#ifndef __TIZEN_APPFW_SERVICE_H__
#define __TIZEN_APPFW_SERVICE_H__

#include <sys/types.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CAPI_SERVICE_MODULE
 * @{
 */


/**
 * @brief Service handle.
 */
typedef struct service_s *service_h;


/**
 * @brief Enumerations of error code for Service.
 */
typedef enum
{
	SERVICE_ERROR_NONE = TIZEN_ERROR_NONE, /**< Successful */
	SERVICE_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */
	SERVICE_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY, /**< Out of memory */
	SERVICE_ERROR_APP_NOT_FOUND = TIZEN_ERROR_APPLICATION_CLASS | 0x21, /**< The application was not found */
	SERVICE_ERROR_KEY_NOT_FOUND = TIZEN_ERROR_KEY_NOT_AVAILABLE, /**< Specified key not found */
	SERVICE_ERROR_KEY_REJECTED = TIZEN_ERROR_KEY_REJECTED, /**< Not available key */
	SERVICE_ERROR_INVALID_DATA_TYPE = TIZEN_ERROR_APPLICATION_CLASS | 0x22, /**< Invalid data type */
} service_error_e;


/**
 * @brief Enumeration of service result.
 */
typedef enum
{
	SERVICE_RESULT_SUCCEEDED = 0, /**< Operation succeeded */
	SERVICE_RESULT_FAILED = -1, /**< Operation failed by the callee */
	SERVICE_RESULT_CANCELED = -2, /**< Operation canceled by the framework */
} service_result_e;


/**
 * @brief	Service operation : default operation for explicit launch
 */
#define SERVICE_OPERATION_DEFAULT "http://tizen.org/appcontrol/operation/default"


/**
 * @brief	Service operation : provide explicit editable access to the given data. 
 */
#define SERVICE_OPERATION_EDIT "http://tizen.org/appcontrol/operation/edit"


/**
 * @brief	Service operation : display the data.
 */
#define SERVICE_OPERATION_VIEW "http://tizen.org/appcontrol/operation/view"


/**
 * @brief	Service operation : pick an item from the data, returning what was selected.
 */
#define SERVICE_OPERATION_PICK "http://tizen.org/appcontrol/operation/pick"


/**
 * @brief	Service operation : create a content, returning what was created.
 */
#define SERVICE_OPERATION_CREATE_CONTENT "http://tizen.org/appcontrol/operation/create_content"


/**
 * @brief	Service operation : perform a call to someone specified by the data.
 */
#define SERVICE_OPERATION_CALL "http://tizen.org/appcontrol/operation/call"


/**
 * @brief	Service operation : deliver some data to someone else.
 */
#define SERVICE_OPERATION_SEND "http://tizen.org/appcontrol/operation/send"


/**
 * @brief	Service operation : deliver text data to someone else.
 */
#define SERVICE_OPERATION_SEND_TEXT "http://tizen.org/appcontrol/operation/send_text"


/**
 * @brief	Service operation : dial a number as specified by the data.
 */
#define SERVICE_OPERATION_DIAL "http://tizen.org/appcontrol/operation/dial"


/**
 * @brief	Service operation : perform a search. 
 */
#define SERVICE_OPERATION_SEARCH "http://tizen.org/appcontrol/operation/search"


/**
 * @brief	Service optional data : the subject of a message.
 */
#define SERVICE_DATA_SUBJECT "http://tizen.org/appcontrol/data/subject"


/**
 * @brief	Service optional data : e-mail addresses.
 */
#define SERVICE_DATA_TO "http://tizen.org/appcontrol/data/to"


/**
 * @brief	Service optional data : e-mail addresses that should be carbon copied.
 */
#define SERVICE_DATA_CC "http://tizen.org/appcontrol/data/cc"


/**
 * @brief	Service optional data : e-mail addresses that should be blind carbon copied.
 */
#define SERVICE_DATA_BCC "http://tizen.org/appcontrol/data/bcc"


/**
 * @brief	Service optional data : the content of the data is associated with #SERVICE_OPERATION_SEND.
 */
#define SERVICE_DATA_TEXT "http://tizen.org/appcontrol/data/text"


/**
 * @brief	Service optional data : the title of the data
 */
#define SERVICE_DATA_TITLE "http://tizen.org/appcontrol/data/title"


/**
 * @brief	Service optional data : the path of selected item.
 */
#define SERVICE_DATA_SELECTED "http://tizen.org/appcontrol/data/selected"


/**
 * @brief   Called when the reply of the launch request is delivered.
 *
 * @remarks The @a request and @a reply must not be deallocated by an application. 
 *
 * @param   [in] request The service handle of the launch request that has sent
 * @param   [in] reply The service handle in which the results of the callee are contained
 * @param   [in] result The result code of the launch request
 * @param   [in] user_data	The user data passed from the callback registration function
 * @pre When the callee replies to the launch request, this callback will be invoked.
 * @see service_send_launch_request()
 * @see service_reply_to_launch_request()
 */
typedef void (*service_reply_cb) (service_h request, service_h reply, service_result_e result, void *user_data);


/**
* @brief   Called to retrieve the extra data that are contained in the service
*
* @remarks The @a key must not be deallocated by an application. 
*
* @param[in] service  The service handle
* @param[in] key The key of the value contained in the service
* @param[in] user_data The user data passed from the foreach function
* @return @c true to continue with the next iteration of the loop, \n @c false to break out of the loop.
* @pre	service_foreach_extra_data() will invoke this callback.
* @see	service_foreach_extra_data()
*/
typedef bool (*service_extra_data_cb)(service_h service, const char *key, void *user_data);


/**
* @brief   Called once for each matched application that can be launched to handle the given service request.
*
* @param [in] service  The service handle
* @param [in] package The package name of the application that can handle the launch request of the given service.
* @param [in] user_data The user data passed from the foreach function
* @return @c true to continue with the next iteration of the loop, \n @c false to break out of the loop.
* @pre	service_foreach_app_matched() will invoke this callback.
* @see service_foreach_app_matched()
*/
typedef bool (*service_app_matched_cb)(service_h service, const char *package, void *user_data);


/**
 * @brief Creates a service handle.
 *
 * @remarks The @a service must be released with service_destroy() by you. 
 * @param [out] service A service handle to be newly created on success
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 * @see	service_destroy()
 */
int service_create(service_h *service);


/**
 * @brief Destroys the service handle and releases all its resources.
 *
 * @param [in] service The service handle
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 * @see	service_create()
 */
int service_destroy(service_h service);


/**
 * @brief Sets the operation to be performed.
 *
 * @details The @a operation is the mandatory information for the launch request.
 * If the operation is not specified, #SERVICE_OPERATION_DEFAULT is used for the launch request.
 * If the operation is #SERVICE_OPERATION_DEFAULT, the package information is mandatory to explicitly launch the application
 * @param [in] service The service handle
 * @param [in] operation The operation to be performed \n
 *     If the @a operation is NULL, it clears the previous value.
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @see service_get_operation()
 * @see SERVICE_OPERATION_DEFAULT
 * @see SERVICE_OPERATION_EDIT
 * @see SERVICE_OPERATION_VIEW
 * @see SERVICE_OPERATION_PICK
 * @see SERVICE_OPERATION_CREATE_CONTENT
 * @see SERVICE_OPERATION_CALL
 * @see SERVICE_OPERATION_SEND
 * @see SERVICE_OPERATION_SEND_TEXT
 * @see SERVICE_OPERATION_DIAL
 * @see SERVICE_OPERATION_SEARCH
 */
int service_set_operation(service_h service, const char *operation);


/**
 * @brief Gets the operation to be performed.
 *
 * @remarks The @a operation must be released with free() by you.
 * @param [in] service The service handle
 * @param [out] operation The operation to be performed
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 * @see	service_set_operation()
 */
int service_get_operation(service_h service, char **operation);


/**
 * @brief Sets the URI of the data.
 *
 * @param [in] service The service handle
 * @param [in] uri The URI of the data this service is operating on \n
 *     If the @a uri is NULL, it clears the previous value.
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	service_get_uri()
 */
int service_set_uri(service_h service, const char *uri);


/**
 * @brief Gets the URI of the data.
 *
 * @remarks The @a uri must be released with free() by you.
 * @param [in] service The service handle
 * @param [out] uri The URI of the data this service is operating on
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 * @see	service_set_uri()
 */
int service_get_uri(service_h service, char **uri);


/**
 * @brief Sets the explicit MIME type of the data
 *
 * @param [in] service The service handle
 * @param [in] mime the explicit MIME type of the data this service is operating on \n
 *     If the @a mime is NULL, it clears the previous value.
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @see	service_get_mime()
 */
int service_set_mime(service_h service, const char *mime);


/**
 * @brief Gets the explicit MIME type of the data.
 *
 * @remarks The @a uri must be released with free() by you.
 * @param [in] service The service handle
 * @param [out] mime The explicit MIME type of the data this service is operating on
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 * @see	service_set_mime()
 */
int service_get_mime(service_h service, char **mime);


/**
 * @brief Sets the package name of the application to explicitly launch
 *
 * @remark This function is @b deprecated. Use service_set_app_id() instead.
 * @param [in] service The service handle
 * @param [in] package The package name of the application to explicitly launch \n
 *     If the @a package is NULL, it clears the previous value.
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 * @see	service_get_package()
 */
int service_set_package(service_h service, const char *package);


/**
 * @brief Gets the package name of the application to explicitly launch
 *
 * @remark This function is @b deprecated. Use service_get_app_id() instead.
 * @remarks The @a package must be released with free() by you.
 * @param [in] service The service handle
 * @param [out] package The package name of the application to explicitly launch
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 * @see	service_set_package()
 */
int service_get_package(service_h service, char **package);


/**
 * @brief Sets the ID of the application to explicitly launch
 *
 * @param [in] service The service handle
 * @param [in] app_id The ID of the application to explicitly launch \n
 *     If the @a app_id is NULL, it clears the previous value.
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 * @see	service_get_app_id()
 */
int service_set_app_id(service_h service, const char *app_id);


/**
 * @brief Gets the ID of the application to explicitly launch
 *
 * @remarks The @a app_id must be released with free() by you.
 * @param [in] service The service handle
 * @param [out] app_id The ID of the application to explicitly launch
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 * @see	service_set_app_id()
 */
int service_get_app_id(service_h service, char **app_id);

/**
 * @brief Sets the window id of the application
 *
 * @param [in] service The service handle
 * @param [in] id the window id of caller application \n
 *     If the @a id is not positive, it clears the previous value.
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 * @see service_get_window()
 */
int service_set_window(service_h service, unsigned int id);


/**
* @brief Gets the window id of the application
*
* @param [in] service The service handle
* @param [out] id The window id of caller application
* @return 0 on success, otherwise a negative error value.
* @retval #SERVICE_ERROR_NONE Successful
* @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
* @see service_set_package()
*/
int service_get_window(service_h service, unsigned int *id);


/**
 * @brief Adds the extra data to the service.
 *
 * @remarks The function replaces any existing value for the given key.
 * @remarks The function returns #SERVICE_ERROR_INVALID_PARAMETER if key or value is zero-length string.
 * @remarks The function returns #SERVICE_ERROR_KEY_REJECTED if the application tries to use same key with system-defined key
 * @param [in] service The service handle
 * @param [in] key The name of the extra data
 * @param [in] value The value associated with given key
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_KEY_REJECTED Not available key
 * @see service_add_extra_data_array()
 * @see service_remove_extra_data()
 * @see service_get_extra_data()
 */
int service_add_extra_data(service_h service, const char *key, const char *value);


/**
 * @brief Adds the extra data array to the service.
 *
 * @remarks The function replaces any existing value for the given key.
 * @remarks The function returns #SERVICE_ERROR_INVALID_PARAMETER if key is zero-length string.
 * @remarks The function returns #SERVICE_ERROR_KEY_REJECTED if the application tries to use same key with system-defined key
 * @param [in] service The service handle
 * @param [in] key The name of the extra data
 * @param [in] value The array value associated with given key
 * @param [in] length The length of the array
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_KEY_REJECTED Not available key
 * @see service_add_extra_data()
 * @see service_remove_extra_data()
 * @see service_get_extra_data()
 */
int service_add_extra_data_array(service_h service, const char *key, const char* value[], int length);


/**
 * @brief Removes the extra data from the service.
 *
 * @param [in] service The service handle
 * @param [in] key The name of the extra data
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_KEY_NOT_FOUND Specified key not found
 * @see service_add_extra_data()
 * @see service_add_extra_data_array()
 * @see service_get_extra_data()
 */
int service_remove_extra_data(service_h service, const char *key);


/**
 * @brief Gets the extra data from the service.
 *
 * @remarks The @a value must be released with free() by you.
 * @remarks The function returns #SERVICE_ERROR_INVALID_DATA_TYPE if the value is array data type.
 * @param [in] service The service handle
 * @param [int] key The name of the extra data
 * @param [out] value The value associated with given key
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_KEY_NOT_FOUND Specified key not found
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SERVICE_ERROR_INVALID_DATA_TYPE Invalid data type
 * @see service_add_extra_data()
 * @see service_add_extra_data_array()
 * @see service_get_extra_data()
 * @see service_remove_extra_data()
 * @see service_foreach_extra_data()
 */
int service_get_extra_data(service_h service, const char *key, char **value);


/**
 * @brief Gets the extra data array from the service.
 *
 * @remarks The @a value must be released with free() by you.
 * @remarks The function returns #SERVICE_ERROR_INVALID_DATA_TYPE if the value is not array data type.
 * @param [in] service The service handle
 * @param [int] key The name of the extra data
 * @param [out] value The array value associated with given key
 * @param [out] length The length of the array
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_KEY_NOT_FOUND Specified key not found
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SERVICE_ERROR_INVALID_DATA_TYPE Invalid data type
 * @see service_add_extra_data()
 * @see service_add_extra_data_array()
 * @see service_remove_extra_data()
 * @see service_foreach_extra_data()
 */
int service_get_extra_data_array(service_h service, const char *key, char ***value, int *length);


/**
 * @brief Checks whether if the extra data associated with given @a key is array data type.
 *
 * @param [in] service The service handle
 * @param [int] key The name of the extra data
 * @param [out] array @c True if the extra data is array data type, otherwise @c false
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @see service_add_extra_data()
 * @see service_add_extra_data_array()
 * @see service_remove_extra_data()
 * @see service_foreach_extra_data()
 */
int service_is_extra_data_array(service_h service, const char *key, bool *array);


/**
 * @brief Retrieves all extra data contained in service.
 * @details This function calls service_extra_data_cb() once for each key-value pair for extra data contained in service. \n
 * If service_extra_data_cb() callback function returns false, then iteration will be finished.
 *
 * @param [in] service The service handle
 * @param [in] callback The iteration callback function
 * @param [in] user_data The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @post This function invokes service_extra_data_cb().
 * @see service_extra_data_cb()
 */
int service_foreach_extra_data(service_h service, service_extra_data_cb callback, void *user_data);


/**
 * @brief Retrieves all applications that can be launched to handle the given service request.
 *
 * @param [in] service The service handle
 * @param [in] callback The iteration callback function
 * @param [in] user_data The user data to be passed to the callback function 
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Success
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @post This function invokes service_app_matched_cb().
 * @see service_app_matched_cb()
 */
int service_foreach_app_matched(service_h service, service_app_matched_cb callback, void *user_data);


/**
 * @brief Sends the launch request.
 *
 * @details The operation is mandatory information for the launch request. \n
 * If the operation is not specified, #SERVICE_OPERATION_DEFAULT is used by default.
 * If the operation is #SERVICE_OPERATION_DEFAULT, the application ID is mandatory to explicitly launch the application
 * @param [in] service The service handle
 * @param [in] callback The callback function to be called when the reply is delivered
 * @param [in] user_data The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #SERVICE_ERROR_APP_NOT_FOUND The application was not found to run the given launch request
 * @post If the launch request is sent for the result, the result will come back through service_reply_cb() from the callee application
 * @see service_reply_to_launch_request()
 * @see service_reply_cb()
 */
int service_send_launch_request(service_h service, service_reply_cb callback, void *user_data);


/**
 * @brief Replies to the launch request that the caller sent
 * @details If the caller application sent the launch request to receive the result, the callee application can return the result back to the caller.
 *
 * @param [in] reply The service handle in which the results of the callee are contained
 * @param [in] request The service handle that the caller sent
 * @param [in] result  The result code of the launch request
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 * @see service_send_launch_request()
 */
int service_reply_to_launch_request(service_h reply, service_h request, service_result_e result);


/**
 * @brief Creates and returns a copy of the given service handle.
 *
 * @remarks A newly created service should be destroyed by calling service_destroy() if it is no longer needed.
 *
 * @param [out] clone If successful, a newly created service handle will be returned.
 * @param [in] service The service handle
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 * @see	service_destroy()
 */
int service_clone(service_h *clone, service_h service);


/**
 * @brief Gets the application ID of the caller from the launch request
 *
 * @remarks The @a service must be the launch reqeust from app_service_cb().
 * @remarks This function returns #SERVICE_ERROR_INVALID_PARAMETER if the given service is not the launch request.
 * @remarks The @a id must be released with free() by you.
 * @param [in] service The service handle from app_service_cb()
 * @param [out] id The application ID of the caller
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 */
int service_get_caller(service_h service, char **id);


/**
 * @brief Check whether the caller is requesting a reply from the launch reqeust
 *
 * @remarks The @a service must be the launch reqeust from app_service_cb().
 * @remarks This function returns #SERVICE_ERROR_INVALID_PARAMETER if the given service is not the launch request.
 * @param [in] service The service handle from app_service_cb()
 * @param [out] requested whether a reply is requested by the caller
 * @return 0 on success, otherwise a negative error value.
 * @retval #SERVICE_ERROR_NONE Successful
 * @retval #SERVICE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ERROR_OUT_OF_MEMORY Out of memory
 */
int service_is_reply_requested(service_h service, bool *requested);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_APPFW_SERVICE_H__ */
