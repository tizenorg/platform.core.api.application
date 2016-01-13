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


#ifndef __TIZEN_APPFW_PREFERENCE_DOC_H__
#define __TIZEN_APPFW_PREFERENCE_DOC_H__

 /**
 * @ingroup CAPI_APPLICATION_MODULE
 * @defgroup CAPI_PREFERENCE_MODULE Preference
 * @brief The @ref CAPI_PREFERENCE_MODULE API provides functions to store and retrieve small pieces of data, used for application preferences.
 *
 * @addtogroup CAPI_PREFERENCE_MODULE
 *
 * @section CAPI_PREFERENCE_MODULE_HEADER Required Header
 *   \#include <app_preference.h>
 *
 * @section CAPI_PREFERENCE_MODULE_OVERVIEW Overview
 *
 * The Preference API provides a mechanism that saves data items in the form of key/value pairs for this application, and later retrieves them.
 * A typical usecase would be for an application preference screen where the user can pick some values for some options. The Preference API
 * has pairs of functions, one to set such a pair, another to retrieve the stored value given in the key. Keys are always text strings, but
 * there are functions for each of the possible value types: integer, double, string, and boolean. There is also a function to clear all of the preferences.
 *
 * The @ref CAPI_PREFERENCE_MODULE API provides a way to register a callback to get notified when a value for a particular key changes. It is useful to know
 * when the display should be updated or some behavior is altered as a result.
 *
 * There is an iterator function which steps through all the data pairs, invoking a callback for each one.
 */

#endif /* __TIZEN_APPFW_PREFERENCE_DOC_H__ */
