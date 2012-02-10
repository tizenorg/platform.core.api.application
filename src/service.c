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

#define LOG_TAG "TIZEN_N_SERVICE"

#define BUNDLE_KEY_PREFIX_AUL "__AUL_"
#define BUNDLE_KEY_PREFIX_SERVICE "__APP_SVC_"

#define BUNDLE_KEY_OPERATION	"__APP_SVC_OP_TYPE__"
#define BUNDLE_KEY_URI		"__APP_SVC_URI__"
#define BUNDLE_KEY_MIME		"__APP_SVC_MIME_TYPE__"
#define BUNDLE_KEY_DATA		"__APP_SVC_DATA__"
#define BUNDLE_KEY_PACKAGE	"__APP_SVC_PKG_NAME__"

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

static int service_create_reply(bundle *data, struct service_s **service);

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
	service_h reply;
	service_result_e result;
	void *user_data;
	service_reply_cb reply_cb;	

	if (appsvc_data == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid service reply", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return;
	}

	if (service_create_reply(appsvc_bundle, &reply) != 0)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : failed to create service reply", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
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
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid callback", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
	}

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
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	service_request = malloc(sizeof(struct service_s));

	if (service_request == NULL)
	{
		LOGE("[%s] OUT_OF_MEMORY(0x%08x) : failed to create a service handle", __FUNCTION__, SERVICE_ERROR_OUT_OF_MEMORY);
		return SERVICE_ERROR_OUT_OF_MEMORY;
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
		LOGE("[%s] OUT_OF_MEMORY(0x%08x) : failed to create a bundle handle", __FUNCTION__);
		return SERVICE_ERROR_OUT_OF_MEMORY;
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
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	service_event = malloc(sizeof(struct service_s));

	if (service_event == NULL)
	{
		LOGE("[%s] OUT_OF_MEMORY(0x%08x) : failed to create a service handle", __FUNCTION__, SERVICE_ERROR_OUT_OF_MEMORY);
		return SERVICE_ERROR_OUT_OF_MEMORY;
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

int service_impl_create_event(bundle *data, struct service_s **service)
{
	return service_create_event(data, service);
}

static int service_create_reply(bundle *data, struct service_s **service)
{
	struct service_s *service_reply;

	if (data == NULL || service == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	service_reply = malloc(sizeof(struct service_s));

	if (service_reply == NULL)
	{
		LOGE("[%s] OUT_OF_MEMORY(0x%08x) : failed to create a service handle", __FUNCTION__, SERVICE_ERROR_OUT_OF_MEMORY);
		return SERVICE_ERROR_OUT_OF_MEMORY;
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
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	bundle_free(service->data);
	service->data = NULL;
	free(service);

	return SERVICE_ERROR_NONE;
}

int service_to_bundle(service_h service, bundle **data)
{
	if (service_valiate_service(service))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (data == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid output param", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	*data = service->data;

	return SERVICE_ERROR_NONE;
}

int service_set_operation(service_h service, const char *operation)
{
	if (service_valiate_service(service))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (operation != NULL)
	{
		if (appsvc_set_operation(service->data, operation) != 0)
		{
			LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid operation", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
			return SERVICE_ERROR_INVALID_PARAMETER;
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

	if (service_valiate_service(service))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (operation == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid operation", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
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
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (uri != NULL)
	{
		if (appsvc_set_uri(service->data, uri) != 0)
		{
			LOGE("[%s] INVALID_PARAMETER(0x%08x : invalid URI)", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
			return SERVICE_ERROR_INVALID_PARAMETER;
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

	if (service_valiate_service(service))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (uri == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid URI", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
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
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (mime != NULL)
	{
		if (appsvc_set_mime(service->data, mime) != 0)
		{
			LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid MIME type", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
			return SERVICE_ERROR_INVALID_PARAMETER;
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

	if (service_valiate_service(service))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (mime == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid MIME type", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
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


int service_set_package(service_h service, const char *package)
{
	if (service_valiate_service(service))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (package != NULL)
	{
		if (appsvc_set_pkgname(service->data, package) != 0)
		{
			LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid package", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
			return SERVICE_ERROR_INVALID_PARAMETER;
		}
	}
	else
	{
		bundle_del(service->data, BUNDLE_KEY_PACKAGE);
	}

	return SERVICE_ERROR_NONE;
}

int service_get_package(service_h service, char **package)
{
	const char *package_value;

	if (service_valiate_service(service))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (package == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid package", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	package_value = appsvc_get_pkgname(service->data);

	if (package_value != NULL)
	{
		*package = strdup(package_value);
	}
	else
	{
		*package = NULL;
	}

	return SERVICE_ERROR_NONE;
}

int service_clone(service_h *clone, service_h service)
{
	service_h service_clone;

	if (service_valiate_service(service))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (clone == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	service_clone = malloc(sizeof(struct service_s));

	if (service_clone == NULL)
	{
		LOGE("[%s] OUT_OF_MEMORY(0x%08x) : failed to create a service handle", __FUNCTION__, SERVICE_ERROR_OUT_OF_MEMORY);
		return SERVICE_ERROR_OUT_OF_MEMORY;
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
	const char *uri;
	const char *mime;
	const char *package;

	bool implicit_default_operation = false;
	int launch_pid;

	if (service_valiate_service(service))
	{
		LOGE("[%s] INVALID_PARAMETER : invalid handle", __FUNCTION__);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	operation = appsvc_get_operation(service->data);

	if (operation == NULL)
	{
		implicit_default_operation = true;
		operation = SERVICE_OPERATION_DEFAULT;
	}

	uri = appsvc_get_uri(service->data);
	mime = appsvc_get_mime(service->data);
	package = appsvc_get_pkgname(service->data);

	// operation : default
	if (!strcmp(operation, SERVICE_OPERATION_DEFAULT))
	{
		if (package == NULL || package[0] == '\0')
		{
			LOGE("[%s] APP_NOT_FOUND(0x%08x) : package must be specified if the operation is default", __FUNCTION__, SERVICE_ERROR_APP_NOT_FOUND);
			return SERVICE_ERROR_APP_NOT_FOUND;
		}
	}

	if (package != NULL && package[0] != '\0')
	{
		if (aul_app_is_running(package))
		{
			if (aul_open_app(package) <= 0)
			{
				LOGE("[%s] APP_NOT_FOUND(0x%08x) : resume the application(%s)", __FUNCTION__, SERVICE_ERROR_APP_NOT_FOUND, package);
				return SERVICE_ERROR_APP_NOT_FOUND;
			}
		}
	}

	service_request_context_h request_context = NULL;

	if (callback != NULL)
	{
		request_context = calloc(1, sizeof(struct service_request_context_s));
		// request_context will be deallocated from service_request_result_broker()

		if (request_context == NULL)
		{
			LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, SERVICE_ERROR_OUT_OF_MEMORY);
			return SERVICE_ERROR_OUT_OF_MEMORY;
		}

		request_context->reply_cb = callback;
		request_context->service = service;
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
		LOGE("[%s] APP_NOT_FOUND(0x%08x) : operation(%s) package(%s) uri(%s) mime(%s)",
			__FUNCTION__, SERVICE_ERROR_APP_NOT_FOUND, operation, package, uri, mime);
		return SERVICE_ERROR_APP_NOT_FOUND;
	}

	return SERVICE_ERROR_NONE;
}

static bool service_copy_reply_data_cb(service_h service, const char *key, void *user_data)
{
	bundle *reply_data;
	bool array = false;
	char *value;
	const char **value_array;
	int value_array_length;

	if (user_data == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return false;
	}

	reply_data = user_data;

	if (!service_is_extra_data_array(service, key, &array))
	{
		if (array == true)
		{
			service_get_extra_data_array(service, key, (char***)&value_array, &value_array_length);
			appsvc_add_data_array(reply_data, key, value_array, value_array_length);
		}
		else
		{
			service_get_extra_data(service, key, &value);
			appsvc_add_data(reply_data, key, value);
		}
	}

	return true;
}

int service_reply_to_launch_request(service_h reply, service_h request, service_result_e result)
{
	bundle *reply_data;
	int appsvc_result;

	if (service_valiate_service(reply))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid reply handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (service_valiate_service(request))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid request handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (appsvc_create_result_bundle(request->data, &reply_data) != 0)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : failed to create result bundle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
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
	if (service_valiate_service(service))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (service_validate_extra_data(key))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid key", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (service_validate_internal_key(key))
	{
		LOGE("[%s] KEY_REJECTED(0x%08x) : key(%s) rejected", __FUNCTION__, SERVICE_ERROR_KEY_REJECTED, key);
		return SERVICE_ERROR_KEY_REJECTED;	
	}

	if (service_validate_extra_data(value))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid value", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (appsvc_get_data(service->data, key) != NULL)
	{
		// overwrite any existing value
		bundle_del(service->data, key);
	}

	if (appsvc_add_data(service->data, key, value) != 0)
	{
		LOGE("[%s] KEY_REJECTED(0x%08x) : key(%s)/value(%s) is rejected from appsvc",
			__FUNCTION__, SERVICE_ERROR_KEY_REJECTED, key, value);
		return SERVICE_ERROR_KEY_REJECTED;			
	}

	return SERVICE_ERROR_NONE;
}


int service_add_extra_data_array(service_h service, const char *key, const char* value[], int length)
{
	if (service_valiate_service(service))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (service_validate_extra_data(key))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid key", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (service_validate_internal_key(key))
	{
		LOGE("[%s] KEY_REJECTED(0x%08x) : key(%s) rejected", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER, key);
		return SERVICE_ERROR_KEY_REJECTED;	
	}

	if (value == NULL || length <= 0)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid array", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}	

	if (appsvc_get_data_array(service->data, key, NULL) != NULL)
	{
		// overwrite any existing value
		bundle_del(service->data,key);
	}

	if (appsvc_add_data_array(service->data, key, value, length) != 0)
	{
		LOGE("[%s] KEY_REJECTED(0x%08x) : key(%s) is rejected from appsvc", __FUNCTION__, SERVICE_ERROR_KEY_REJECTED, key);
		return SERVICE_ERROR_KEY_REJECTED;			
	}

	return SERVICE_ERROR_NONE;
}


int service_remove_extra_data(service_h service, const char *key)
{
	if (service_valiate_service(service))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (service_validate_extra_data(key))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid key", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (service_validate_internal_key(key))
	{
		LOGE("[%s] KEY_REJECTED(0x%08x) : key(%s) rejected", __FUNCTION__, SERVICE_ERROR_KEY_REJECTED, key);
		return SERVICE_ERROR_KEY_REJECTED;
	}

	if (bundle_del(service->data, key))
	{
		LOGE("[%s] KEY_NOT_FOUND(0x%08x) : key(%s)", __FUNCTION__, SERVICE_ERROR_KEY_NOT_FOUND, key);
		return SERVICE_ERROR_KEY_NOT_FOUND;
	}

	return SERVICE_ERROR_NONE;
}


int service_get_extra_data(service_h service, const char *key, char **value)
{
	const char *data_value;

	if (service_valiate_service(service))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (service_validate_extra_data(key))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid key", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (service_validate_internal_key(key))
	{
		LOGE("[%s] KEY_REJECTED(0x%08x) : key(%s) rejected", __FUNCTION__, SERVICE_ERROR_KEY_REJECTED, key);
		return SERVICE_ERROR_KEY_REJECTED;
	}

	if (value == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid value", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	data_value = appsvc_get_data(service->data, key);

	if (data_value == NULL)	
	{
		if (errno == ENOTSUP)
		{
			LOGE("[%s] INVALID_DATA_TYPE(0x%08x) : key(%s)", __FUNCTION__, SERVICE_ERROR_INVALID_DATA_TYPE, key);
			return SERVICE_ERROR_INVALID_DATA_TYPE;
		}
		else
		{
			LOGE("[%s] KEY_NOT_FOUND(0x%08x) : key(%s)", __FUNCTION__, SERVICE_ERROR_INVALID_DATA_TYPE, key);
			return SERVICE_ERROR_KEY_NOT_FOUND;
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

	if (service_valiate_service(service))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (service_validate_extra_data(key))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid key", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (service_validate_internal_key(key))
	{
		LOGE("[%s] KEY_REJECTED(0x%08x) : key(%s) rejected", __FUNCTION__, SERVICE_ERROR_KEY_REJECTED, key);
		return SERVICE_ERROR_KEY_REJECTED;
	}

	if (value == NULL || length == 0)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid value", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	array_data = appsvc_get_data_array(service->data, key, &array_data_length);

	if (array_data == NULL)
	{
		if (errno == ENOTSUP)
		{
			LOGE("[%s] INVALID_DATA_TYPE(0x%08x) : key(%s)", __FUNCTION__, SERVICE_ERROR_INVALID_DATA_TYPE, key);
			return SERVICE_ERROR_INVALID_DATA_TYPE;
		}
		else
		{
			LOGE("[%s] KEY_NOT_FOUND(0x%08x) : key(%s)", __FUNCTION__, SERVICE_ERROR_KEY_NOT_FOUND, key);
			return SERVICE_ERROR_KEY_NOT_FOUND;
		}
	}

	array_data_clone = calloc(array_data_length, sizeof(char*));

	if (array_data_clone == NULL)
	{
		LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, SERVICE_ERROR_OUT_OF_MEMORY);
		return SERVICE_ERROR_OUT_OF_MEMORY;
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
	if (service_valiate_service(service))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (service_validate_extra_data(key))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid key", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (array == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid output param", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (!service_validate_internal_key(key))
	{
		if (!appsvc_data_is_array(service->data, key))
		{
			*array = false;
		}
		else
		{
			*array = true;
		}
	}
	else
	{
		*array = false;
	}

	return SERVICE_ERROR_NONE;
}


typedef struct {
	service_h service;
	service_extra_data_cb callback;
	void* user_data;
	bool* foreach_break;
} foreach_context_extra_data_t;

static void service_cb_broker_bundle_iterate(const char *key, const char *val, void *data)
{
	foreach_context_extra_data_t* foreach_context = NULL;
	service_extra_data_cb extra_data_cb;

	if (key == NULL || val == NULL || data == NULL)
	{
		return;
	}

	foreach_context = (foreach_context_extra_data_t*)data;

	if ( *(foreach_context->foreach_break) == true)
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
	
		*(foreach_context->foreach_break) = stop_foreach;
	}

}

int service_foreach_extra_data(service_h service, service_extra_data_cb callback, void *user_data)
{
	bool foreach_break = false;
	
	if (service_valiate_service(service))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (callback == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid callback", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	foreach_context_extra_data_t foreach_context = {
		.service = service,
		.callback = callback,
		.user_data = user_data,
		.foreach_break = &foreach_break
	};

	bundle_iterate(service->data, service_cb_broker_bundle_iterate, &foreach_context);

	return SERVICE_ERROR_NONE;
}

typedef struct {
	service_h service;
	service_app_matched_cb callback;
	void* user_data;
	bool* foreach_break;
} foreach_context_launchable_app_t;

int service_cb_broker_foreach_app_matched(const char *package, void *data)
{
	foreach_context_launchable_app_t *foreach_context;
	service_app_matched_cb app_matched_cb;

	if (package == NULL || data == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return -1;
	}

	foreach_context = (foreach_context_launchable_app_t*)data;

	if ( *(foreach_context->foreach_break) == true)
	{
		return -1;
	}

	app_matched_cb = foreach_context->callback;

	if (app_matched_cb != NULL)
	{
		bool stop_foreach = false;
		
		stop_foreach = !app_matched_cb(foreach_context->service, package, foreach_context->user_data);
	
		*(foreach_context->foreach_break) = stop_foreach;
	}

	return 0;
}

int service_foreach_app_matched(service_h service, service_app_matched_cb callback, void *user_data)
{
	bool foreach_break = false;

	if (service_valiate_service(service))
	{
		LOGE( "[%s] INVALID_PARAMETER(0x%08x) : invalid handle", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	if (callback == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid callback", __FUNCTION__, SERVICE_ERROR_INVALID_PARAMETER);
		return SERVICE_ERROR_INVALID_PARAMETER;
	}

	foreach_context_launchable_app_t foreach_context = {
		.service = service,
		.callback = callback,
		.user_data = user_data,
		.foreach_break = &foreach_break
	};

	appsvc_get_list(service->data, service_cb_broker_foreach_app_matched, &foreach_context);

	return SERVICE_ERROR_NONE;
}

