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

#include <string.h>
#include <aul.h>

char *app_get_data_path(void)
{
	const char *buf = aul_get_app_data_path();
	return buf != NULL ? strdup(buf) : NULL;
}

char *app_get_cache_path(void)
{
	const char *buf = aul_get_app_cache_path();
	return buf != NULL ? strdup(buf) : NULL;
}

char *app_get_resource_path(void)
{
	const char *buf = aul_get_app_resource_path();
	return buf != NULL ? strdup(buf) : NULL;
}

char *app_get_shared_data_path(void)
{
	const char *buf = aul_get_app_shared_data_path();
	return buf != NULL ? strdup(buf) : NULL;
}

char *app_get_shared_resource_path(void)
{
	const char *buf = aul_get_app_shared_resource_path();
	return buf != NULL ? strdup(buf) : NULL;
}

char *app_get_shared_trusted_path(void)
{
	const char *buf = aul_get_app_shared_trusted_path();
	return buf != NULL ? strdup(buf) : NULL;
}

char *app_get_external_data_path(void)
{
	const char *buf = aul_get_app_external_data_path();
	return buf != NULL ? strdup(buf) : NULL;
}

char *app_get_external_cache_path(void)
{
	const char *buf = aul_get_app_external_cache_path();
	return buf != NULL ? strdup(buf) : NULL;
}

char *app_get_external_shared_data_path(void)
{
	const char *buf = aul_get_app_external_shared_data_path();
	return buf != NULL ? strdup(buf) : NULL;
}
