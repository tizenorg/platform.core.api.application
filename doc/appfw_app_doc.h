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


#ifndef __TIZEN_APPFW_APP_DOC_H__
#define __TIZEN_APPFW_APP_DOC_H__

/**
* @ingroup CAPI_APPLICATION_FRAMEWORK
* @defgroup CAPI_APPLICATION_MODULE Application
*
* @addtogroup CAPI_APPLICATION_MODULE
*
* @brief The @ref CAPI_APPLICATION_MODULE API provides functions to manage the main event loop, the application's state change events, basic system events,
*        and get information about the application.
*
*
* @section CAPI_APPLICATION_MODULE_HEADER Required Header
*   \#include <app.h>
*
* @section CAPI_APPLICATION_MODULE_OVERVIEW Overview
*
* The Application API handles an application state change or system events and provides mechanisms that launch other applications.
*
* The Tizen native application is similar to a conventional Linux application, but has some additional features optimized for mobile
* devices, which may have constraints such as relatively small screen sizes and lack of system resources compared to a larger system.
* For example, for power management reasons, the application may wish to take  actions to reduce usage when it finds out that it had its display window
* covered over by another application's window. State change events are delivered so that this is possible.
*
* The Application API provides interfaces for the following three categories:
* - Starting or exiting the main event loop (mandatory for every Tizen native application)
* - Registering callbacks for application state change events
* - Registering callbacks for basic system events
*
* @subsection CAPI_APPLICATION_MODULE_EVENT_LOOP Event Loop
* For an application to operate successfully, it must receive events from the platform.
* For this, it needs to start the main event loop - this is mandatory for all Tizen native applications.
* The ui_app_main() function is used to start the event loop.
* Before calling this function, you should set up the #ui_app_lifecycle_callback_s structure which is passed to the function (see the following sections).
*
* @subsection CAPI_APPLICATION_MODULE_STATE_CHANGE_EVENT Registering Callbacks for Application State Change Events
*  The application state change events include the following:
* <p>
* <table>
* <tr>
*  <th> State </th>
*  <th> Description </th>
* </tr>
* <tr>
*  <td> app_create_cb() </td>
*  <td>Hook to take necessary actions before the main event loop starts.
*   Your UI generation code should be placed here so that you do not miss any events from your application UI.
* </td>
* </tr>
* <tr>
*  <td> app_pause_cb() </td>
*  <td> Hook to take necessary actions when an application becomes invisible.
*    For example, you might wish to release memory resources so other applications can use these resources.
*    It is important not to starve the application in front, which is interacting with the user.
* </td>
* </tr>
* <tr>
*  <td> app_resume_cb() </td>
*  <td> Hook to take necessary actions when an application becomes visible.
*   If anything is relinquished in app_pause_cb() but is necessary to resume
*   the application, it must be re-allocated here.
*  </td>
* </tr>
* <tr>
*  <td> app_terminate_cb() </td>
*  <td> Hook to take necessary actions when your application is terminating.
*   Your application should release all resources, especially any
*   allocations and shared resources must be freed here so other running applications can fully use these shared resources.
*  </td>
* </tr>
* <tr>
*  <td> app_control_cb() </td>
*  <td> Hook to take necessary actions when your application called by another application.
*   When the application gets launch request, this callback function is called.
*   The application can get information about what is to be performed by using App Control API from app_control handle.
*  </td>
* </tr>
* </table>
*
* For further explanation of application state changes, see the @ref CAPI_APPLICATION_MODULE_STATES_TRANSITIONS section.
*
* @subsection CAPI_APPLICATION_MODULE_SYSTEM_EVENT Registering Callbacks for System Events
*  The system event callbacks can be registered using ui_app_add_event_handler(). Multiple system event callbacks can be registered.
*  The basic system event includes the following:
* <p>
* <table>
* <tr>
*  <th> System event types </th>
*  <th> Description </th>
* </tr>
* <tr>
*  <td> APP_EVENT_LOW_MEMORY </td>
*  <td> The system memory is running low.
* Recommended actions are as follows:
*  - Action  1. Save data in main memory to persistent memory/storage
*    to avoid data loss in case the Tizen platform Low Memory Killer kills your application to get more free memory.
*    The explanation of how Low Memory Killer behaves is here(link), but basically if the low memory notice does not lead to enough
*    memory being freed, something will be killed. Release cached data in main memory to secure more free memory.
* </td>
* </tr>
* <tr>
*  <td> APP_EVENT_LOW_BATTERY </td>
*  <td> The battery power is running low.
*   Recommended actions are as follows:
* - Action 1. Save data in main memory to persistent memory/storage to avoid sudden loss of data in case power goes off completely.
* - Action 2. Stop heavy CPU consumption or power consumption activities to save remaining power.
* </td>
* </tr>
* <tr>
*  <td> APP_EVENT_DEVICE_ORIENT </td>
*  <td> The orientation of device is changed.
*  Recommended action is as follows: change display orientation to match display orientation
* </td>
* </tr>
* <tr>
*  <td> APP_EVANG_LANGUAGE_CHANGED </td>
*  <td> The language setting is changed.
*   Recommended action: refresh the display into the new language
* </td>
* </tr>
* <tr>
*  <td> APP_EVENT_REGION_FORMAT_CHANGED </td>
*  <td> The region format setting changed.
*  Region change means a different time zone; the application UI may need to update the time to reflect the time zone change.
* </td>
* </tr>
* <tr>
*  <td> APP_EVENT_SUSPENDED_STATE_CHANGED </td>
*  <td> The suspended state is changed.
*   The application will enter to the suspended state, or exited from the state. Do the necessary action before or after the suspended; the application shall not respond to requests of the other modules.
*  </td>
* </tr>
* </table>
*
* @subsection CAPI_APPLICATION_MODULE_STATES_TRANSITIONS The Application States and Transitions
* The Tizen native application can be in one of the several different states. Typically, the application is launched by the user from the Launcher, or by
* another application. As the application is starting, app_create_cb() is executed, and then the main event loop starts. After executing event loop, app_control_cb() and app_resume_cb() will be invoked.
* The application now normally becomes the front-most window, with focus. When the application loses the front-most/focus status, the app_pause_cb() callback is invoked. There
* are many scenarios for your application to go into the pause state, which means your application is not terminated, but is running in the background:
* - A new application is launched from the request of your application
* - The user requests to go to the home screen
* - A system event occurs and it causes a resident application with higher priority (e.g. a phone call is received) to become active, and hides
*   your application temporarily
* - An alarm went off for another application so it now becomes the top-most window and hides your application
*
* Since Tizen 2.4, the application on the background goes in to a suspended state. In the suspended state, the application process is executed with limited CPU resources. In other words, the platform does
* not allow the running of the background applications.
*
*  When your application becomes visible again, the app_resume_cb() callback is invoked. Some possible scenarios for your application to become visible are:
* - Another application requests your application to run (perhaps the Task Navigator which shows all running applications and lets
*   the user select any app to run)
* - All applications on top of your application in the window stack finish
* - An alarm is going off for your application so it becomes the top-most window and hides other applications
*
* When your application starts exiting, the app_pause_cb() callback is invoked if the application is visible, and then the app_terminate_cb() callback is invoked. Possible scenarios to start termination of your application are:
* - Your application itself requests to exit by calling ui_app_exit() to terminate the event loop
* - The Low Memory Killer is killing your application in a low memory situation.
*
* Note that a service application doesn't have UI, so the service application doesn't have Paused state.
*
* Application state changes are managed by the underlying framework.
* Refer to the following state diagram to see the possible transitions:
* @image html capi_appfw_application_states.png "Application states"
* <p>
* <table>
* <tr>
*  <th> State </th>
*  <th> Description </th>
* </tr>
* <tr>
*  <td> READY </td>
*  <td> The application is launched. </td>
* </tr>
* <tr>
*  <td> CREATED </td>
*  <td> The application starts the main loop. </td>
* </tr>
* <tr>
*  <td> RUNNING </td>
*  <td> The application is running and visible to the user. </td>
* </tr>
* <tr>
*  <td> PAUSED </td>
*  <td> The application is running but invisible to the user. </td>
* </tr>
* <tr>
*  <td> TERMINATED </td>
*  <td> The application is terminated </td>
* </tr>
* </table>
*
* The Application API defines five states with their corresponding transition handlers.
* The state transition is notified through the state transition callback function,
* whether the application is created, is running, is paused, is resumed, or is terminated.
* And the application has to perform a specific task that is appropriate to those state changes.
*
* @image html capi_appfw_application_lifecycle.png "General application lifecycle"
*
* @subsection CAPI_APPLICATION_MODULE_HOW_TO_START  How to start the Tizen native application
* An application can be launched by the user from the Launcher or by another application.
*
* Regardless of how an application is launched, the Application Framework starts an application by creating a new process and calls
* the entry point of the application. Like a conventional Linux application, the main function of its application is the entry point.
* In the Tizen application, the main task is to hand over control to the Application Framework by calling the ui_app_main() function.
* @code
* bool app_create(void *user_data)
* {
*     // Hook to take necessary actions before the main event loop starts
*     // Initialize UI resources and application's data
*     // If this function returns @c true, the main loop of the application starts
*     // If this function returns @c false, the application is terminated
*     return true;
* }
*
* void app_control(app_control_h app_control, void *user_data)
* {
*     // Handle the launch request
* }
*
* void app_pause(void *user_data)
* {
*     // Take necessary actions when the application becomes invisible
* }
*
* void app_resume(void *user_data)
* {
*     // Take necessary actions when the application becomes visible.
* }
*
* void app_terminate(void *user_data)
* {
*     // Release all resources
* }
*
* int main(int argc, char *argv[])
* {
*     struct appdata ad;
*
*     ui_app_lifecycle_callback_s event_callback = {0,};
*
*     event_callback.create = app_create;
*     event_callback.terminate = app_terminate;
*     event_callback.pause = app_pause;
*     event_callback.resume = app_resume;
*     event_callback.app_control = app_control;
*
*     memset(&ad, 0x0, sizeof(struct appdata));
*
*     return ui_app_main(argc, argv, &event_callback, &ad);
* }
* @endcode
*
* The ui_app_main() function initializes the application and then starts the main loop.
*
* The ui_app_main() function takes four parameters and uses them to initialize the application.
* The @a argc and @a argv parameters contain the values from the Application Framework, so that you should never have to change the values.
* The third parameter is a state transition handler that is responsible for managing the state
* transitions that the application goes through while it is running. The fourth parameter is the application data to be passed to each state handler.
*
* When ui_app_main() is first invoked, the application moves from the ready state to the created state.
* The application has to initialize itself.
*
* During this transition, the Application Framework calls the application's app_create_cb() state transition callback function just
* before the application enters the main loop. Within the registered app_create_cb() callback function, you should initialize the
* application's resources and create the main window.
*
* If the app_create_cb() callback function returns @c false, the application moves to the terminated state.\n
* If the app_create_cb() callback function returns @c true, the application enters the main loop.
*
* @subsection CAPI_APPLICATION_MODULE_LAUNCH_OPTIONS  Handling the launch options
* The Application Framework calls the application's app_control_cb() callback function just after the application enters the main loop.
* This callback function is passed to the app_control containing the reason due to which the application is launched.
* For example, the application might be launched to open a file to handle the request that has been sent by another application.
* In all of these cases, the application is responsible for checking the contents of the app_control and responding appropriately. The contents
* of the app_control can be empty in situations where the application is launched by the user from the Launcher.
*
* @code
* static void app_control(app_control_h app_control, void *user_data)
* {
*     struct appdata *ad = (struct appdata *)user_data;
*     char *operation;
*     char *uri;
*     char *mime_type;
*
*     app_control_get_operation(app_control, operation);
*
*     if (!strcmp(operation, SERVICE_OPERATION_VIEW))
*     {
*         app_control_get_uri(app_control, &uri);
*         app_control_get_mime(app_control, &mime_type);
*
*         if (uri && !strcmp(mime_type, "image/jpg"))
*         {
*             display_image_file(ad, uri); // display a specific image file
*         }
*     }
*
*     if (ad->win)
*         elm_win_activate(ad->win);
*
* }
* @endcode
*
*/

#endif /* __TIZEN_APPFW_APP_DOC_H__ */
