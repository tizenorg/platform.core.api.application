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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <bundle.h>
#include <aul.h>
#include <appsvc.h>
#include <dlog.h>

#include <app_service.h>
#include <app_service_private.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "CAPI_APPFW_APPLICATION_SERVICE"

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


typedef enum {
	SERVICE_TYPE_REQUEST,
	SERVICE_TYPE_EVENT,
	SERVICE_TYPE_REPLY,
} service_type_e;

struct service_s {
	int id;
	service_type_e type;
	bundle *data;
};

typedef struct service_request_context_s {
	service_h service;
	service_reply_cb reply_cb;
	void *user_data;
} *service_request_context_h;

extern int appsvc_allow_transient_app(bundle *b, unsigned int id);

static int service_create_reply(bundle *data, struct service_s **service);

static const char* service_error_to_string(service_error_e error)
{
	switch (error)
	{
	case SERVICE_ERROR_NONE:
		return "NONE";

	case SERVICE_ERROR_INVALID_PARAMETER:
		return "INVALID_PARAMETER";

	case SERVICE_ERROR_OUT_OF_MEMORY:
		return "OUT_OF_MEMORY";

	case SERVICE_ERROR_APP_NOT_FOUND:
		return "APP_NOT_FOUND";

	case SERVICE_ERROR_KEY_NOT_FOUND:
		return "KEY_NOT_FOUND";

	case SERVICE_ERROR_KEY_REJECTED:
		return "KEY_REJECTED";

	case SERVICE_ERROR_INVALID_DATA_TYPE:
		return "INVALID_DATA_TYPE";

	case SERVICE_ERROR_LAUNCH_REJECTED:
		return "LAUNCH_REJECTED";

	default :
		return "UNKNOWN";
	}
}

int service_error(service_error_e error, const char* function, const char *description)
{
	if (description)
	{
		LOGE("[%s] %s(0x%08x) : %s", function, service_error_to_string(error), error, description);	
	}
	else
	{
		LOGE("[%s] %s(0x%08x)", function, service_error_to_string(error), error);	
	}

	return error;
}

static int service_validate_extra_data(const char *data)
{
	if (data == NULL || data[0] == '\0')
	{
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	return SERVICE_ERROR_NONE;
}

static int service_valiate_service(service_h service)
{
	if (service == NULL || service->data == NULL)
	{
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	return SERVICE_ERROR_NONE;
}

static int service_new_id()
{
	static int sid = 0;
	return sid++;
}

int service_validate_internal_key(const char *key)
{
	if (strncmp(BUNDLE_KEY_PREFIX_AUL, key, strlen(BUNDLE_KEY_PREFIX_AUL)) == 0)
	{
		return -1;
	}

	if (strncmp(BUNDLE_KEY_PREFIX_SERVICE, key, strlen(BUNDLE_KEY_PREFIX_SERVICE)) == 0)
	{
		return -1;
	}

	return 0;
}

static void service_request_result_broker(bundle *appsvc_bundle, int appsvc_request_code, appsvc_result_val appsvc_result, void *appsvc_data)
{
	service_request_context_h request_context;
	service_h request;
	service_h reply = NULL;
	service_result_e result;
	void *user_data;
	service_reply_cb reply_cb;	

	if (appsvc_data == NULL)
	{
		service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid service reply");
		return;
	}

	if (service_create_reply(appsvc_bundle, &reply) != 0)
	{
		service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "failed to create service reply");
		return;		
	}

	request_context = appsvc_data;
	request = request_context->service;

	switch (appsvc_result)
	{
	case APPSVC_RES_OK:
		result = SERVICE_RESULT_SUCCEEDED;
		break;

	case APPSVC_RES_NOT_OK:
		result = SERVICE_RESULT_FAILED;
		break;

	case APPSVC_RES_CANCEL:
		result = SERVICE_RESULT_CANCELED;
		break;

	default:
		result = SERVICE_RESULT_CANCELED;
		break;
	}

	user_data = request_context->user_data;
	reply_cb = request_context->reply_cb;

	if (reply_cb != NULL)
	{
		reply_cb(request, reply, result, user_data);
	}
	else
	{
		service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid callback ");
	}

	service_destroy(reply);

	if (request_context->service != NULL)
	{
		service_destroy(request_context->service);
	}

	free(request_context);
}


int service_create(service_h *service)
{
	return service_create_request(NULL, service);
}

int service_create_request(bundle *data, service_h *service)
{
	struct service_s *service_request;

	if (service == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	service_request = malloc(sizeof(struct service_s));

	if (service_request == NULL)
	{
		return service_error(SERVICE_ERROR_OUT_OF_MEMORY, __FUNCTION__, "failed to create a service handle");
	}

	service_request->type = SERVICE_TYPE_REQUEST;

	if (data != NULL)
	{
		service_request->data = bundle_dup(data);
	}
	else
	{
		service_request->data = bundle_create();
	}

	if (service_request->data == NULL)
	{
		free(service_request);
		return service_error(SERVICE_ERROR_OUT_OF_MEMORY, __FUNCTION__, "failed to create a bundle");
	}

	service_request->id = service_new_id();

	*service = service_request;

	return SERVICE_ERROR_NONE;
}

int service_create_event(bundle *data, struct service_s **service)
{
	struct service_s *service_event;

	const char *operation;

	if (data == NULL || service == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	service_event = malloc(sizeof(struct service_s));

	if (service_event == NULL)
	{
		return service_error(SERVICE_ERROR_OUT_OF_MEMORY, __FUNCTION__, "failed to create a service handle");
	}	

	service_event->type = SERVICE_TYPE_EVENT;
	service_event->data = bundle_dup(data);
	service_event->id = service_new_id();

	operation = appsvc_get_operation(service_event->data);

	if (operation == NULL)
	{
		appsvc_set_operation(service_event->data, SERVICE_OPERATION_DEFAULT);
	}

	*service = service_event;

	return SERVICE_ERROR_NONE;
}

static int service_create_reply(bundle *data, struct service_s **service)
{
	struct service_s *service_reply;

	if (data == NULL || service == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	service_reply = malloc(sizeof(struct service_s));

	if (service_reply == NULL)
	{
		return service_error(SERVICE_ERROR_OUT_OF_MEMORY, __FUNCTION__, "failed to create a service handle");
	}	

	service_reply->type = SERVICE_TYPE_REPLY;
	service_reply->data = bundle_dup(data);
	service_reply->id = service_new_id();

	*service = service_reply;

	return SERVICE_ERROR_NONE;
}

int service_destroy(service_h service)
{
	if (service_valiate_service(service))
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	bundle_free(service->data);
	service->data = NULL;
	free(service);

	return SERVICE_ERROR_NONE;
}

int service_to_bundle(service_h service, bundle **data)
{
	if (service_valiate_service(service) || data == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	*data = service->data;

	return SERVICE_ERROR_NONE;
}

int service_set_operation(service_h service, const char *operation)
{
	if (service_valiate_service(service))
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (operation != NULL)
	{
		if (appsvc_set_operation(service->data, operation) != 0)
		{
			return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid operation");
		}
	}
	else
	{
		bundle_del(service->data, BUNDLE_KEY_OPERATION);
	}

	return SERVICE_ERROR_NONE;
}

int service_get_operation(service_h service, char **operation)
{
	const char *operation_value;

	if (service_valiate_service(service) || operation == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	operation_value = appsvc_get_operation(service->data);

	if (operation_value != NULL)
	{
		*operation = strdup(operation_value);
	}
	else
	{
		*operation = NULL;
	}

	return SERVICE_ERROR_NONE;
}


int service_set_uri(service_h service, const char *uri)
{
	if (service_valiate_service(service))
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (uri != NULL)
	{
		if (appsvc_set_uri(service->data, uri) != 0)
		{
			return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid URI");
		}
	}
	else
	{
		bundle_del(service->data, BUNDLE_KEY_URI);
	}
	
	return SERVICE_ERROR_NONE;
}


int service_get_uri(service_h service, char **uri)
{
	const char *uri_value;

	if (service_valiate_service(service) || uri == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	uri_value = appsvc_get_uri(service->data);

	if (uri_value != NULL)
	{
		*uri = strdup(uri_value);
	}
	else
	{
		*uri = NULL;
	}

	return SERVICE_ERROR_NONE;
}


int service_set_mime(service_h service, const char *mime)
{
	if (service_valiate_service(service))
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (mime != NULL)
	{
		if (appsvc_set_mime(service->data, mime) != 0)
		{
			return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid MIME type");
		}
	}
	else
	{
		bundle_del(service->data, BUNDLE_KEY_MIME);
	}

	return SERVICE_ERROR_NONE;
}


int service_get_mime(service_h service, char **mime)
{
	const char *mime_value;

	if (service_valiate_service(service) || mime == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	mime_value = appsvc_get_mime(service->data);

	if (mime_value != NULL)
	{
		*mime = strdup(mime_value);
	}
	else
	{
		*mime = NULL;
	}

	return SERVICE_ERROR_NONE;
}


int service_set_category(service_h service, const char *category)
{
	if (service_valiate_service(service))
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (category != NULL)
	{
		if (appsvc_set_category(service->data, category) != 0)
		{
			return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid MIME type");
		}
	}
	else
	{
		bundle_del(service->data, BUNDLE_KEY_MIME);
	}

	return SERVICE_ERROR_NONE;
}


int service_get_category(service_h service, char **category)
{
	const char *category_value;

	if (service_valiate_service(service) || category == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	category_value = appsvc_get_category(service->data);

	if (category_value != NULL)
	{
		*category = strdup(category_value);
	}
	else
	{
		*category = NULL;
	}

	return SERVICE_ERROR_NONE;
}


int service_set_package(service_h service, const char *package)
{
	// TODO: this function must be deprecated
	return service_set_app_id(service, package);
}

int service_get_package(service_h service, char **package)
{
	// TODO: this function must be deprecated
	return service_get_app_id(service, package);
}


int service_set_app_id(service_h service, const char *app_id)
{
	if (service_valiate_service(service))
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (app_id != NULL)
	{
		if (appsvc_set_appid(service->data, app_id) != 0)
		{
			return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid application ID");
		}
	}
	else
	{
		bundle_del(service->data, BUNDLE_KEY_PACKAGE);
	}

	return SERVICE_ERROR_NONE;
}


int service_get_app_id(service_h service, char **app_id)
{
	const char *app_id_value;

	if (service_valiate_service(service) || app_id == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	app_id_value = appsvc_get_appid(service->data);

	if (app_id_value != NULL)
	{
		*app_id = strdup(app_id_value);
	}
	else
	{
		*app_id = NULL;
	}

	return SERVICE_ERROR_NONE;
}

int service_set_window(service_h service, unsigned int id)
{
	if (service_valiate_service(service))
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (id > 0)
	{
		if (appsvc_allow_transient_app(service->data, id) != 0)
		{
			return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid id");
		}
	}
	else
	{
		bundle_del(service->data, BUNDLE_KEY_WINDOW);
	}

	return SERVICE_ERROR_NONE;
}

int service_get_window(service_h service, unsigned int *id)
{
	const char *window_id;

	if (service_valiate_service(service) || id == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	window_id = bundle_get_val(service->data, BUNDLE_KEY_WINDOW);

	if (window_id != NULL)
	{
		*id = atoi(window_id);
	}
	else
	{
		*id = 0;
	}

	return SERVICE_ERROR_NONE;
}

int service_clone(service_h *clone, service_h service)
{
	service_h service_clone;

	if (service_valiate_service(service) || clone == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	service_clone = malloc(sizeof(struct service_s));

	if (service_clone == NULL)
	{
		return service_error(SERVICE_ERROR_OUT_OF_MEMORY, __FUNCTION__, "failed to create a service handle");
	}

	service_clone->id = service_new_id();
	service_clone->type = service->type;
	service_clone->data = bundle_dup(service->data);

	*clone = service_clone;

	return SERVICE_ERROR_NONE;
}


int service_send_launch_request(service_h service, service_reply_cb callback, void *user_data)
{
	const char *operation;
	const char *appid;

	bool implicit_default_operation = false;
	int launch_pid;

	service_request_context_h request_context = NULL;

	if (service_valiate_service(service))
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	operation = appsvc_get_operation(service->data);

	if (operation == NULL)
	{
		implicit_default_operation = true;
		operation = SERVICE_OPERATION_DEFAULT;
	}

	appid = appsvc_get_appid(service->data);

	// operation : default
	if (!strcmp(operation, SERVICE_OPERATION_DEFAULT))
	{
		if (appid == NULL)
		{
			return service_error(SERVICE_ERROR_APP_NOT_FOUND, __FUNCTION__, "package must be specified if the operation is default value");
		}
	}

	if (callback != NULL)
	{
		service_h request_clone = NULL;

		request_context = calloc(1, sizeof(struct service_request_context_s));

		if (request_context == NULL)
		{
			return service_error(SERVICE_ERROR_OUT_OF_MEMORY, __FUNCTION__, NULL);
		}

		request_context->reply_cb = callback;

		if (service_clone(&request_clone, service) != SERVICE_ERROR_NONE)
		{
			free(request_context);
			return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "failed to clone the service request handle");
		}

		request_context->service = request_clone;
		request_context->user_data = user_data;
	}

	if (implicit_default_operation == true)
	{
		appsvc_set_operation(service->data, SERVICE_OPERATION_DEFAULT);
	}

	launch_pid = appsvc_run_service(service->data, service->id, callback ? service_request_result_broker : NULL, request_context);

	if (implicit_default_operation == true)
	{
		bundle_del(service->data, BUNDLE_KEY_OPERATION);
	}

	if (launch_pid < 0)
	{
		if (launch_pid == APPSVC_RET_ENOMATCH)
		{
			return service_error(SERVICE_ERROR_APP_NOT_FOUND, __FUNCTION__, NULL);
		}
		else
		{
			return service_error(SERVICE_ERROR_LAUNCH_REJECTED, __FUNCTION__, NULL);
		}
	}

	return SERVICE_ERROR_NONE;
}

static bool service_copy_reply_data_cb(service_h service, const char *key, void *user_data)
{
	bundle *reply_data = user_data;
	char *value = NULL;
	char **value_array = NULL;
	int value_array_length = 0;
	int value_array_index = 0;

	if (reply_data == NULL)
	{
		service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
		return false;
	}

	if (appsvc_data_is_array(service->data, key))
	{
		service_get_extra_data_array(service, key, &value_array, &value_array_length);
		appsvc_add_data_array(reply_data, key, (const char**)value_array, value_array_length);

		for (value_array_index=0; value_array_index < value_array_length; value_array_index++)
		{
			free(value_array[value_array_index]);
		}

		free(value_array);
	}
	else
	{
		service_get_extra_data(service, key, &value);
		appsvc_add_data(reply_data, key, value);
		free(value);
	}

	return true;
}

int service_reply_to_launch_request(service_h reply, service_h request, service_result_e result)
{
	bundle *reply_data;
	int appsvc_result;

	if (service_valiate_service(reply) || service_valiate_service(request))
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (appsvc_create_result_bundle(request->data, &reply_data) != 0)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "failed to create a result bundle");
	}

	service_foreach_extra_data(reply, service_copy_reply_data_cb, reply_data);

	switch (result)
	{
	case SERVICE_RESULT_SUCCEEDED:
		appsvc_result = APPSVC_RES_OK;
		break;

	case SERVICE_RESULT_FAILED:
		appsvc_result = APPSVC_RES_NOT_OK;
		break;

	case SERVICE_RESULT_CANCELED:
		appsvc_result = APPSVC_RES_CANCEL;
		break;

	default:
		appsvc_result = APPSVC_RES_CANCEL;
		break;
	}

	appsvc_send_result(reply_data, appsvc_result);
	
	return SERVICE_ERROR_NONE;
}


int service_add_extra_data(service_h service, const char *key, const char *value)
{
	if (service_valiate_service(service) || service_validate_extra_data(key) || service_validate_extra_data(value))
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (service_validate_internal_key(key))
	{
		return service_error(SERVICE_ERROR_KEY_REJECTED, __FUNCTION__, "the given key is reserved as internal use");
	}

	if (appsvc_get_data(service->data, key) != NULL)
	{
		// overwrite any existing value
		bundle_del(service->data, key);
	}

	if (appsvc_add_data(service->data, key, value) != 0)
	{
		return service_error(SERVICE_ERROR_KEY_REJECTED, __FUNCTION__, "failed to add data to the appsvc handle");
	}

	return SERVICE_ERROR_NONE;
}


int service_add_extra_data_array(service_h service, const char *key, const char* value[], int length)
{
	if (service_valiate_service(service) || service_validate_extra_data(key))
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (value == NULL || length <= 0)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid array");
	}	

	if (service_validate_internal_key(key))
	{
		return service_error(SERVICE_ERROR_KEY_REJECTED, __FUNCTION__, "the given key is reserved as internal use");
	}

	if (appsvc_get_data_array(service->data, key, NULL) != NULL)
	{
		// overwrite any existing value
		bundle_del(service->data,key);
	}

	if (appsvc_add_data_array(service->data, key, value, length) != 0)
	{
		return service_error(SERVICE_ERROR_KEY_REJECTED, __FUNCTION__, "failed to add array data to the appsvc handle");		
	}

	return SERVICE_ERROR_NONE;
}


int service_remove_extra_data(service_h service, const char *key)
{
	if (service_valiate_service(service) || service_validate_extra_data(key))
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (service_validate_internal_key(key))
	{
		return service_error(SERVICE_ERROR_KEY_REJECTED, __FUNCTION__, "the given key is reserved as internal use");
	}

	if (bundle_del(service->data, key))
	{
		return service_error(SERVICE_ERROR_KEY_NOT_FOUND, __FUNCTION__, NULL);
	}

	return SERVICE_ERROR_NONE;
}


int service_get_extra_data(service_h service, const char *key, char **value)
{
	const char *data_value;

	if (service_valiate_service(service) || service_validate_extra_data(key) || value == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}


	if (service_validate_internal_key(key))
	{
		return service_error(SERVICE_ERROR_KEY_REJECTED, __FUNCTION__, "the given key is reserved as internal use");
	}

	data_value = appsvc_get_data(service->data, key);

	if (data_value == NULL)	
	{
		if (errno == ENOTSUP)
		{
			return service_error(SERVICE_ERROR_INVALID_DATA_TYPE, __FUNCTION__, NULL);
		}
		else
		{
			return service_error(SERVICE_ERROR_KEY_NOT_FOUND, __FUNCTION__, NULL);
		}
	}

	*value = strdup(data_value);

	return SERVICE_ERROR_NONE;
}


int service_get_extra_data_array(service_h service, const char *key, char ***value, int *length)
{
	const char **array_data;
	int array_data_length;
	char **array_data_clone;
	int i;

	if (service_valiate_service(service) || service_validate_extra_data(key))
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (value == NULL || length == 0)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (service_validate_internal_key(key))
	{
		return service_error(SERVICE_ERROR_KEY_REJECTED, __FUNCTION__, "the given key is reserved as internal use");
	}

	array_data = appsvc_get_data_array(service->data, key, &array_data_length);

	if (array_data == NULL)
	{
		if (errno == ENOTSUP)
		{
			return service_error(SERVICE_ERROR_INVALID_DATA_TYPE, __FUNCTION__, NULL);
		}
		else
		{
			return service_error(SERVICE_ERROR_KEY_NOT_FOUND, __FUNCTION__, NULL);
		}
	}

	array_data_clone = calloc(array_data_length, sizeof(char*));

	if (array_data_clone == NULL)
	{
		return service_error(SERVICE_ERROR_OUT_OF_MEMORY, __FUNCTION__, NULL);
	}

	for (i=0; i<array_data_length; i++)
	{
		if (array_data[i] != NULL)
		{
			array_data_clone[i] = strdup(array_data[i]);
		}
	}

	*value = array_data_clone;
	*length = array_data_length;

	return SERVICE_ERROR_NONE;
}


int service_is_extra_data_array(service_h service, const char *key, bool *array)
{
	if (service_valiate_service(service) || service_validate_extra_data(key) || array == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (service_validate_internal_key(key))
	{
		return service_error(SERVICE_ERROR_KEY_REJECTED, __FUNCTION__, "the given key is reserved as internal use");
	}

	if (!appsvc_data_is_array(service->data, key))
	{
		*array = false;
	}
	else
	{
		*array = true;
	}

	return SERVICE_ERROR_NONE;
}


typedef struct {
	service_h service;
	service_extra_data_cb callback;
	void* user_data;
	bool foreach_break;
} foreach_context_extra_data_t;

static void service_cb_broker_bundle_iterator(const char *key, const int type, const bundle_keyval_t *kv, void *user_data)
{
	foreach_context_extra_data_t* foreach_context = NULL;
	service_extra_data_cb extra_data_cb;

	if (key == NULL || !(type == BUNDLE_TYPE_STR || type == BUNDLE_TYPE_STR_ARRAY))
	{
		return;
	}

	foreach_context = (foreach_context_extra_data_t*)user_data;

	if (foreach_context->foreach_break == true)
	{
		return;
	}

	if (service_validate_internal_key(key))
	{
		return;
	}
	
	extra_data_cb = foreach_context->callback;

	if (extra_data_cb != NULL)
	{
		bool stop_foreach = false;
		
		stop_foreach = !extra_data_cb(foreach_context->service, key, foreach_context->user_data);
	
		foreach_context->foreach_break = stop_foreach;
	}

}


int service_foreach_extra_data(service_h service, service_extra_data_cb callback, void *user_data)
{
	foreach_context_extra_data_t foreach_context = {
		.service = service,
		.callback = callback,
		.user_data = user_data,
		.foreach_break = false
	};
	
	if (service_valiate_service(service) || callback == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	bundle_foreach(service->data, service_cb_broker_bundle_iterator, &foreach_context);

	return SERVICE_ERROR_NONE;
}

typedef struct {
	service_h service;
	service_app_matched_cb callback;
	void* user_data;
	bool foreach_break;
} foreach_context_launchable_app_t;

int service_cb_broker_foreach_app_matched(const char *package, void *data)
{
	foreach_context_launchable_app_t *foreach_context;
	service_app_matched_cb app_matched_cb;

	if (package == NULL || data == NULL)
	{
		service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
		return -1;
	}

	foreach_context = (foreach_context_launchable_app_t*)data;

	if (foreach_context->foreach_break == true)
	{
		return -1;
	}

	app_matched_cb = foreach_context->callback;

	if (app_matched_cb != NULL)
	{
		bool stop_foreach = false;
		
		stop_foreach = !app_matched_cb(foreach_context->service, package, foreach_context->user_data);
	
		foreach_context->foreach_break = stop_foreach;
	}

	return 0;
}

int service_foreach_app_matched(service_h service, service_app_matched_cb callback, void *user_data)
{
	foreach_context_launchable_app_t foreach_context = {
		.service = service,
		.callback = callback,
		.user_data = user_data,
		.foreach_break = false
	};

	if (service_valiate_service(service) || callback == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	appsvc_get_list(service->data, service_cb_broker_foreach_app_matched, &foreach_context);

	return SERVICE_ERROR_NONE;
}


int service_get_caller(service_h service, char **package)
{
	const char *bundle_value;
	char *package_dup;

	if (service_valiate_service(service) || package == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (service->type != SERVICE_TYPE_EVENT)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid service handle type");
	}

        bundle_value = bundle_get_val(service->data, AUL_K_CALLER_APPID);
        if (bundle_value == NULL)
        {
                return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "failed to retrieve the appid of the caller");
        }

        package_dup = strdup(bundle_value);

        if (package_dup == NULL)
        {
                return service_error(SERVICE_ERROR_OUT_OF_MEMORY, __FUNCTION__, NULL);
        }

        *package = package_dup;

	return SERVICE_ERROR_NONE;
}


int service_is_reply_requested(service_h service, bool *requested)
{
	const char *bundle_value;
	
	if (service_valiate_service(service) || requested == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (service->type != SERVICE_TYPE_EVENT)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "invalid service handle type");
	}

	bundle_value = bundle_get_val(service->data, AUL_K_WAIT_RESULT);

	if (bundle_value != NULL)
	{
		*requested = true;
	}
	else
	{
		*requested = false;
	}

	return SERVICE_ERROR_NONE;
}

int service_import_from_bundle(service_h service, bundle *data)
{
	bundle *data_dup = NULL;

	if (service_valiate_service(service) || data == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	data_dup = bundle_dup(data);

	if (data_dup == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "failed to duplicate the bundle");
	}

	if (service->data != NULL)
	{
		bundle_free(service->data);
	}

	service->data = data_dup;

	return SERVICE_ERROR_NONE;
}

int service_export_as_bundle(service_h service, bundle **data)
{
	bundle *data_dup = NULL;

	if (service_valiate_service(service) || data == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	data_dup = bundle_dup(service->data);

	if (data_dup == NULL)
	{
		return service_error(SERVICE_ERROR_INVALID_PARAMETER, __FUNCTION__, "failed to duplicate the bundle");
	}

	*data = data_dup;

	return SERVICE_ERROR_NONE;
}

