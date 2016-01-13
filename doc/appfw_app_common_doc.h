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


#ifndef __TIZEN_APPFW_APP_COMMON_DOC_H__
#define __TIZEN_APPFW_APP_COMMON_DOC_H__

 /**
 * @ingroup CAPI_APPLICATION_MODULE
 * @defgroup CAPI_APP_COMMON_MODULE App Common
 * @brief The @ref CAPI_APP_COMMON_MODULE API provides functions to getting information about the application.
 *
 * @addtogroup CAPI_APP_COMMON_MODULE
 *
 * @section CAPI_APP_COMMON_MODULE_HEADER Required Header
 *   \#include <app_common.h>
 *
 * @section CAPI_APP_COMMON_MODULE_OVERVIEW Overview
 *
 * The App common API provides common apis that can be used at UI application and Service application.
 * This API provides interfaces for getting information about the application.
 *
 * @subsection CAPI_APPLICATION_MODULE_PACKAGE Application Package
 * The Tizen native application consists of structured directories to manage the application executable file, library files, resource files, and data.
 * When you build the application, the Tizen SDK packages those as an application package for distribution.
 *
 * @image html capi_appfw_application_package.png "Application package"
 *
 * <table>
 * <tr>
 * <th>Directories</th>
 * <th>Description</th>
 * </tr>
 * <tr>
 * <td>&lt;package id&gt;</td>
 * <td>The fully qualified name of an application (such as org.tizen.calculator).</td>
 * </tr>
 * <tr>
 * <td>bin</td>
 * <td>The executable file of the application.</td>
 * </tr>
 * <tr>
 * <td>lib</td>
 * <td>The application library files</td>
 * </tr>
 * <tr>
 * <td>res</td>
 * <td>The root directory in which all resource files are located.\n
 * The application cannot write and modify any resource files</td>
 * </tr>
 * <tr>
 * <td>data</td>
 * <td>The root directory in which an application's private data is located.\n
 * The application can read and write its own data files in the application's data directory.</td>
 * </tr>
 * <tr>
 * <td>shared/</td>
 * <td>The shared directory for sharing with other applications.</td>
 * </tr>
 * </table>
 *
 * @subsection CAPI_APPLICATION_MODULE_GET_INFORMATION Getting Information About the Application
 * The API provides functions for obtaining an application's package name and absolute path to specified resources
 * like Image, Sound, Video, UI layout (EDJ), and so on.
 * It also provides functions to :
 * - Get the current orientation of the device
 * - Get the Internal/External root folders which are shared among all applications
 *
 */

#endif /* __TIZEN_APPFW_APP_COMMON_DOC_H__ */
