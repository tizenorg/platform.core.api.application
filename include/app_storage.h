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

#ifndef __TIZEN_APPFW_STORAGE_H__
#define __TIZEN_APPFW_STORAGE_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C"
{
#endif

 /**
 * @addtogroup CAPI_STORAGE_MODULE
 * @{
 */


/**
 * @brief Enumerations of error code for Storage.
 */
typedef enum
{
	STORAGE_ERROR_NONE = TIZEN_ERROR_NONE, /**< Successful */
	STORAGE_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */
	STORAGE_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY, /**< Out of memory */
	STORAGE_ERROR_NOT_SUPPORTED = TIZEN_ERROR_NO_SUCH_DEVICE /**< Not supported storage */
} storage_error_e;


/**
 * @brief Enumerations of the storage type.
 */
typedef enum
{
	STORAGE_TYPE_INTERNAL, /**< Internal device storage (built-in storage in a device, non-removable) */
	STORAGE_TYPE_EXTERNAL, /**< External storage */
} storage_type_e;


/**
 * @brief Enumerations of the state of storage device.
 */
typedef enum
{
	STORAGE_STATE_UNMOUNTABLE = -2, /**< Storage is present but cannot be mounted. Typically it happens if the file system of the storage is corrupted. */
	STORAGE_STATE_REMOVED = -1, /**< Storage is not present. */
	STORAGE_STATE_MOUNTED = 0, /**< Storage is present and mounted with read/write access. */
	STORAGE_STATE_MOUNTED_READ_ONLY = 1, /**< Storage is present and mounted with read only access. */
} storage_state_e;


/**
* @brief   Called to get information once for each supported storage.
*
* @param [in] storage The unique storage ID
* @param [in] type The type of the storage
* @param [in] state The current state of the storage
* @param [in] path The absolute path to the root directory of the @a storage
* @param [in] user_data The user data passed from the foreach function
* @return @c true to continue with the next iteration of the loop, \n @c false to break out of the loop.
* @pre	storage_foreach_device_supported() will invoke this callback function.
* @see	storage_foreach_device_supported()
*/
typedef bool (*storage_device_supported_cb)(int storage, storage_type_e type, storage_state_e state, const char *path, void *user_data);


/**
* @brief   Called when the state of storage changes
*
* @param [in] storage The unique storage ID
* @param [in] state The current state of the storage
* @param [in] user_data The user data passed from the foreach function
* @pre	storage_set_state_changed_cb() will invoke this callback function.
* @see	storage_set_state_changed_cb()
* @see	storage_unset_state_changed_cb()
*/
typedef void (*storage_state_changed_cb)(int storage, storage_state_e state, void *user_data);


/**
 * @brief Retrieves all storage in device.
 * @details This function invokes the callback function once for each @a storage in device. \n
 * If storage_device_supported_cb() returns @c false, then iteration will be finished.
 *
 * @param [in] callback The iteration callback function
 * @param [in] user_data The user data to be passed to the callback function 
 * @return 0 on success, otherwise a negative error value.
 * @retval #STORAGE_ERROR_NONE Successful
 * @retval #STORAGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @post	This function invokes storage_device_supported_cb() repeatedly for each supported device.
 * @see storage_device_supported_cb()
 */
int storage_foreach_device_supported(storage_device_supported_cb callback, void *user_data);


/**
 * @brief   Gets the absolute path to the root directory of the given @a storage.
 * @details 
 * Files saved on the internal/external storage are readable or writeable by all applications.
 * When an application is uninstalled, the files written by that application are not removed from the internal/external storage.
 *
 * @remarks @a path must be released with free() by you.
 *
 * @param[in] storage The storage device
 * @param[out] path The absolute path to the storage directory
 * @return  0 on success, otherwise a negative error value.
 * @retval #STORAGE_ERROR_NONE Successful
 * @retval #STORAGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #STORAGE_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #STORAGE_ERROR_NOT_SUPPORTED  Not supported storage
 * @see storage_get_state()
 */ 
int storage_get_root_directory(int storage, char **path);


/**
 * @brief   Gets the type of the given @a storage.
 *
 * @param[in] storage The storage device
 * @param[out] type The type of the storage
 * @return  0 on success, otherwise a negative error value.
 * @retval #STORAGE_ERROR_NONE Successful
 * @retval #STORAGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #STORAGE_ERROR_NOT_SUPPORTED  Not supported storage
 */
int storage_get_type(int storage, storage_type_e *type);


/**
 * @brief   Gets the current state of the given @a storage.
 *
 * @param[in] storage The storage device
 * @param[out] state The current state of the storage,
 * @return  0 on success, otherwise a negative error value.
 * @retval #STORAGE_ERROR_NONE Successful
 * @retval #STORAGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #STORAGE_ERROR_NOT_SUPPORTED  Not supported storage
 * @see storage_get_root_directory()
 * @see storage_get_total_space()
 * @see storage_get_available_space()
 */
int storage_get_state(int storage, storage_state_e *state);


/**
 * @brief   Registers a callback function to be invoked when the state of the storage changes.
 *
 * @param[in] storage The storage device
 * @param[in] callback The callback function to register
 * @param[in] user_data The user data to be passed to the callback function
 * @return  0 on success, otherwise a negative error value.
 * @retval #STORAGE_ERROR_NONE Successful
 * @retval #STORAGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #STORAGE_ERROR_NOT_SUPPORTED  Not supported storage
 * @post storage_state_changed_cb() will be invoked if the state of registered storage changes.
 * @see storage_state_changed_cb()
 * @see storage_unset_state_changed_cb()
 */
int storage_set_state_changed_cb(int storage, storage_state_changed_cb callback, void *user_data);


/**
 * @brief Unregisters the callback function.
 *
 * @param [in] storage The storage device to monitor
 * @return 0 on success, otherwise a negative error value.
 * @retval #STORAGE_ERROR_NONE Successful
 * @retval #STORAGE_ERROR_NOT_SUPPORTED  Not supported storage
 * @see storage_state_changed_cb()
 * @see storage_set_state_changed_cb()
 */
int storage_unset_state_changed_cb(int storage);

/**
 * @brief   Gets the total space of the given @a storage in bytes.
 *
 * @param[in]	storage	The storage device
 * @param[out]	bytes	The total space size of the storage (bytes)
 * @return  0 on success, otherwise a negative error value
 * @retval #STORAGE_ERROR_NONE Successful
 * @retval #STORAGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #STORAGE_ERROR_NOT_SUPPORTED Not supported storage
 * @see storage_get_state()
 * @see storage_get_available_space()
 */
int storage_get_total_space(int storage, unsigned long long *bytes);

/**
 * @brief   Gets the available space size of the given @a storage in bytes.
 *
 * @param[in] storage The storage device
 * @param[out] bytes The available space size of the storage (bytes)
 * @return  0 on success, otherwise a negative error value.
 * @retval #STORAGE_ERROR_NONE Successful
 * @retval #STORAGE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #STORAGE_ERROR_NOT_SUPPORTED Not supported storage

 * @see storage_get_state()
 * @see storage_get_total_space()
 */
int storage_get_available_space(int storage, unsigned long long *bytes);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_APPFW_STORAGE_H__ */
