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

#define LOG_TAG "CAPI_APPFW_APPLICATION"

int app_get_package_app_name(const char *appid, char **name)
{
	char *name_token = NULL;

	if (appid == NULL)
	{
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	// com.vendor.name -> name
	name_token = strrchr(appid, '.');

	if (name_token == NULL)
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, NULL);
	}

	name_token++;

	*name = strdup(name_token);

	if (*name == NULL)
	{
		return app_error(APP_ERROR_OUT_OF_MEMORY, __FUNCTION__, NULL);
	}

	return APP_ERROR_NONE;
}

int app_get_package(char **package)
{
	return app_get_id(package);
}

int app_get_id(char **id)
{
	static char id_buf[TIZEN_PATH_MAX] = {0, };
	int ret = -1;

	if (id == NULL)
	{
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (id_buf[0] == '\0')
	{
		ret = aul_app_get_appid_bypid(getpid(), id_buf, sizeof(id_buf));
		if (ret < 0) {
			return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "failed to get the application ID");
		}
	}

	if (id_buf[0] == '\0')
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "failed to get the application ID");
	}

	*id = strdup(id_buf);

	if (*id == NULL)
	{
		return app_error(APP_ERROR_OUT_OF_MEMORY, __FUNCTION__, NULL);
	}

	return APP_ERROR_NONE;
}

static int app_get_appinfo(const char *package, const char *property, char **value)
{
	ail_appinfo_h appinfo;
	char *appinfo_value;
	char *appinfo_value_dup;

	if (ail_get_appinfo(package, &appinfo) != 0)
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "failed to get app-info");
	}
	
	if (ail_appinfo_get_str(appinfo, property, &appinfo_value) != 0)
	{
		ail_destroy_appinfo(appinfo);
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "failed to get app-property");
	}

	appinfo_value_dup = strdup(appinfo_value);

	ail_destroy_appinfo(appinfo);

	if (appinfo_value_dup == NULL)
	{
		return app_error(APP_ERROR_OUT_OF_MEMORY, __FUNCTION__, NULL);
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
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (app_get_id(&package) != 0)
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "failed to get the package");
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
		return app_error(APP_ERROR_INVALID_PARAMETER, __FUNCTION__, NULL);
	}

	if (app_get_id(&package) != 0)
	{
		return app_error(APP_ERROR_INVALID_CONTEXT, __FUNCTION__, "failed to get the package");
	}

	retval = app_get_appinfo(package, AIL_PROP_VERSION_STR, version);

	if (package != NULL)
	{
		free(package);
	}

	return retval;	
}

