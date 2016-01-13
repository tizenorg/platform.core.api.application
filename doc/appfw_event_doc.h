/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
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


#ifndef __TIZEN_APPFW_EVENT_DOC_H__
#define __TIZEN_APPFW_EVENT_DOC_H__

 /**
 * @ingroup CAPI_APPLICATION_MODULE
 * @defgroup CAPI_EVENT_MODULE Event
 * @brief The @ref CAPI_EVENT_MODULE API provides functions to serve publication and subscription of event.
 *
 * @section CAPI_EVENT_MODULE_HEADER Required Header
 *   \#include <app_event.h>
 *
 * @section CAPI_EVENT_MODULE_OVERVIEW Overview
 *
 * The @ref CAPI_EVENT_MODULE API provides functions to broadcast user-defined event.
 *     - event_publish_app_event()
 *     - event_publish_trusted_app_event()
 * If you want to send an event to trusted application, you can use event_publish_trusted_app_event() API. Only applications which have same signature with sender application can receive the event.
 *
 * The @ref CAPI_EVENT_MODULE API provides functions to add and remove the event handler for event subscription.
 * The event handler's function will be called when the interested event occurs.
 * Adding multiple event handlers to one event is possible.
 *
 * There are two types event.
 *     - User-Event is user-defined event.
 *     - System-Event is pre-defined platform event. See definitions for system event. NOTE: You can't receive the system-event which is not supported on the target device, even if you register event handler for that event. Some system-events require the privilege. (Refer to @ref CAPI_EVENT_MODULE_PRIVILEGED_SYSTEM_EVENT)
 *
 * The type of event data is bundle type.
 *     - For System-Event, see definitions for key and value of system event.
 *
 * @subsection CAPI_EVENT_MODULE_EVENT_FORMAT The name-format of User-Event.
 * The format of User-Event's name MUST be "event.{sender's appid}.{user-defined name}", unless the API calls using event_name will be failed.
 * The {user-defined name} have some restrictions.
 *     - Must have length of Min 1 byte and Max 127 bytes.
 *     - Must only contain the characters("[A~Z][a~z][0~9]_") and not begin with a digit.
 * For example,
 *     "event.org.tizen.testapp.user_event_1".
 *
 * @subsection CAPI_EVENT_MODULE_LAUNCH_ON_EVENT Launch-On-Event (Service Application only)
 * Service application can be launched by event triggering.
 * For Launch-On-Event,
 *     - The APP_CONTROL_OPERATION_LAUNCH_ON_EVENT must be defined in the manifest file.
 *     - The uri name represents event name and the format is "event://{event name"}".
 *         For uri example,
 *             "event://tizen.system.event.battery_charger_status". (System-Event)
 *     - The APP_CONTROL_OPERATION_LAUNCH_ON_EVENT can not be requested via app_control_send_launch_request().
 *
 * How to know the triggered event after launch by event triggering.
 *     - You can get the event name and data in the first called app_control_cb which is called after 'Created' of application states.
 * @code
 * static void
 * app_control(app_control_h app_control, void *data)
 * {
 *     //check "launch on event"
 *     int ret = 0;
 *     const char *event_uri = "event://tizen.system.event.battery_charger_status";
 *     char *operation = NULL;
 *     char *uri = NULL;
 *     char *event_value = NULL;
 *
 *     ret = app_control_get_operation(app_control, &operation);
 *     if (ret == APP_CONTROL_ERROR_NONE && operation &&
 *             strcmp(operation, APP_CONTROL_OPERATION_LAUNCH_ON_EVENT) == 0) {
 *         ret = app_control_get_uri(app_control, &uri);
 *         if (ret == APP_CONTROL_ERROR_NONE && uri) {
 *             if (strncmp(uri, event_uri, strlen(event_uri) + 1) == 0) {
 *                 ret = app_control_get_extra_data(app_control, "battery_charger_status", &event_value);
 *                 if (ret == APP_CONTROL_ERROR_NONE && event_value) {
 *                     free(event_value);
 *                 }
 *             }
 *             free(uri);
 *         }
 *         free(operation);
 *     }
 * }
 * @endcode
 *     - Use event_add_event_handler() API in the first called app_control_cb for further subscription of that event.
 *
 * NOTE : Only several of System-Events support Launch-On-Event.
 * The System-Events and their Conditions which support Launch-On-Event are listed in the table below.
 * <table>
 * <tr>
 * <th>Name</th>
 * <th>Condition</th>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_BATTERY_CHARGER_STATUS</td>
 * <td>When Charger CONNECTED</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_USB_STATUS</td>
 * <td>When Usb CONNECTED</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_EARJACK_STATUS</td>
 * <td>When Earjack CONNECTED</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_INCOMMING_MSG</td>
 * <td>When MSG_TYPE & MSG_ID exist</td>
 * </tr>
 * </table>
 *
 * @subsection CAPI_EVENT_MODULE_PRIVILEGED_SYSTEM_EVENT Privileged System-Events.
 * There are some system-events which need the privilege for normal operation.
 * You can know these privileged system-events from the 'privilege' comment of the definition of each system-event.
 * If you try to add event handler for these events without privilege, you will get EVENT_ERROR_PERMISSION_DENIED return when you use the API for adding event handler.
 * Also, If you declare app_control operation and uri in the manifest file for these events to enable 'Launch On Event' without privilege, there will be no real launch.
 *
 * @subsection CAPI_EVENT_MODULE_SYSTEM_EVENT Supported System-Events.
 * The supported system-event's name, keys and values of each event are listed in the table below.
 * <table>
 * <tr>
 * <th>Name</th>
 * <th>Key</th>
 * <th>Value</th>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_BATTERY_CHARGER_STATUS</td>
 * <td>EVENT_KEY_BATTERY_CHARGER_STATUS</td>
 * <td>EVENT_VAL_BATTERY_CHARGER_DISCONNECTED<br/>EVENT_VAL_BATTERY_CHARGER_CONNECTED<br/>EVENT_VAL_BATTERY_CHARGER_CHARGING<br/>EVENT_VAL_BATTERY_CHARGER_DISCHARGING</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_BATTERY_LEVEL_STATUS</td>
 * <td>EVENT_KEY_BATTERY_LEVEL_STATUS</td>
 * <td>EVENT_VAL_BATTERY_LEVEL_EMPTY<br/>EVENT_VAL_BATTERY_LEVEL_CRITICAL<br/>EVENT_VAL_BATTERY_LEVEL_LOW<br/>EVENT_VAL_BATTERY_LEVEL_HIGH<br/>EVENT_VAL_BATTERY_LEVEL_FULL</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_USB_STATUS</td>
 * <td>EVENT_KEY_USB_STATUS</td>
 * <td>EVENT_VAL_USB_DISCONNECTED<br/>EVENT_VAL_USB_CONNECTED<br/>EVENT_VAL_USB_AVAILABLE</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_EARJACK_STATUS</td>
 * <td>EVENT_KEY_EARJACK_STATUS</td>
 * <td>EVENT_VAL_EARJACK_DISCONNECTED<br/>EVENT_VAL_EARJACK_CONNECTED</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_DISPLAY_STATE</td>
 * <td>EVENT_KEY_DISPLAY_STATE</td>
 * <td>EVENT_VAL_DISPLAY_NORMAL<br/>EVENT_VAL_DISPLAY_DIM<br/>EVENT_VAL_DISPLAY_OFF</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_BOOT_COMPLETED</td>
 * <td>N/A</td>
 * <td>N/A</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_SYSTEM_SHUTDOWN</td>
 * <td>N/A</td>
 * <td>N/A</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_LOW_MEMORY</td>
 * <td>EVENT_KEY_LOW_MEMORY</td>
 * <td>EVENT_VAL_MEMORY_NORMAL<br/>EVENT_VAL_MEMORY_SOFT_WARNING<br/>EVENT_VAL_MEMORY_HARD_WARNING</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_WIFI_STATE</td>
 * <td>EVENT_KEY_WIFI_STATE</td>
 * <td>EVENT_VAL_WIFI_OFF<br/>EVENT_VAL_WIFI_ON<br/>EVENT_VAL_WIFI_CONNECTED</td>
 * </tr>
 * <tr>
 * <td rowspan="3">SYSTEM_EVENT_BT_STATE</td>
 * <td>EVENT_KEY_BT_STATE</td>
 * <td>EVENT_VAL_BT_OFF<br/>EVENT_VAL_BT_ON</td>
 * </tr>
 * <tr>
 * <td>EVENT_KEY_BT_LE_STATE</td>
 * <td>EVENT_VAL_BT_LE_OFF<br/>EVENT_VAL_BT_LE_ON</td>
 * </tr>
 * <tr>
 * <td>EVENT_KEY_BT_TRANSFERING_STATE</td>
 * <td>EVENT_VAL_BT_NON_TRANSFERING<br/>EVENT_VAL_BT_TRANSFERING</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_LOCATION_ENABLE_STATE</td>
 * <td>EVENT_KEY_LOCATION_ENABLE_STATE</td>
 * <td>EVENT_VAL_LOCATION_DISABLED<br/>EVENT_VAL_LOCATION_ENABLED</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_GPS_ENABLE_STATE</td>
 * <td>EVENT_KEY_GPS_ENABLE_STATE</td>
 * <td>EVENT_VAL_GPS_DISABLED<br/>EVENT_VAL_GPS_ENABLED</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_NPS_ENABLE_STATE</td>
 * <td>EVENT_KEY_NPS_ENABLE_STATE</td>
 * <td>EVENT_VAL_NPS_DISABLED<br/>EVENT_VAL_NPS_ENABLED</td>
 * </tr>
 * <tr>
 * <td rowspan="2">SYSTEM_EVENT_INCOMMING_MSG</td>
 * <td>EVENT_KEY_MSG_TYPE</td>
 * <td>EVENT_VAL_SMS<br/>EVENT_VAL_PUSH<br/>EVENT_VAL_CB</td>
 * </tr>
 * <tr>
 * <td>EVENT_KEY_MSG_ID</td>
 * <td>string of unsigned int type value.</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_TIME_CHANGED</td>
 * <td>N/A</td>
 * <td>N/A</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_TIME_ZONE</td>
 * <td>EVENT_KEY_TIME_ZONE</td>
 * <td>The value of this key is timezone value of tz database, for example,<br/> "Asia/Seoul", "America/New_York",<br/>refer to the Time Zone Database of IANA.</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_HOUR_FORMAT</td>
 * <td>EVENT_KEY_HOUR_FORMAT</td>
 * <td>EVENT_VAL_HOURFORMAT_12<br/>EVENT_VAL_HOURFORMAT_24</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_LANGUAGE_SET</td>
 * <td>EVENT_KEY_LANGUAGE_SET</td>
 * <td>The value of this key is full name of locale, for example,<br/> "ko_KR.UTF8" : in case of Korean language<br/> "en_US.UTF8" : in case of USA language,<br/>refer to linux locale info.</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_REGION_FORMAT</td>
 * <td>EVENT_KEY_REGION_FORMAT</td>
 * <td>The value of this key is full name of locale, for example,<br/> "ko_KR.UTF8" : in case of Korean region format<br/> "en_US.UTF8" : in case of USA region format,<br/>refer to linux locale info.</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_SILENT_MODE</td>
 * <td>EVENT_KEY_SILENT_MODE</td>
 * <td>EVENT_VAL_SILENTMODE_ON<br/>EVENT_VAL_SILENTMODE_OFF</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_VIBRATION_STATE</td>
 * <td>EVENT_KEY_VIBRATION_STATE</td>
 * <td>EVENT_VAL_VIBRATION_ON<br/>EVENT_VAL_VIBRATION_OFF</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_SCREEN_AUTOROTATE_STATE</td>
 * <td>EVENT_KEY_SCREEN_AUTOROTATE_STATE</td>
 * <td>EVENT_VAL_SCREEN_AUTOROTATE_ON<br/>EVENT_VAL_SCREEN_AUTOROTATE_OFF</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_MOBILE_DATA_STATE</td>
 * <td>EVENT_KEY_MOBILE_DATA_STATE</td>
 * <td>EVENT_VAL_MOBILE_DATA_OFF<br/>EVENT_VAL_MOBILE_DATA_ON</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_DATA_ROAMING_STATE</td>
 * <td>EVENT_KEY_DATA_ROAMING_STATE</td>
 * <td>EVENT_VAL_DATA_ROAMING_OFF<br/>EVENT_VAL_DATA_ROAMING_ON</td>
 * </tr>
 * <tr>
 * <td>SYSTEM_EVENT_FONT_SET</td>
 * <td>EVENT_KEY_FONT_SET</td>
 * <td>The value of this key is font name of string type by font-config.</td>
 * </tr>
 * </table>
 *
 */

#endif /* __TIZEN_APPFW_EVENT_DOC_H__ */
