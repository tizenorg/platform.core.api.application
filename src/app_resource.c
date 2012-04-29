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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <bundle.h>
#include <appcore-common.h>
#include <aul.h>
#include <dlog.h>

#include <app_private.h>
#include <app_service_private.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "TIZEN_N_APPLICATION"

static const char *INSTALLED_PATH = "/opt/apps";
static const char *RES_DIRECTORY_NAME = "res";
static const char *DATA_DIRECTORY_NAME = "data";

static char * app_get_root_directory(char *buffer, int size)
{
	char *package = NULL;
	char root_directory[TIZEN_PATH_MAX] = {0, };

	if (app_get_package(&package) != APP_ERROR_NONE)
	{
		app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "failed to get the package");
		return NULL;
	}

	snprintf(root_directory, sizeof(root_directory), "%s/%s", INSTALLED_PATH, package);

	free(package);	

	if (size < strlen(root_directory)+1)
	{
		app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "the buffer is not big enough");
		return NULL;
	}

	snprintf(buffer, size, "%s", root_directory);

	return buffer;
}

static char* app_get_resource_directory(char *buffer, int size)
{
	char root_directory[TIZEN_PATH_MAX] = {0, };
	char resource_directory[TIZEN_PATH_MAX] = {0, };

	if (app_get_root_directory(root_directory, sizeof(root_directory)) == NULL)
	{
		app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "failed to get the root directory of the application");
		return NULL;
	}

	snprintf(resource_directory, sizeof(resource_directory), "%s/%s", root_directory, RES_DIRECTORY_NAME);

	if (size < strlen(resource_directory) +1)
	{
		app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "the buffer is not big enough");
		return NULL;
	}

	snprintf(buffer, size, "%s", resource_directory);

	return buffer;
}

char* app_get_data_directory(char *buffer, int size)
{
	static char data_directory[TIZEN_PATH_MAX] = {0, };
	static int data_directory_length = 0;

	if (data_directory[0] == '\0')
	{
		char *root_directory = NULL;

		root_directory = calloc(1, TIZEN_PATH_MAX);

		if (root_directory == NULL)
		{
			app_error(APP_ERROR_OUT_OF_MEMORY, __FUNCTION__, NULL);
			return NULL;
		}

		if (app_get_root_directory(root_directory, TIZEN_PATH_MAX) == NULL)
		{
			free(root_directory);
			app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "failed to get the path to the root directory");
			return NULL;
		}

		snprintf(data_directory, sizeof(data_directory), "%s/%s", root_directory, DATA_DIRECTORY_NAME);

		data_directory_length = strlen(data_directory);

		free(root_directory);
	}

	if (size < data_directory_length+1)
	{
		app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "the buffer is not big enough");
		return NULL;
	}

	snprintf(buffer, size, "%s", data_directory);

	return buffer;
}

char* app_get_resource(const char *resource, char *buffer, int size)
{
	static char resource_directory[TIZEN_PATH_MAX] = {0, };
	static int resource_directory_length = 0;

	int resource_path_length = 0;

	if (resource == NULL)
	{
		app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
		return NULL;
	}

	if (buffer == NULL || size <= 0)
	{
		app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
		return NULL;
	}

	if (resource_directory[0] == '\0')
	{
		if (app_get_resource_directory(resource_directory, sizeof(resource_directory)) == NULL)
		{
			app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "failed to get the path to the resource directory");
			return NULL;
		}

		resource_directory_length = strlen(resource_directory);
	}

	resource_path_length = resource_directory_length + strlen("/") + strlen(resource);

	if (size < resource_path_length+1)
	{
		app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, "the buffer is not big enough");
		return NULL;
	}

	snprintf(buffer, size, "%s/%s", resource_directory, resource);

	return buffer;
}

