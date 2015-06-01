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


#ifndef __TIZEN_APPFW_I18N_H__
#define __TIZEN_APPFW_I18N_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C"
{
#endif

 /**
 * @addtogroup CAPI_I18N_MODULE
 * @{
 */

/**
 * @brief	Marks a string for translation, gets replaced with the translated string at runtime.
 * @param [in] msg The string to be translated.
 */
#define _(msg) i18n_get_text(msg)

/**
 * @brief Gets the localized translation for the specified string.
 *
 * @details If a translation was not found in the localization file(.po file), @a message is returned.
 *
 * @remarks Do not free returned value
 *
 * @param [in] message The string to be translated
 * @return  The localized translation for the given @a message on success, otherwise the given @a message.
 */
char* i18n_get_text(const char *message);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_APPFW_I18N_H__ */
