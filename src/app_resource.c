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

static char* app_get_resource_directory()
{
	static char *resource_directory = NULL;

	if (resource_directory == NULL)
	{
		char *resource_directory_tmp;
		char *package = NULL;
	
		if (app_get_package(&package) != APP_ERROR_NONE)
		{
			LOGE("[%s] INVALID_CONTEXT(0x%08x)", __FUNCTION__, APP_ERROR_INVALID_CONTEXT);
			return NULL;
		}

		resource_directory_tmp = calloc(1, sizeof(char) * TIZEN_PATH_MAX);

		if (resource_directory_tmp == NULL)
		{
			LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, APP_ERROR_OUT_OF_MEMORY);
			return NULL;
		}

		snprintf(resource_directory_tmp, TIZEN_PATH_MAX, PATH_FMT_RES_DIR, package);

		if (package != NULL)
		{
			free(package);
		}

		resource_directory = resource_directory_tmp;
	}

	return resource_directory;
}

char* app_get_resource(const char *resource, char *buffer, int size)
{
	char *resource_directory;
	int abs_path_size;

	if (resource == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid resource", __FUNCTION__, APP_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	if (buffer == NULL || size <= 0)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid buffer", __FUNCTION__, APP_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	resource_directory = app_get_resource_directory();

	if (resource_directory == NULL)
	{
		LOGE("[%s] INVALID_CONTEXT(0x%08x) : failed to get the path to the resource directory", __FUNCTION__, APP_ERROR_INVALID_CONTEXT);
		return NULL;
	}

	abs_path_size = strlen(resource_directory)+ strlen("/") + strlen(resource);

	if (size <= abs_path_size)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : the buffer is not big enough", __FUNCTION__, APP_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	snprintf(buffer, size, "%s/%s", resource_directory, resource);

	return buffer;
}

char* app_get_data_directory(char *buffer, int size)
{
	static char *abs_path = NULL;

	if (buffer == NULL || size <= 0)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : invalid buffer", __FUNCTION__, APP_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	if (abs_path == NULL)
	{
		char *package;
		char *abs_path_tmp;

		if (app_get_package(&package) != 0)
		{
			LOGE("[%s] INVALID_CONTEXT(0x%08x)", __FUNCTION__, APP_ERROR_INVALID_CONTEXT);
			return NULL;
		}

		abs_path_tmp = calloc(1, sizeof(char) * TIZEN_PATH_MAX);

		if (abs_path_tmp == NULL)
		{
			LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, APP_ERROR_OUT_OF_MEMORY);
			return NULL;
		}

		snprintf(abs_path_tmp, TIZEN_PATH_MAX, PATH_FMT_DATA_DIR, package);

		if (package != NULL)
		{
			free(package);
		}

		abs_path = abs_path_tmp;
	}

	if (abs_path == NULL)
	{
		LOGE("[%s] INVALID_CONTEXT(0x%08x) : failed to get the absolute path to the data directory", __FUNCTION__, APP_ERROR_INVALID_CONTEXT);
		return NULL;
	}

	if (size <= strlen(abs_path))
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x) : the buffer is not big enough", __FUNCTION__, APP_ERROR_INVALID_PARAMETER);
		return NULL;
	}

	snprintf(buffer, size, "%s", abs_path);

	return buffer;
}

