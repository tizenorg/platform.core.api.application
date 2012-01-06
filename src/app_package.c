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
#include <ail.h>
#include <dlog.h>

#include <app_private.h>
#include <app_service_private.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "TIZEN_N_APPLICATION"

int app_get_project_name(const char *package, char **name)
{
	char *name_token = NULL;

	if (package == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, APP_ERROR_INVALID_PARAMETER);
		return APP_ERROR_INVALID_PARAMETER;
	}

	// com.vendor.name -> name
	name_token = strrchr(package, '.');

	if (name_token == NULL)
	{
		LOGE("[%s] INVALID_CONTEXT(0x%08x)", __FUNCTION__, APP_ERROR_INVALID_CONTEXT);
		return APP_ERROR_INVALID_CONTEXT;
	}

	name_token++;

	*name = strdup(name_token);

	if (*name == NULL)
	{
		LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, APP_ERROR_OUT_OF_MEMORY);
		return APP_ERROR_OUT_OF_MEMORY;
	}

	return APP_ERROR_NONE;
}

int app_get_package(char **package)
{
	static char package_buf[TIZEN_PATH_MAX] = {0, };

	if (package == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, APP_ERROR_INVALID_PARAMETER);
		return APP_ERROR_INVALID_PARAMETER;
	}

	if (package_buf[0] == '\0')
	{
		aul_app_get_pkgname_bypid(getpid(), package_buf, sizeof(package_buf));
	}

	if (package_buf[0] == '\0')
	{
		LOGE("[%s] INVALID_CONTEXT(0x%08x) : failed to get the package of the application", __FUNCTION__, APP_ERROR_INVALID_CONTEXT);
		return APP_ERROR_INVALID_CONTEXT;
	}

	*package = strdup(package_buf);

	if (*package == NULL)
	{
		LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, APP_ERROR_OUT_OF_MEMORY);
		return APP_ERROR_OUT_OF_MEMORY;
	}

	return APP_ERROR_NONE;
}

static int app_get_appinfo(const char *package, ail_prop_str_e property, char **value)
{
	ail_appinfo_h appinfo;
	char *appinfo_value;
	char *appinfo_value_dup;

	if (ail_package_get_appinfo(package, &appinfo) != 0)
	{
		LOGE("[%s] INVALID_CONTEXT(0x%08x) : failed to get app-info", __FUNCTION__, APP_ERROR_INVALID_CONTEXT);
		return APP_ERROR_INVALID_CONTEXT;
	}
	
	if (ail_appinfo_get_str(appinfo, property, &appinfo_value) != 0)
	{
		ail_package_destroy_appinfo(appinfo);
		LOGE("[%s] INVALID_CONTEXT(0x%08x) : failed to get app-property", __FUNCTION__, APP_ERROR_INVALID_CONTEXT);
		return APP_ERROR_INVALID_CONTEXT;
	}

	appinfo_value_dup = strdup(appinfo_value);

	ail_package_destroy_appinfo(appinfo);

	if (appinfo_value_dup == NULL)
	{
		LOGE("[%s] OUT_OF_MEMORY(0x%08x)", __FUNCTION__, APP_ERROR_OUT_OF_MEMORY);
		return APP_ERROR_OUT_OF_MEMORY;
	}

	*value = appinfo_value_dup;
	
	return APP_ERROR_NONE;
}

int app_get_name(char **name)
{
	char *package = NULL;
	int retval;

	if(name == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, APP_ERROR_INVALID_PARAMETER);
		return APP_ERROR_INVALID_PARAMETER;
	}

	if (app_get_package(&package) != 0)
	{
		LOGE("[%s] INVALID_CONTEXT(0x%08x)", __FUNCTION__, APP_ERROR_INVALID_CONTEXT);
		return APP_ERROR_INVALID_CONTEXT;
	}

	retval = app_get_appinfo(package, AIL_PROP_NAME_STR, name);

	if (package != NULL)
	{
		free(package);
	}

	return retval;
}

int app_get_version(char **version)
{
	char *package;
	int retval;

	if(version == NULL)
	{
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, APP_ERROR_INVALID_PARAMETER);
		return APP_ERROR_INVALID_PARAMETER;
	}

	if (app_get_package(&package) != 0)
	{
		LOGE("[%s] INVALID_CONTEXT(0x%08x)", __FUNCTION__, APP_ERROR_INVALID_CONTEXT);
		return APP_ERROR_INVALID_CONTEXT;
	}

	retval = app_get_appinfo(package, AIL_PROP_VERSION_STR, version);

	if (package != NULL)
	{
		free(package);
	}

	return retval;	
}

