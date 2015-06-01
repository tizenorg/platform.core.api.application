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


#ifndef __TIZEN_APPFW_PREFERENCE_H__
#define __TIZEN_APPFW_PREFERENCE_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CAPI_PREFERENCE_MODULE 
 * @{
 */


/**
 * @brief Enumerations of error code for Preference.
 */
typedef enum
{
	PREFERENCE_ERROR_NONE = TIZEN_ERROR_NONE, /**< Successful */
	PREFERENCE_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */
	PREFERENCE_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY, /**< Out of memory */
	PREFERENCE_ERROR_NO_KEY = TIZEN_ERROR_KEY_NOT_AVAILABLE, /**< Required key not available */
	PREFERENCE_ERROR_IO_ERROR = TIZEN_ERROR_IO_ERROR , /**< Internal I/O Error */
} preference_error_e;


/**
 * @brief	Called when the given key's value in the preference changes.
 *
 * @details When the @a key is added or removed, this callback function is skipped. (only update can be handled)
 *
 * @param   [in] key	The name of the key in the preference
 * @param   [in] user_data The user data passed from the callback registration function
 * @pre		This function is invoked when the value of the key is overwritten after you register this callback using preference_set_changed_cb()
 * @see preference_set_changed_cb()
 * @see preference_unset_changed_cb()
 * @see	preference_set_boolean()
 * @see preference_set_int()
 * @see preference_set_string()
 * @see preference_set_double()
 */
typedef void (*preference_changed_cb) (const char *key, void *user_data);


/**
* @brief   Called to get key string once for each key-value pair in the preference.
*
* @remarks You should not free @a key returned by this function.
*
* @param	[in] key The key of the value added to the preference
* @param	[in] value The value associated with the key
* @param	[in] user_data The user data passed from the foreach function
* @return @c true to continue with the next iteration of the loop, \n @c false to break out of the loop.
* @pre		preference_foreach_item() will invoke this callback function.
* @see		preference_foreach_item()
*/
typedef bool (*preference_item_cb)(const char *key, void *user_data);


/**
 * @brief Sets an integer value in the preference.
 *
 * @param [in] key	The name of the key to modify
 * @param [in] value  The new @c int value for the given key
 * @return 0 on success, otherwise a negative error value.
 * @retval #PREFERENCE_ERROR_NONE Successful
 * @retval #PREFERENCE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PREFERENCE_ERROR_IO_ERROR Internal I/O Error
 * @see	preference_get_int()
 *
 */
int preference_set_int(const char *key, int value);


/**
 * @brief Gets a integer value from the preference.
 *
 * @param [in] key The name of the key to retrieve
 * @param [out] value  The @c int value for the given key
 * @return 0 on success, otherwise a negative error value.
 * @retval #PREFERENCE_ERROR_NONE	Successful
 * @retval #PREFERENCE_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval #PREFERENCE_ERROR_NO_KEY	Required key not available
 * @retval #PREFERENCE_ERROR_IO_ERROR Internal I/O Error
 * @see	preference_set_int()
 */
int preference_get_int(const char *key, int *value);


/**
 * @brief Sets a double value in the preference.
 *
 * @param [in] key The name of the key to modify
 * @param [in] value  The new @c double value associated with given key
 * @return 0 on success, otherwise a negative error value.
 * @retval #PREFERENCE_ERROR_NONE	Successful
 * @retval #PREFERENCE_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval #PREFERENCE_ERROR_IO_ERROR Internal I/O Error
 * @see	preference_get_double()
 *
 */
int preference_set_double(const char *key, double value);


/**
 * @brief Gets a double value from the preference.
 *
 * @param [in] key The name of the key to retrieve
 * @param [out] value  The @c double value associated with given key
 * @return 0 on success, otherwise a negative error value.
 * @retval #PREFERENCE_ERROR_NONE	Successful
 * @retval #PREFERENCE_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval #PREFERENCE_ERROR_NO_KEY	Required key not available
 * @retval #PREFERENCE_ERROR_IO_ERROR Internal I/O Error
 * @see	preference_set_double()
 *
 */
int preference_get_double(const char *key, double *value);


/**
 * @brief Sets a string value in the preference.
 *
 * @details It makes a deep copy of the added string value.
 * 
 * @param [in] key The name of the key to modify
 * @param [in] value  The new @c string value associated with given key
 * @return 0 on success, otherwise a negative error value.
 * @retval #PREFERENCE_ERROR_NONE	Successful
 * @retval #PREFERENCE_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval #PREFERENCE_ERROR_IO_ERROR Internal I/O Error
 * @see	preference_get_string()
 *
 */
int preference_set_string(const char *key, const char *value);


/**
 * @brief Gets a string value from the preference.
 *
 * @remarks @a value must be released with free() by you.
 * @param [in]	key	The name of the key to retrieve
 * @param [out] value	The @c string value associated with given key
 * @return 0 on success, otherwise a negative error value.
 * @retval #PREFERENCE_ERROR_NONE	Successful
 * @retval #PREFERENCE_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval #PREFERENCE_ERROR_OUT_OF_MEMORY	Out of memory
 * @retval #PREFERENCE_ERROR_NO_KEY	Required key not available
 * @retval #PREFERENCE_ERROR_IO_ERROR Internal I/O Error
 * @see	preference_set_string()
 */
int preference_get_string(const char *key, char **value);


/**
 * @brief Sets a boolean value in the preference.
 *
 * @param [in] key The name of the key to modify
 * @param [in] value  The new boolean @c value associated with given key
 * @return 0 on success, otherwise a negative error value.
 * @retval #PREFERENCE_ERROR_NONE	Successful
 * @retval #PREFERENCE_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval #PREFERENCE_ERROR_IO_ERROR Internal I/O Error   
 * @see	preference_get_boolean()
 */
int preference_set_boolean(const char *key, bool value);


/**
 * @brief Gets a boolean value from the preference.
 *
 * @param [in] key The name of the key to retrieve
 * @param [out] value  The boolean @c value associated with given key
 * @return 0 on success, otherwise a negative error value.
 * @retval #PREFERENCE_ERROR_NONE	Successful
 * @retval #PREFERENCE_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval #PREFERENCE_ERROR_NO_KEY	Required key not available
 * @retval #PREFERENCE_ERROR_IO_ERROR Internal I/O Error
 * @see	preference_set_boolean()
 */
int preference_get_boolean(const char *key, bool *value);


/**
 * @brief Removes any value with the given @a key from the preference.
 *
 * @param [in] key The name of the key to remove
 * @return 0 on success, otherwise a negative error value.
 * @retval #PREFERENCE_ERROR_NONE Successful
 * @retval #PREFERENCE_ERROR_INVALID_PARAMETER Invalid parameter 
 * @retval #PREFERENCE_ERROR_IO_ERROR Internal I/O Error
 *
 */
int preference_remove(const char *key);


/**
 * @brief Checks whether if the given @a key exists in the preference.
 *
 * @param [in] key The name of the key to check
 * @param [out] existing  @c true if the @a key exists in the preference, otherwise @c false
 * @return 0 on success, otherwise a negative error value.
 * @retval #PREFERENCE_ERROR_NONE Successful
 * @retval #PREFERENCE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PREFERENCE_ERROR_IO_ERROR Internal I/O Error
 */
int preference_is_existing(const char *key, bool *existing);


/**
 * @brief Removes all key-value pairs from the preference.
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #PREFERENCE_ERROR_NONE Successful
 * @retval #PREFERENCE_ERROR_IO_ERROR Internal I/O Error
 * @see	preference_remove()
 */
int preference_remove_all(void);


/**
 * @brief Registers a callback function to be invoked when value of the given key in the preference changes.
 *
 * @param [in] key The name of the key to monitor
 * @param [in] callback The callback function to register
 * @param [in] user_data The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #PREFERENCE_ERROR_NONE Successful
 * @retval #PREFERENCE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PREFERENCE_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #PREFERENCE_ERROR_NO_KEY Required key not available
 * @retval #PREFERENCE_ERROR_IO_ERROR Internal I/O Error
 * @post	preference_changed_cb() will be invoked.
 * @see	preference_unset_changed_cb()
 * @see preference_changed_cb()
 */
int preference_set_changed_cb(const char *key, preference_changed_cb callback, void *user_data);


/**
 * @brief Unregisters the callback function.
 *
 * @param [in] key The name of the key to monitor
 * @return 0 on success, otherwise a negative error value.
 * @retval #PREFERENCE_ERROR_NONE Successful
 * @retval #PREFERENCE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PREFERENCE_ERROR_IO_ERROR Internal I/O Error
 * @see	preference_set_changed_cb()
 */
int preference_unset_changed_cb(const char *key);


/**
 * @brief Retrieves all key-value pairs in the preference by invoking the callback function.
 *
 * @param [in] callback The callback function to get key value once for each key-value pair in the preference
 * @param [in] user_data The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #PREFERENCE_ERROR_NONE Successful
 * @retval #PREFERENCE_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #PREFERENCE_ERROR_IO_ERROR Internal I/O Error
 * @post This function invokes preference_item_cb() repeatedly to get each key-value pair in the preference.
 * @see preference_item_cb()
 */
int preference_foreach_item(preference_item_cb callback, void *user_data);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_APPFW_PREFERENCE_H__ */
