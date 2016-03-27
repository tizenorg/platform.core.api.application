/*
 * Copyright (c) 2011 - 2016 Samsung Electronics Co., Ltd All Rights Reserved
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


#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <bundle.h>
#include <bundle_internal.h>
#include <aul.h>
#include <appsvc.h>
#include <aul_svc.h>
#include <dlog.h>

#include <app_control.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "CAPI_APPFW_APP_CONTROL"

#ifndef TIZEN_PATH_MAX
#define TIZEN_PATH_MAX 1024
#endif

#define BUNDLE_KEY_PREFIX_AUL "__AUL_"
#define BUNDLE_KEY_PREFIX_SERVICE "__APP_SVC_"

#define BUNDLE_KEY_OPERATION	"__APP_SVC_OP_TYPE__"
#define BUNDLE_KEY_URI		"__APP_SVC_URI__"
#define BUNDLE_KEY_MIME		"__APP_SVC_MIME_TYPE__"
#define BUNDLE_KEY_DATA		"__APP_SVC_DATA__"
#define BUNDLE_KEY_PACKAGE	"__APP_SVC_PKG_NAME__"
#define BUNDLE_KEY_WINDOW	"__APP_SVC_K_WIN_ID__"
#define BUNDLE_KEY_CATEGORY	"__APP_SVC_CATEGORY__"

#define LAUNCH_MODE_SIZE 8
#define LAUNCH_MODE_SINGLE "single"
#define LAUNCH_MODE_GROUP "group"

typedef enum {
	APP_CONTROL_TYPE_REQUEST,
	APP_CONTROL_TYPE_EVENT,
	APP_CONTROL_TYPE_REPLY,
} app_control_type_e;

struct app_control_s {
	int id;
	app_control_type_e type;
	bundle *data;
	int launch_pid;
};

typedef struct app_control_request_context_s {
	app_control_h app_control;
	app_control_reply_cb reply_cb;
	void *user_data;
} *app_control_request_context_h;

extern int appsvc_allow_transient_app(bundle *b, unsigned int id);
extern int appsvc_request_transient_app(bundle *b, unsigned int callee_id, appsvc_host_res_fn cbfunc, void *data);
static int app_control_create_reply(bundle *data, struct app_control_s **app_control);

static const char* app_control_error_to_string(app_control_error_e error)
{
	switch (error) {
	case APP_CONTROL_ERROR_NONE:
		return "NONE";
	case APP_CONTROL_ERROR_INVALID_PARAMETER:
		return "INVALID_PARAMETER";
	case APP_CONTROL_ERROR_OUT_OF_MEMORY:
		return "OUT_OF_MEMORY";
	case APP_CONTROL_ERROR_APP_NOT_FOUND:
		return "APP_NOT_FOUND";
	case APP_CONTROL_ERROR_KEY_NOT_FOUND:
		return "KEY_NOT_FOUND";
	case APP_CONTROL_ERROR_KEY_REJECTED:
		return "KEY_REJECTED";
	case APP_CONTROL_ERROR_INVALID_DATA_TYPE:
		return "INVALID_DATA_TYPE";
	case APP_CONTROL_ERROR_LAUNCH_REJECTED:
		return "LAUNCH_REJECTED";
	case APP_CONTROL_ERROR_PERMISSION_DENIED:
		return "PERMISSION_DENIED";
	case APP_CONTROL_ERROR_LAUNCH_FAILED:
		return "LAUNCH_FAILED";
	case APP_CONTROL_ERROR_TIMED_OUT:
		return "TIMED_OUT";
	case APP_CONTROL_ERROR_IO_ERROR:
		return "IO ERROR";
	default:
		return "UNKNOWN";
	}
}

int app_control_error(app_control_error_e error, const char* function, const char *description)
{
	if (description)
		LOGE("[%s] %s(0x%08x) : %s", function, app_control_error_to_string(error), error, description);
	else {
		if (error == APP_CONTROL_ERROR_KEY_NOT_FOUND)
			LOGW("[%s] %s(0x%08x)", function, app_control_error_to_string(error), error);
		else
			LOGE("[%s] %s(0x%08x)", function, app_control_error_to_string(error), error);
	}

	return error;
}

static int app_control_validate_extra_data(const char *data)
{
	if (data == NULL || data[0] == '\0')
		return APP_CONTROL_ERROR_INVALID_PARAMETER;

	return APP_CONTROL_ERROR_NONE;
}

static int app_control_validate(app_control_h app_control)
{
	if (app_control == NULL || app_control->data == NULL)
		return APP_CONTROL_ERROR_INVALID_PARAMETER;

	return APP_CONTROL_ERROR_NONE;
}

static int app_control_new_id()
{
	static int sid = 0;
	return sid++;
}

int app_control_validate_internal_key(const char *key)
{
	if (strncmp(BUNDLE_KEY_PREFIX_AUL, key, strlen(BUNDLE_KEY_PREFIX_AUL)) == 0)
		return -1;

	if (strncmp(BUNDLE_KEY_PREFIX_SERVICE, key, strlen(BUNDLE_KEY_PREFIX_SERVICE)) == 0)
		return -1;

	return 0;
}

static void app_control_request_result_broker(bundle *appsvc_bundle, int appsvc_request_code, appsvc_result_val appsvc_result, void *appsvc_data)
{
	app_control_request_context_h request_context;
	app_control_h request;
	app_control_h reply = NULL;
	app_control_result_e result;
	void *user_data;
	app_control_reply_cb reply_cb;

	if (appsvc_data == NULL) {
		app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid app_control reply");
		return;
	}

	if (app_control_create_reply(appsvc_bundle, &reply) != 0) {
		app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "failed to create app_control reply");
		return;
	}

	request_context = appsvc_data;
	request = request_context->app_control;

	switch (appsvc_result) {
	case APPSVC_RES_OK:
		result = APP_CONTROL_RESULT_SUCCEEDED;
		break;
	case APPSVC_RES_NOT_OK:
		result = APP_CONTROL_RESULT_FAILED;
		break;
	case APPSVC_RES_CANCEL:
		result = APP_CONTROL_RESULT_CANCELED;
		break;
	default:
		result = APP_CONTROL_RESULT_CANCELED;
		break;
	}

	user_data = request_context->user_data;
	reply_cb = request_context->reply_cb;

	if (reply_cb != NULL)
		reply_cb(request, reply, result, user_data);
	else
		app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid callback ");

	app_control_destroy(reply);

	if (request_context->app_control != NULL)
		app_control_destroy(request_context->app_control);

	free(request_context);
}

int app_control_create_request(bundle *data, app_control_h *app_control)
{
	struct app_control_s *app_control_request;

	if (app_control == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	app_control_request = malloc(sizeof(struct app_control_s));
	if (app_control_request == NULL)
		return app_control_error(APP_CONTROL_ERROR_OUT_OF_MEMORY, __FUNCTION__, "failed to create a app_control handle");

	app_control_request->type = APP_CONTROL_TYPE_REQUEST;

	if (data != NULL)
		app_control_request->data = bundle_dup(data);
	else
		app_control_request->data = bundle_create();

	if (app_control_request->data == NULL) {
		free(app_control_request);
		return app_control_error(APP_CONTROL_ERROR_OUT_OF_MEMORY, __FUNCTION__, "failed to create a bundle");
	}

	app_control_request->id = app_control_new_id();
	app_control_request->launch_pid = -1;

	*app_control = app_control_request;

	return APP_CONTROL_ERROR_NONE;
}

int app_control_create(app_control_h *app_control)
{
	return app_control_create_request(NULL, app_control);
}

int app_control_create_event(bundle *data, struct app_control_s **app_control)
{
	struct app_control_s *app_control_event;

	const char *operation;

	if (data == NULL || app_control == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	app_control_event = malloc(sizeof(struct app_control_s));
	if (app_control_event == NULL)
		return app_control_error(APP_CONTROL_ERROR_OUT_OF_MEMORY, __FUNCTION__, "failed to create a app_control handle");

	app_control_event->type = APP_CONTROL_TYPE_EVENT;
	app_control_event->data = bundle_dup(data);
	app_control_event->id = app_control_new_id();

	operation = appsvc_get_operation(app_control_event->data);
	if (operation == NULL)
		appsvc_set_operation(app_control_event->data, APP_CONTROL_OPERATION_DEFAULT);

	*app_control = app_control_event;

	return APP_CONTROL_ERROR_NONE;
}

static int app_control_create_reply(bundle *data, struct app_control_s **app_control)
{
	struct app_control_s *app_control_reply;

	if (data == NULL || app_control == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	app_control_reply = malloc(sizeof(struct app_control_s));
	if (app_control_reply == NULL)
		return app_control_error(APP_CONTROL_ERROR_OUT_OF_MEMORY, __FUNCTION__, "failed to create a app_control handle");

	app_control_reply->type = APP_CONTROL_TYPE_REPLY;
	app_control_reply->data = bundle_dup(data);
	app_control_reply->id = app_control_new_id();

	*app_control = app_control_reply;

	return APP_CONTROL_ERROR_NONE;
}

int app_control_destroy(app_control_h app_control)
{
	if (app_control_validate(app_control))
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (app_control->type == APP_CONTROL_TYPE_REQUEST && app_control->launch_pid > 0
			&& bundle_get_val(app_control->data, AUL_SVC_K_LAUNCH_RESULT_APP_STARTED) == NULL)
		aul_remove_caller_cb(app_control->launch_pid, app_control);

	bundle_free(app_control->data);
	app_control->data = NULL;
	free(app_control);

	return APP_CONTROL_ERROR_NONE;
}

int app_control_to_bundle(app_control_h app_control, bundle **data)
{
	if (app_control_validate(app_control) || data == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	*data = app_control->data;

	return APP_CONTROL_ERROR_NONE;
}

int app_control_set_operation(app_control_h app_control, const char *operation)
{
	if (app_control_validate(app_control))
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (operation != NULL) {
		if (appsvc_set_operation(app_control->data, operation) != 0)
			return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid operation");
	} else {
		bundle_del(app_control->data, BUNDLE_KEY_OPERATION);
	}

	return APP_CONTROL_ERROR_NONE;
}

int app_control_get_operation(app_control_h app_control, char **operation)
{
	const char *operation_value;

	if (app_control_validate(app_control) || operation == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	operation_value = appsvc_get_operation(app_control->data);
	if (operation_value != NULL)
		*operation = strdup(operation_value);
	else
		*operation = NULL;

	return APP_CONTROL_ERROR_NONE;
}

int app_control_set_uri(app_control_h app_control, const char *uri)
{
	if (app_control_validate(app_control))
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (uri != NULL) {
		if (appsvc_set_uri(app_control->data, uri) != 0)
			return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid URI");
	} else {
		bundle_del(app_control->data, BUNDLE_KEY_URI);
	}

	return APP_CONTROL_ERROR_NONE;
}

int app_control_get_uri(app_control_h app_control, char **uri)
{
	const char *uri_value;

	if (app_control_validate(app_control) || uri == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	uri_value = appsvc_get_uri(app_control->data);
	if (uri_value != NULL)
		*uri = strdup(uri_value);
	else
		*uri = NULL;

	return APP_CONTROL_ERROR_NONE;
}

int app_control_set_mime(app_control_h app_control, const char *mime)
{
	if (app_control_validate(app_control))
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (mime != NULL) {
		if (appsvc_set_mime(app_control->data, mime) != 0)
			return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid MIME type");
	} else {
		bundle_del(app_control->data, BUNDLE_KEY_MIME);
	}

	return APP_CONTROL_ERROR_NONE;
}

int app_control_get_mime(app_control_h app_control, char **mime)
{
	const char *mime_value;

	if (app_control_validate(app_control) || mime == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	mime_value = appsvc_get_mime(app_control->data);
	if (mime_value != NULL)
		*mime = strdup(mime_value);
	else
		*mime = NULL;

	return APP_CONTROL_ERROR_NONE;
}

int app_control_set_category(app_control_h app_control, const char *category)
{
	if (app_control_validate(app_control))
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (category != NULL) {
		if (appsvc_set_category(app_control->data, category) != 0)
			return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid Category");
	} else {
		bundle_del(app_control->data, BUNDLE_KEY_CATEGORY);
	}

	return APP_CONTROL_ERROR_NONE;
}

int app_control_get_category(app_control_h app_control, char **category)
{
	const char *category_value;

	if (app_control_validate(app_control) || category == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	category_value = appsvc_get_category(app_control->data);
	if (category_value != NULL)
		*category = strdup(category_value);
	else
		*category = NULL;

	return APP_CONTROL_ERROR_NONE;
}

int app_control_set_package(app_control_h app_control, const char *package)
{
	/* TODO: this function must be deprecated */
	return app_control_set_app_id(app_control, package);
}

int app_control_get_package(app_control_h app_control, char **package)
{
	/* TODO: this function must be deprecated */
	return app_control_get_app_id(app_control, package);
}

int app_control_set_app_id(app_control_h app_control, const char *app_id)
{
	if (app_control_validate(app_control))
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (app_id != NULL) {
		if (appsvc_set_appid(app_control->data, app_id) != 0)
			return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid application ID");
	} else {
		bundle_del(app_control->data, BUNDLE_KEY_PACKAGE);
	}

	return APP_CONTROL_ERROR_NONE;
}

int app_control_get_app_id(app_control_h app_control, char **app_id)
{
	const char *app_id_value;

	if (app_control_validate(app_control) || app_id == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	app_id_value = appsvc_get_appid(app_control->data);
	if (app_id_value != NULL)
		*app_id = strdup(app_id_value);
	else
		*app_id = NULL;

	return APP_CONTROL_ERROR_NONE;
}

int app_control_set_window(app_control_h app_control, unsigned int id)
{
	if (app_control_validate(app_control))
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (id > 0) {
		if (appsvc_allow_transient_app(app_control->data, id) != 0)
			return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid id");
	} else {
		bundle_del(app_control->data, BUNDLE_KEY_WINDOW);
	}

	return APP_CONTROL_ERROR_NONE;
}

int app_control_get_window(app_control_h app_control, unsigned int *id)
{
	const char *window_id;

	if (app_control_validate(app_control) || id == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	window_id = bundle_get_val(app_control->data, BUNDLE_KEY_WINDOW);
	if (window_id != NULL)
		*id = atoi(window_id);
	else
		*id = 0;

	return APP_CONTROL_ERROR_NONE;
}

int app_control_clone(app_control_h *clone, app_control_h app_control)
{
	app_control_h app_control_clone;

	if (app_control_validate(app_control) || clone == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	app_control_clone = malloc(sizeof(struct app_control_s));
	if (app_control_clone == NULL)
		return app_control_error(APP_CONTROL_ERROR_OUT_OF_MEMORY, __FUNCTION__, "failed to create a app_control handle");

	app_control_clone->id = app_control_new_id();
	app_control_clone->type = app_control->type;
	app_control_clone->data = bundle_dup(app_control->data);

	*clone = app_control_clone;

	return APP_CONTROL_ERROR_NONE;
}

int app_control_set_launch_mode(app_control_h app_control,
		app_control_launch_mode_e mode)
{
	char launch_mode[LAUNCH_MODE_SIZE] = { 0, };

	if (app_control_validate(app_control)) {
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER,
				__FUNCTION__, NULL);
	}

	switch (mode) {
	case APP_CONTROL_LAUNCH_MODE_SINGLE:
		strncpy(launch_mode, LAUNCH_MODE_SINGLE, strlen(LAUNCH_MODE_SINGLE));
		break;
	case APP_CONTROL_LAUNCH_MODE_GROUP:
		strncpy(launch_mode, LAUNCH_MODE_GROUP, strlen(LAUNCH_MODE_GROUP));
		break;
	default:
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER,
				__FUNCTION__, "invalid mode");
	}

	return appsvc_set_launch_mode(app_control->data, launch_mode);
}

int app_control_get_launch_mode(app_control_h app_control,
		app_control_launch_mode_e *mode)
{
	const char *launch_mode;

	if (app_control_validate(app_control)) {
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER,
				__FUNCTION__, NULL);
	}

	launch_mode = appsvc_get_launch_mode(app_control->data);
	if (launch_mode == NULL) {
		*mode = APP_CONTROL_LAUNCH_MODE_SINGLE;
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER,
				__FUNCTION__, "fail to get launch_mode");
	} else {
		if (!strcmp(launch_mode, LAUNCH_MODE_SINGLE)) {
			*mode = APP_CONTROL_LAUNCH_MODE_SINGLE;
		} else if (!strcmp(launch_mode, LAUNCH_MODE_GROUP)) {
			*mode = APP_CONTROL_LAUNCH_MODE_GROUP;
		} else {
			*mode = APP_CONTROL_LAUNCH_MODE_SINGLE;
			return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER,
					__FUNCTION__, "launch_mode is not matched");
		}
	}

	return APP_CONTROL_ERROR_NONE;
}

int app_control_set_defapp(app_control_h app_control, const char *app_id)
{
	int ret;

	if (app_control_validate(app_control) || app_id == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	ret = aul_svc_set_appid(app_control->data, app_id);
	if (ret < 0)
		return app_control_error(APP_CONTROL_ERROR_IO_ERROR, __FUNCTION__, NULL);

	ret = aul_set_default_app_by_operation(app_control->data);
	if (ret < 0) {
		if (ret == AUL_R_EILLACC)
			return app_control_error(APP_CONTROL_ERROR_PERMISSION_DENIED, __FUNCTION__, NULL);
		else
			return app_control_error(APP_CONTROL_ERROR_IO_ERROR, __FUNCTION__, NULL);
	}

	return APP_CONTROL_ERROR_NONE;
}

int app_control_unset_defapp(const char *app_id)
{
	int ret;

	if (app_id == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	ret = aul_unset_default_app_by_operation(app_id);
	if (ret < 0) {
		if (ret == AUL_R_EILLACC)
			return app_control_error(APP_CONTROL_ERROR_PERMISSION_DENIED, __FUNCTION__, NULL);
		else
			return app_control_error(APP_CONTROL_ERROR_IO_ERROR, __FUNCTION__, NULL);
	}

	return APP_CONTROL_ERROR_NONE;
}

static void __update_launch_pid(int launched_pid, void *data)
{
	app_control_h app_control;

	if (data == NULL)
		return;

	app_control = data;

	app_control->launch_pid = launched_pid;
}

static void __handle_launch_result(int launched_pid, void *data)
{
	app_control_request_context_h request_context;
	app_control_h reply = NULL;
	app_control_h request;
	app_control_result_e result;
	app_control_reply_cb reply_cb;
	void *user_data;
	char callee[255] = {0, };
	int ret = 0;

	if (data == NULL)
		return;

	request_context = (app_control_request_context_h)data;

	if (app_control_create_event(request_context->app_control->data, &reply) != 0) {
		app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "failed to create app_control event");
		return;
	}

	ret = aul_app_get_appid_bypid(launched_pid, callee, sizeof(callee));
	if (ret < 0)
		LOGE("aul_app_get_appid_bypid failed: %d", launched_pid);

	app_control_set_app_id(reply, callee);
	LOGI("app control async result callback callee pid:%d", launched_pid);

	result = APP_CONTROL_RESULT_APP_STARTED;
	request = request_context->app_control;
	user_data = request_context->user_data;
	reply_cb = request_context->reply_cb;

	if (reply_cb != NULL)
		reply_cb(request, reply, result, user_data);
	else
		app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid callback ");

	app_control_destroy(reply);
}

int app_control_send_launch_request(app_control_h app_control, app_control_reply_cb callback, void *user_data)
{
	const char *operation;
	bool implicit_default_operation = false;
	int launch_pid;
	app_control_request_context_h request_context = NULL;

	if (app_control_validate(app_control))
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	operation = appsvc_get_operation(app_control->data);
	if (operation == NULL) {
		implicit_default_operation = true;
		operation = APP_CONTROL_OPERATION_DEFAULT;
	}

	if (!strcmp(operation, APP_CONTROL_OPERATION_LAUNCH_ON_EVENT))
		return app_control_error(APP_CONTROL_ERROR_LAUNCH_REJECTED, __FUNCTION__,
				"Not supported operation value");

	/* TODO: Check the privilege for call operation */

	/* operation : default */
	if (!strcmp(operation, APP_CONTROL_OPERATION_DEFAULT)) {
		const char *appid  = appsvc_get_appid(app_control->data);
		if (appid == NULL)
			return app_control_error(APP_CONTROL_ERROR_APP_NOT_FOUND, __FUNCTION__, "package must be specified if the operation is default value");
	}

	if (callback != NULL) {
		app_control_h request_clone = NULL;

		request_context = calloc(1, sizeof(struct app_control_request_context_s));
		if (request_context == NULL)
			return app_control_error(APP_CONTROL_ERROR_OUT_OF_MEMORY, __FUNCTION__, NULL);

		request_context->reply_cb = callback;

		if (app_control_clone(&request_clone, app_control) != APP_CONTROL_ERROR_NONE) {
			free(request_context);
			return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "failed to clone the app_control request handle");
		}

		request_context->app_control = request_clone;
		request_context->user_data = user_data;
	}

	if (implicit_default_operation == true)
		appsvc_set_operation(app_control->data, APP_CONTROL_OPERATION_DEFAULT);

	launch_pid = appsvc_usr_run_service(app_control->data, app_control->id, callback ? app_control_request_result_broker : NULL, request_context, getuid());
	if (implicit_default_operation == true)
		bundle_del(app_control->data, BUNDLE_KEY_OPERATION);

	if (launch_pid < 0) {
		if (launch_pid == APPSVC_RET_ENOMATCH)
			return app_control_error(APP_CONTROL_ERROR_APP_NOT_FOUND, __FUNCTION__, NULL);
		else if (launch_pid == APPSVC_RET_EILLACC)
			return app_control_error(APP_CONTROL_ERROR_PERMISSION_DENIED, __FUNCTION__, NULL);
		else if (launch_pid == APPSVC_RET_EINVAL)
			return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
		else
			return app_control_error(APP_CONTROL_ERROR_LAUNCH_REJECTED, __FUNCTION__, NULL);
	}

	app_control->launch_pid = launch_pid;
	/* app_control_enable_app_started_result_event called */
	if (bundle_get_val(app_control->data, AUL_SVC_K_LAUNCH_RESULT_APP_STARTED)) {
		char callee[255] = {0,};
		if (aul_app_get_appid_bypid(launch_pid, callee, sizeof(callee)) != AUL_R_OK)
			LOGE("aul_app_get_appid_bypid failed: %d", launch_pid);

		if (request_context && request_context->app_control)
			request_context->app_control->launch_pid = launch_pid;

		aul_add_caller_cb(launch_pid, __handle_launch_result, request_context);

		/* launched without app selector */
		if (strncmp(callee, APP_SELECTOR, strlen(APP_SELECTOR)) != 0)
			aul_invoke_caller_cb(request_context);

	} else { /* default case */
		aul_add_caller_cb(launch_pid, __update_launch_pid, app_control);
	}

	return APP_CONTROL_ERROR_NONE;
}

int app_control_send_terminate_request(app_control_h app_control)
{
	if (app_control_validate(app_control))
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (app_control->type != APP_CONTROL_TYPE_REQUEST || app_control->launch_pid < 0)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	appsvc_subapp_terminate_request_pid(app_control->launch_pid);

	return APP_CONTROL_ERROR_NONE;
}

static bool app_control_copy_reply_data_cb(app_control_h app_control, const char *key, void *user_data)
{
	bundle *reply_data = user_data;
	char *value = NULL;
	char **value_array = NULL;
	int value_array_length = 0;
	int value_array_index = 0;

	if (reply_data == NULL) {
		app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
		return false;
	}

	if (appsvc_data_is_array(app_control->data, key)) {
		app_control_get_extra_data_array(app_control, key, &value_array, &value_array_length);
		appsvc_add_data_array(reply_data, key, (const char**)value_array, value_array_length);

		for (value_array_index = 0; value_array_index < value_array_length; value_array_index++)
			free(value_array[value_array_index]);

		free(value_array);
	} else {
		app_control_get_extra_data(app_control, key, &value);
		appsvc_add_data(reply_data, key, value);
		free(value);
	}

	return true;
}

int app_control_reply_to_launch_request(app_control_h reply, app_control_h request, app_control_result_e result)
{
	bundle *reply_data;
	int appsvc_result;
	int ret = 0;

	if (app_control_validate(reply) || app_control_validate(request))
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (result == APP_CONTROL_RESULT_APP_STARTED)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "APP_CONTROL_RESULT_APP_STARTED is not allowed to use");

	if (appsvc_create_result_bundle(request->data, &reply_data) != 0)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "failed to create a result bundle");

	app_control_foreach_extra_data(reply, app_control_copy_reply_data_cb, reply_data);

	switch (result) {
	case APP_CONTROL_RESULT_SUCCEEDED:
		appsvc_result = APPSVC_RES_OK;
		break;
	case APP_CONTROL_RESULT_FAILED:
		appsvc_result = APPSVC_RES_NOT_OK;
		break;
	case APP_CONTROL_RESULT_CANCELED:
		appsvc_result = APPSVC_RES_CANCEL;
		break;
	default:
		appsvc_result = APPSVC_RES_CANCEL;
		break;
	}

	ret = appsvc_send_result(reply_data, appsvc_result);
	bundle_free(reply_data);
	if (ret < 0) {
		if (ret == APPSVC_RET_EINVAL)
			return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
		else
			return app_control_error(APP_CONTROL_ERROR_LAUNCH_REJECTED, __FUNCTION__, NULL);
	}

	return APP_CONTROL_ERROR_NONE;
}


int app_control_add_extra_data(app_control_h app_control, const char *key, const char *value)
{
	if (app_control_validate(app_control) || app_control_validate_extra_data(key) || app_control_validate_extra_data(value))
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (app_control_validate_internal_key(key))
		return app_control_error(APP_CONTROL_ERROR_KEY_REJECTED, __FUNCTION__, "the given key is reserved as internal use");

	if (appsvc_get_data(app_control->data, key) != NULL) {
		/* overwrite any existing value */
		bundle_del(app_control->data, key);
	}

	if (appsvc_add_data(app_control->data, key, value) != 0)
		return app_control_error(APP_CONTROL_ERROR_KEY_REJECTED, __FUNCTION__, "failed to add data to the appsvc handle");

	return APP_CONTROL_ERROR_NONE;
}


int app_control_add_extra_data_array(app_control_h app_control, const char *key, const char* value[], int length)
{
	if (app_control_validate(app_control) || app_control_validate_extra_data(key))
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (value == NULL || length <= 0)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid array");

	if (app_control_validate_internal_key(key))
		return app_control_error(APP_CONTROL_ERROR_KEY_REJECTED, __FUNCTION__, "the given key is reserved as internal use");

	if (appsvc_get_data_array(app_control->data, key, NULL) != NULL) {
		/* overwrite any existing value */
		bundle_del(app_control->data, key);
	}

	if (appsvc_add_data_array(app_control->data, key, value, length) != 0)
		return app_control_error(APP_CONTROL_ERROR_KEY_REJECTED, __FUNCTION__, "failed to add array data to the appsvc handle");

	return APP_CONTROL_ERROR_NONE;
}

int app_control_remove_extra_data(app_control_h app_control, const char *key)
{
	if (app_control_validate(app_control) || app_control_validate_extra_data(key))
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (app_control_validate_internal_key(key))
		return app_control_error(APP_CONTROL_ERROR_KEY_REJECTED, __FUNCTION__, "the given key is reserved as internal use");

	if (bundle_del(app_control->data, key))
		return app_control_error(APP_CONTROL_ERROR_KEY_NOT_FOUND, __FUNCTION__, NULL);

	return APP_CONTROL_ERROR_NONE;
}

int app_control_get_extra_data(app_control_h app_control, const char *key, char **value)
{
	const char *data_value;

	if (app_control_validate(app_control) || app_control_validate_extra_data(key) || value == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (app_control_validate_internal_key(key))
		return app_control_error(APP_CONTROL_ERROR_KEY_REJECTED, __FUNCTION__, "the given key is reserved as internal use");

	data_value = appsvc_get_data(app_control->data, key);
	if (data_value == NULL) {
		if (errno == ENOTSUP)
			return app_control_error(APP_CONTROL_ERROR_INVALID_DATA_TYPE, __FUNCTION__, NULL);
		else
			return app_control_error(APP_CONTROL_ERROR_KEY_NOT_FOUND, __FUNCTION__, NULL);
	}

	*value = strdup(data_value);

	return APP_CONTROL_ERROR_NONE;
}

int app_control_get_extra_data_array(app_control_h app_control, const char *key, char ***value, int *length)
{
	const char **array_data;
	int array_data_length;
	char **array_data_clone;
	int i;

	if (app_control_validate(app_control) || app_control_validate_extra_data(key))
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (value == NULL || length == 0)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (app_control_validate_internal_key(key))
		return app_control_error(APP_CONTROL_ERROR_KEY_REJECTED, __FUNCTION__, "the given key is reserved as internal use");

	array_data = appsvc_get_data_array(app_control->data, key, &array_data_length);
	if (array_data == NULL) {
		if (errno == ENOTSUP)
			return app_control_error(APP_CONTROL_ERROR_INVALID_DATA_TYPE, __FUNCTION__, NULL);
		else
			return app_control_error(APP_CONTROL_ERROR_KEY_NOT_FOUND, __FUNCTION__, NULL);
	}

	array_data_clone = calloc(array_data_length, sizeof(char*));
	if (array_data_clone == NULL)
		return app_control_error(APP_CONTROL_ERROR_OUT_OF_MEMORY, __FUNCTION__, NULL);

	for (i = 0; i < array_data_length; i++) {
		if (array_data[i] != NULL)
			array_data_clone[i] = strdup(array_data[i]);
	}

	*value = array_data_clone;
	*length = array_data_length;

	return APP_CONTROL_ERROR_NONE;
}

int app_control_is_extra_data_array(app_control_h app_control, const char *key, bool *array)
{
	if (app_control_validate(app_control) || app_control_validate_extra_data(key) || array == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (app_control_validate_internal_key(key))
		return app_control_error(APP_CONTROL_ERROR_KEY_REJECTED, __FUNCTION__, "the given key is reserved as internal use");

	if (!appsvc_data_is_array(app_control->data, key))
		*array = false;
	else
		*array = true;

	return APP_CONTROL_ERROR_NONE;
}

typedef struct {
	app_control_h app_control;
	app_control_extra_data_cb callback;
	void* user_data;
	bool foreach_break;
} foreach_context_extra_data_t;

static void app_control_cb_broker_bundle_iterator(const char *key, const int type, const bundle_keyval_t *kv, void *user_data)
{
	foreach_context_extra_data_t* foreach_context = NULL;
	app_control_extra_data_cb extra_data_cb;

	if (key == NULL || !(type == BUNDLE_TYPE_STR || type == BUNDLE_TYPE_STR_ARRAY))
		return;

	foreach_context = (foreach_context_extra_data_t*)user_data;
	if (foreach_context->foreach_break == true)
		return;

	if (app_control_validate_internal_key(key))
		return;

	extra_data_cb = foreach_context->callback;

	if (extra_data_cb != NULL) {
		bool stop_foreach = false;

		stop_foreach = !extra_data_cb(foreach_context->app_control, key, foreach_context->user_data);

		foreach_context->foreach_break = stop_foreach;
	}
}

int app_control_foreach_extra_data(app_control_h app_control, app_control_extra_data_cb callback, void *user_data)
{
	foreach_context_extra_data_t foreach_context = {
		.app_control = app_control,
		.callback = callback,
		.user_data = user_data,
		.foreach_break = false
	};

	if (app_control_validate(app_control) || callback == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	bundle_foreach(app_control->data, app_control_cb_broker_bundle_iterator, &foreach_context);

	return APP_CONTROL_ERROR_NONE;
}

typedef struct {
	app_control_h app_control;
	app_control_app_matched_cb callback;
	void* user_data;
	bool foreach_break;
} foreach_context_launchable_app_t;

int app_control_cb_broker_foreach_app_matched(const char *package, void *data)
{
	foreach_context_launchable_app_t *foreach_context;
	app_control_app_matched_cb app_matched_cb;

	if (package == NULL || data == NULL) {
		app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
		return -1;
	}

	foreach_context = (foreach_context_launchable_app_t*)data;
	if (foreach_context->foreach_break == true)
		return -1;

	app_matched_cb = foreach_context->callback;
	if (app_matched_cb != NULL) {
		bool stop_foreach = false;

		stop_foreach = !app_matched_cb(foreach_context->app_control, package, foreach_context->user_data);

		foreach_context->foreach_break = stop_foreach;
	}

	return 0;
}

int app_control_foreach_app_matched(app_control_h app_control, app_control_app_matched_cb callback, void *user_data)
{
	foreach_context_launchable_app_t foreach_context = {
		.app_control = app_control,
		.callback = callback,
		.user_data = user_data,
		.foreach_break = false
	};

	if (app_control_validate(app_control) || callback == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	appsvc_usr_get_list(app_control->data, app_control_cb_broker_foreach_app_matched, &foreach_context, getuid());

	return APP_CONTROL_ERROR_NONE;
}

int app_control_get_caller(app_control_h app_control, char **package)
{
	const char *bundle_value;
	char *package_dup;

	if (app_control_validate(app_control) || package == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (app_control->type != APP_CONTROL_TYPE_EVENT)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid app_control handle type");

	bundle_value = bundle_get_val(app_control->data, AUL_K_CALLER_APPID);
	if (bundle_value == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "failed to retrieve the appid of the caller");

	package_dup = strdup(bundle_value);
	if (package_dup == NULL)
		return app_control_error(APP_CONTROL_ERROR_OUT_OF_MEMORY, __FUNCTION__, NULL);

	*package = package_dup;

	return APP_CONTROL_ERROR_NONE;
}

int app_control_is_reply_requested(app_control_h app_control, bool *requested)
{
	const char *bundle_value;

	if (app_control_validate(app_control) || requested == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	if (app_control->type != APP_CONTROL_TYPE_EVENT)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid app_control handle type");

	bundle_value = bundle_get_val(app_control->data, AUL_K_WAIT_RESULT);
	if (bundle_value != NULL)
		*requested = true;
	else
		*requested = false;

	return APP_CONTROL_ERROR_NONE;
}

int app_control_import_from_bundle(app_control_h app_control, bundle *data)
{
	bundle *data_dup = NULL;

	if (app_control_validate(app_control) || data == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	data_dup = bundle_dup(data);
	if (data_dup == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "failed to duplicate the bundle");

	if (app_control->data != NULL)
		bundle_free(app_control->data);

	app_control->data = data_dup;

	return APP_CONTROL_ERROR_NONE;
}

int app_control_export_as_bundle(app_control_h app_control, bundle **data)
{
	bundle *data_dup = NULL;

	if (app_control_validate(app_control) || data == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	data_dup = bundle_dup(app_control->data);
	if (data_dup == NULL)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, "failed to duplicate the bundle");

	*data = data_dup;

	return APP_CONTROL_ERROR_NONE;
}

int app_control_request_transient_app(app_control_h app_control, unsigned int callee_id, app_control_host_res_fn cbfunc, void *data)
{
	int ret;

	if (app_control_validate(app_control))
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	ret = appsvc_request_transient_app(app_control->data, callee_id, (appsvc_host_res_fn)cbfunc, data);
	if (ret < 0)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	return APP_CONTROL_ERROR_NONE;
}

int app_control_enable_app_started_result_event(app_control_h app_control)
{
	int ret;

	if (app_control_validate(app_control))
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	ret = aul_svc_subscribe_launch_result(app_control->data, AUL_SVC_K_LAUNCH_RESULT_APP_STARTED);
	if (ret < 0)
		return app_control_error(APP_CONTROL_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);

	return APP_CONTROL_ERROR_NONE;
}
