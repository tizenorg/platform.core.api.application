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
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <ctype.h>
#include <string.h>
#include <execinfo.h>
#include <glib.h>

#include <app_preference.h>
#include <app_preference_internal.h>
#include <app_common.h>

#include <sys/syscall.h>

#ifdef PREFERENCE_TIMECHECK
#include <sys/time.h>
#endif

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#define PREFERENCE_ERROR_RETRY_CNT 7
#define PREFERENCE_ERROR_RETRY_SLEEP_UTIME 10000

#define DELIMITER 29

static int g_posix_errno;
static int g_preference_errno;
static char *g_pref_dir_path = NULL;

enum preference_op_t {
    PREFERENCE_OP_GET = 0,
    PREFERENCE_OP_SET = 1
};

#ifdef PREFERENCE_TIMECHECK
double correction, startT;

double set_start_time(void)
{
	struct timeval tv;
	double curtime;

	gettimeofday(&tv, NULL);
	curtime = tv.tv_sec * 1000 + (double)tv.tv_usec / 1000;
	return curtime;
}

double exec_time(double start)
{
	double end = set_start_time();
	return (end - start - correction);
}

int init_time(void)
{
	double temp_t;
	temp_t = set_start_time();
	correction = exec_time(temp_t);

	return 0;
}
#endif

char* _preference_get_pref_dir_path()
{
	char *app_data_path = NULL;

	if (!g_pref_dir_path) {
		g_pref_dir_path = (char *)malloc(PREFERENCE_KEY_PATH_LEN + 1);

		if ((app_data_path = app_get_data_path()) == NULL) {
			ERR("IO_ERROR(0x%08x) : fail to get data directory", PREFERENCE_ERROR_IO_ERROR);
			free(g_pref_dir_path);
			g_pref_dir_path = NULL;
			return NULL;
		}

		snprintf(g_pref_dir_path, PREFERENCE_KEY_PATH_LEN, "%s%s", app_data_path, PREF_DIR);
		INFO("pref_dir_path: %s", g_pref_dir_path);
		free(app_data_path);
	}
	return g_pref_dir_path;
}

int _preference_keynode_set_keyname(keynode_t *keynode, const char *keyname)
{
	if (keynode->keyname) free(keynode->keyname);
	keynode->keyname = strndup(keyname, PREFERENCE_KEY_PATH_LEN);
	retvm_if(keynode->keyname == NULL, PREFERENCE_ERROR_IO_ERROR, "strndup Fails");
	return PREFERENCE_ERROR_NONE;
}

static inline void _preference_keynode_set_value_int(keynode_t *keynode, const int value)
{
	keynode->type = PREFERENCE_TYPE_INT;
	keynode->value.i = value;
}

static inline void _preference_keynode_set_value_boolean(keynode_t *keynode, const int value)
{
	keynode->type = PREFERENCE_TYPE_BOOLEAN;
	keynode->value.b = !!value;
}

static inline void _preference_keynode_set_value_double(keynode_t *keynode, const double value)
{
	keynode->type = PREFERENCE_TYPE_DOUBLE;
	keynode->value.d = value;
}

static inline void _preference_keynode_set_value_string(keynode_t *keynode, const char *value)
{
	keynode->type = PREFERENCE_TYPE_STRING;
	keynode->value.s = strdup(value);
}

inline keynode_t *_preference_keynode_new(void)
{
	keynode_t *keynode;
	keynode = calloc(1, sizeof(keynode_t));

	return keynode;
}

inline void _preference_keynode_free(keynode_t *keynode)
{
	if(keynode) {
		if (keynode->keyname)
			free(keynode->keyname);
		if (keynode->type == PREFERENCE_TYPE_STRING && keynode->value.s)
			free(keynode->value.s);
		free(keynode);
	}
}

int _preference_get_key_name(const char *path, char **keyname)
{
	int read_size = 0;
	size_t keyname_len = 0;
	char *convert_key = NULL;
	FILE *fp = NULL;

	if( (fp = fopen(path, "r")) == NULL ) {
		return PREFERENCE_ERROR_FILE_OPEN;
	}

	read_size = fread((void *)&keyname_len, sizeof(int), 1, fp);
	if (read_size <= 0 || keyname_len > PREFERENCE_KEY_PATH_LEN) {
		fclose(fp);
		return PREFERENCE_ERROR_FILE_FREAD;
	}

	convert_key = (char *)calloc(1, keyname_len+1);
	if (convert_key == NULL) {
		LOGE("memory alloc failed");
		fclose(fp);
		return PREFERENCE_ERROR_OUT_OF_MEMORY;
	}

	read_size = fread((void *)convert_key, keyname_len, 1, fp);
	if (read_size <= 0) {
		free(convert_key);
		fclose(fp);
		return PREFERENCE_ERROR_FILE_FREAD;
	}

	*keyname = convert_key;

	fclose(fp);

	return PREFERENCE_ERROR_NONE;
}

int _preference_get_key_path(keynode_t *keynode, char *path)
{
	const char *key = NULL;
	char *pref_dir_path = NULL;
	gchar *convert_key;
	char *keyname = keynode->keyname;

	if(!keyname) {
		LOGE("keyname is null");
		return PREFERENCE_ERROR_WRONG_PREFIX;
	}

	pref_dir_path = _preference_get_pref_dir_path();
	if (!pref_dir_path) {
		LOGE("_preference_get_pref_dir_path() failed.");
		return PREFERENCE_ERROR_IO_ERROR;
	}

	convert_key = g_compute_checksum_for_string(G_CHECKSUM_SHA1,
							keyname,
							strlen(keyname));
	if (convert_key == NULL) {
		LOGE("fail to convert");
		return PREFERENCE_ERROR_IO_ERROR;
	}

	key = (const char*)convert_key;

	snprintf(path, PATH_MAX-1, "%s%s", pref_dir_path, key);

	g_free(convert_key);

	return PREFERENCE_ERROR_NONE;
}

static int _preference_set_key_check_pref_dir()
{
	char *pref_dir_path = NULL;
	mode_t dir_mode = 0664 | 0111;

	pref_dir_path = _preference_get_pref_dir_path();
	if (!pref_dir_path) {
		LOGE("_preference_get_pref_dir_path() failed.");
		return PREFERENCE_ERROR_IO_ERROR;
	}

	if (access(pref_dir_path, F_OK) < 0) {
		if (mkdir(pref_dir_path, dir_mode) < 0) {
			char err_buf[ERR_LEN] = {0,};
			strerror_r(errno, err_buf, sizeof(err_buf));
			ERR("mkdir() failed(%d/%s)", errno, err_buf);
			return PREFERENCE_ERROR_IO_ERROR;
		}
	}

	return PREFERENCE_ERROR_NONE;
}

static int _preference_set_key_creation(const char* path)
{
	int fd;
	mode_t temp;
	temp = umask(0000);
	fd = open(path, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	umask(temp);

	if(fd == -1) {
		char err_buf[ERR_LEN] = {0,};
		strerror_r(errno, err_buf, sizeof(err_buf));
		ERR("open(rdwr,create) error: %d(%s)", errno, err_buf);
		return PREFERENCE_ERROR_IO_ERROR;
	}
	close(fd);

	return PREFERENCE_ERROR_NONE;
}

static int _preference_set_file_lock(int fd, short type)
{
	struct flock l;

	l.l_type = type;
	l.l_start= 0;		/*Start at begin*/
	l.l_whence = SEEK_SET;
	l.l_len = 0;		/*Do it with whole file*/

	return fcntl(fd, F_SETLK, &l);
}

static int _preference_get_pid_of_file_lock_owner(int fd, short type)
{
	struct flock l;

	l.l_type = type;
	l.l_start= 0;		/*Start at begin*/
	l.l_whence = SEEK_SET;
	l.l_len = 0;		/*Do it with whole file*/

	if(fcntl(fd, F_GETLK, &l) < 0) {
		WARN("error in getting lock info");
		return -1;
	}

	if(l.l_type == F_UNLCK)
		return 0;
	else
		return l.l_pid;
}


static int _preference_set_read_lock(int fd)
{
	return _preference_set_file_lock(fd, F_RDLCK);
}

static int _preference_set_write_lock(int fd)
{
	return _preference_set_file_lock(fd, F_WRLCK);
}

static int _preference_set_unlock(int fd)
{
	return _preference_set_file_lock(fd, F_UNLCK);
}

static void _preference_log_subject_label(void)
{
	int fd;
	int ret;
	char smack_label[256] = {0,};
	char curren_path[256] = {0,};
	int tid;
	char err_buf[ERR_LEN] = {0,};

	tid = (int)syscall(SYS_gettid);
	snprintf(curren_path, sizeof(curren_path)-1, "/proc/%d/attr/current", tid);
	fd = open(curren_path, O_RDONLY);
	if (fd < 0) {
		strerror_r(errno, err_buf, sizeof(err_buf));
		LOGE("fail to open self current attr (err: %s)", err_buf);
		return;
	}

	ret = read(fd, smack_label, sizeof(smack_label)-1);
	if (ret < 0) {
		close(fd);
		strerror_r(errno, err_buf, sizeof(err_buf));
		LOGE("fail to open self current attr (err: %s)", err_buf);
		return;
	}

	ERR("current(%d) subject label : %s", tid, smack_label);

	close(fd);
}

static int _preference_check_retry_err(keynode_t *keynode, int preference_errno, int io_errno, int op_type)
{
	int is_busy_err = 0;

	if (preference_errno == PREFERENCE_ERROR_FILE_OPEN)
	{
		switch (io_errno)
		{
			case ENOENT :
			{
				if(op_type == PREFERENCE_OP_SET)
				{
					int rc = 0;
					char path[PATH_MAX] = {0,};
					rc = _preference_get_key_path(keynode, path);
					if (rc != PREFERENCE_ERROR_NONE) {
						ERR("_preference_get_key_path error");
						_preference_log_subject_label();
						break;
					}

					rc = _preference_set_key_check_pref_dir();
					if (rc != PREFERENCE_ERROR_NONE) {
						ERR("_preference_set_key_check_pref_dir() failed.");
						_preference_log_subject_label();
						break;
					}

					rc = _preference_set_key_creation(path);
					if (rc != PREFERENCE_ERROR_NONE) {
						ERR("_preference_set_key_creation error : %s", path);
						_preference_log_subject_label();
						break;
					}
					INFO("%s key is created", keynode->keyname);

					is_busy_err = 1;
				}
				break;
			}
			case EACCES :
			{
				_preference_log_subject_label();
				break;
			}
			case EAGAIN :
			case EMFILE :
			case ENFILE :
			case ETXTBSY :
			{
				is_busy_err = 1;
			}
		}
	}
	else if (preference_errno == PREFERENCE_ERROR_FILE_CHMOD)
	{
		switch (io_errno)
		{
			case EINTR :
			case EBADF :
			{
				is_busy_err = 1;
			}
		}
	}
	else if (preference_errno == PREFERENCE_ERROR_FILE_LOCK)
	{
		switch (io_errno)
		{
			case EBADF :
			case EAGAIN :
			case ENOLCK :
			{
				is_busy_err = 1;
			}
		}
	}
	else if (preference_errno == PREFERENCE_ERROR_FILE_WRITE)
	{
		switch (io_errno)
		{
			case 0 :
			case EAGAIN :
			case EINTR :
			case EIO :
			case ENOMEM :
			{
				is_busy_err = 1;
			}
		}
	}
	else if (preference_errno == PREFERENCE_ERROR_FILE_FREAD)
	{
		switch (io_errno)
		{
			case EAGAIN :
			case EINTR :
			case EIO :
			{
				is_busy_err = 1;
			}
		}
	}
	else
	{
		is_busy_err = 0;
	}

	if (is_busy_err == 1) {
		return 1;
	}
	else
	{
		char err_buf[ERR_LEN] = {0,};
		strerror_r(errno, err_buf, sizeof(err_buf));
		ERR("key(%s), check retry err: %d/(%d/%s).",keynode->keyname, preference_errno, io_errno, err_buf);
		return 0;
	}
}

static int _preference_set_key_filesys(keynode_t *keynode, int *io_errno)
{
	char path[PATH_MAX] = {0,};
	FILE *fp = NULL;
	int ret = -1;
	int func_ret = PREFERENCE_ERROR_NONE;
	int err_no = 0;
	char err_buf[100] = { 0, };
	int is_write_error = 0;
	int retry_cnt = 0;
	size_t keyname_len = 0;

retry_open :
	errno = 0;
	err_no = 0;
	func_ret = PREFERENCE_ERROR_NONE;

	ret = _preference_get_key_path(keynode, path);
	retv_if(ret != PREFERENCE_ERROR_NONE, ret);

	if( (fp = fopen(path, "r+")) == NULL ) {
		func_ret = PREFERENCE_ERROR_FILE_OPEN;
		err_no = errno;
		goto out_return;
	}

retry :
	errno = 0;
	err_no = 0;
	func_ret = PREFERENCE_ERROR_NONE;

	ret = _preference_set_write_lock(fileno(fp));
	if (ret == -1) {
		func_ret = PREFERENCE_ERROR_FILE_LOCK;
		err_no = errno;
		ERR("file(%s) lock owner(%d)",
			keynode->keyname,
			_preference_get_pid_of_file_lock_owner(fileno(fp), F_WRLCK));
		goto out_return;
	}

	/* write keyname and size */
	keyname_len = strlen(keynode->keyname);

	ret = fwrite((void *)&keyname_len, sizeof(int), 1, fp);
	if (ret <= 0) {
		if (!errno) {
			LOGW("number of written items is 0. try again");
			errno = EAGAIN;
		}
		err_no = errno;
		func_ret = PREFERENCE_ERROR_FILE_WRITE;
		goto out_unlock;
	}

	ret = fwrite((void *)keynode->keyname, keyname_len, 1, fp);
	if (ret <= 0) {
		if (!errno) {
			LOGW("number of written items is 0. try again");
			errno = EAGAIN;
		}
		err_no = errno;
		func_ret = PREFERENCE_ERROR_FILE_WRITE;
		goto out_unlock;
	}

	/* write key type */
	ret = fwrite((void *)&(keynode->type), sizeof(int), 1, fp);
	if (ret <= 0) {
		if (!errno) {
			LOGW("number of written items is 0. try again");
			errno = EAGAIN;
		}
		err_no = errno;
		func_ret = PREFERENCE_ERROR_FILE_WRITE;
		goto out_unlock;
	}

	/* write key value */
	switch (keynode->type) {
	case PREFERENCE_TYPE_INT:
		ret = fwrite((void *)&(keynode->value.i), sizeof(int), 1, fp);
		if (ret <= 0) is_write_error = 1;
		break;
	case PREFERENCE_TYPE_DOUBLE:
		ret = fwrite((void *)&(keynode->value.d), sizeof(double), 1, fp);
		if (ret <= 0) is_write_error = 1;
		break;
	case PREFERENCE_TYPE_BOOLEAN:
		ret = fwrite((void *)&(keynode->value.b), sizeof(int), 1, fp);
		if (ret <= 0) is_write_error = 1;
		break;
	case PREFERENCE_TYPE_STRING:
		ret = fprintf(fp,"%s",keynode->value.s);
		if (ret < strlen(keynode->value.s)) is_write_error = 1;
		if (ftruncate(fileno(fp), ret) == -1)
			is_write_error = 1;
		break;
	default :
		func_ret = PREFERENCE_ERROR_WRONG_TYPE;
		goto out_unlock;
	}

	if (is_write_error) {
		if (!errno) {
			LOGW("number of written items is 0. try again");
			errno = EAGAIN;
		}
		err_no = errno;
		func_ret = PREFERENCE_ERROR_FILE_WRITE;
		goto out_unlock;
	}

	fflush(fp);

out_unlock :
	ret = _preference_set_unlock(fileno(fp));
	if (ret == -1) {
		func_ret = PREFERENCE_ERROR_FILE_LOCK;
		err_no = errno;
		goto out_return;
	}

out_return :
	if (func_ret != PREFERENCE_ERROR_NONE) {
		strerror_r(err_no, err_buf, 100);
		if (_preference_check_retry_err(keynode, func_ret, err_no, PREFERENCE_OP_SET)) {
			if (retry_cnt < PREFERENCE_ERROR_RETRY_CNT) {
				WARN("_preference_set_key_filesys(%d-%s) step(%d) failed(%d / %s) retry(%d)", keynode->type, keynode->keyname, func_ret, err_no, err_buf, retry_cnt);
				retry_cnt++;
				usleep((retry_cnt)*PREFERENCE_ERROR_RETRY_SLEEP_UTIME);

				if (fp)
					goto retry;
				else
					goto retry_open;
			} else {
				ERR("_preference_set_key_filesys(%d-%s) step(%d) faild(%d / %s) over the retry count.",
					keynode->type, keynode->keyname, func_ret, err_no, err_buf);
			}
		} else {
			ERR("_preference_set_key_filesys(%d-%s) step(%d) failed(%d / %s)\n", keynode->type, keynode->keyname, func_ret, err_no, err_buf);
		}
	} else {
		if(retry_cnt > 0) {
			DBG("_preference_set_key_filesys ok with retry cnt(%d)", retry_cnt);
		}
	}

	if (fp) {
		if (func_ret == PREFERENCE_ERROR_NONE) {
			ret = fdatasync(fileno(fp));
			if (ret == -1) {
				err_no = errno;
				func_ret = PREFERENCE_ERROR_FILE_SYNC;
			}
		}
		fclose(fp);
	}
	*io_errno = err_no;

	return func_ret;
}

static int _preference_set_key(keynode_t *keynode)
{
	int ret = 0;
	int io_errno = 0;
	char err_buf[100] = { 0, };

	ret = _preference_set_key_filesys(keynode, &io_errno);
	if (ret == PREFERENCE_ERROR_NONE) {
		g_posix_errno = PREFERENCE_ERROR_NONE;
		g_preference_errno = PREFERENCE_ERROR_NONE;
	} else {
		strerror_r(io_errno, err_buf, 100);
		ERR("_preference_set_key(%s) step(%d) failed(%d / %s)", keynode->keyname, ret, io_errno, err_buf);
		g_posix_errno = io_errno;
		g_preference_errno = ret;
	}

	return ret;
}


/*
 * This function set the integer value of given key
 * @param[in]	key	key
 * @param[in]	intval integer value to set
 * @return 0 on success, -1 on error
 */
API int preference_set_int(const char *key, int intval)
{
	START_TIME_CHECK

	retvm_if(key == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: key is NULL");

	int func_ret = PREFERENCE_ERROR_NONE;

	keynode_t* pKeyNode = _preference_keynode_new();
	retvm_if(pKeyNode == NULL, PREFERENCE_ERROR_OUT_OF_MEMORY, "key malloc fail");

	func_ret = _preference_keynode_set_keyname(pKeyNode, key);
	if (func_ret != PREFERENCE_ERROR_NONE) {
		ERR("set key name error");
		_preference_keynode_free(pKeyNode);
		return PREFERENCE_ERROR_IO_ERROR;
	}
	_preference_keynode_set_value_int(pKeyNode, intval);

	if (_preference_set_key(pKeyNode) != PREFERENCE_ERROR_NONE) {
		ERR("preference_set_int(%d) : key(%s/%d) error", getpid(), key, intval);
		func_ret = PREFERENCE_ERROR_IO_ERROR;
	} else {
		INFO("%s(%d) success", key, intval);
	}

	_preference_keynode_free(pKeyNode);

	END_TIME_CHECK

	return func_ret;
}

/*
* This function set the boolean value of given key
* @param[in]	key	key
* @param[in]	boolval boolean value to set
		(Integer value 1 is 'True', and 0 is 'False')
* @return 0 on success, -1 on error
*/
API int preference_set_boolean(const char *key, bool boolval)
{
	START_TIME_CHECK

	retvm_if(key == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: key is NULL");

	int func_ret = PREFERENCE_ERROR_NONE;
	keynode_t* pKeyNode = _preference_keynode_new();
	retvm_if(pKeyNode == NULL, PREFERENCE_ERROR_OUT_OF_MEMORY, "key malloc fail");

	func_ret = _preference_keynode_set_keyname(pKeyNode, key);
	if (func_ret != PREFERENCE_ERROR_NONE) {
		ERR("set key name error");
		_preference_keynode_free(pKeyNode);
		return PREFERENCE_ERROR_IO_ERROR;
	}
	_preference_keynode_set_value_boolean(pKeyNode, boolval);

	if (_preference_set_key(pKeyNode) != PREFERENCE_ERROR_NONE) {
		ERR("preference_set_boolean(%d) : key(%s/%d) error", getpid(), key, boolval);
		func_ret = PREFERENCE_ERROR_IO_ERROR;
	} else {
		INFO("%s(%d) success", key, boolval);
	}

	_preference_keynode_free(pKeyNode);

	END_TIME_CHECK

	return func_ret;
}

/*
 * This function set the double value of given key
 * @param[in]	key	key
 * @param[in]	dblval double value to set
 * @return 0 on success, -1 on error
 */
API int preference_set_double(const char *key, double dblval)
{
	START_TIME_CHECK

	retvm_if(key == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: key is NULL");

	int func_ret = PREFERENCE_ERROR_NONE;
	keynode_t* pKeyNode = _preference_keynode_new();
	retvm_if(pKeyNode == NULL, PREFERENCE_ERROR_OUT_OF_MEMORY, "key malloc fail");

	func_ret = _preference_keynode_set_keyname(pKeyNode, key);
	if (func_ret != PREFERENCE_ERROR_NONE) {
		ERR("set key name error");
		_preference_keynode_free(pKeyNode);
		return PREFERENCE_ERROR_IO_ERROR;
	}
	_preference_keynode_set_value_double(pKeyNode, dblval);

	if (_preference_set_key(pKeyNode) != PREFERENCE_ERROR_NONE) {
		ERR("preference_set_double(%d) : key(%s/%f) error", getpid(), key, dblval);
		func_ret = PREFERENCE_ERROR_IO_ERROR;
	} else {
		INFO("%s(%f) success", key, dblval);
	}

	_preference_keynode_free(pKeyNode);

	END_TIME_CHECK

	return func_ret;
}

/*
 * This function set the string value of given key
 * @param[in]	key	key
 * @param[in]	strval string value to set
 * @return 0 on success, -1 on error
 */
API int preference_set_string(const char *key, const char *strval)
{
	START_TIME_CHECK

	retvm_if(key == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: key is NULL");
	retvm_if(strval == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: value is NULL");

	int func_ret = PREFERENCE_ERROR_NONE;
	keynode_t* pKeyNode = _preference_keynode_new();
	retvm_if(pKeyNode == NULL, PREFERENCE_ERROR_OUT_OF_MEMORY, "key malloc fail");

	func_ret = _preference_keynode_set_keyname(pKeyNode, key);
	if (func_ret != PREFERENCE_ERROR_NONE) {
		ERR("set key name error");
		_preference_keynode_free(pKeyNode);
		return PREFERENCE_ERROR_IO_ERROR;
	}
	_preference_keynode_set_value_string(pKeyNode, strval);

	if (_preference_set_key(pKeyNode) != PREFERENCE_ERROR_NONE) {
		ERR("preference_set_string(%d) : key(%s/%s) error", getpid(), key, strval);
		func_ret = PREFERENCE_ERROR_IO_ERROR;
	} else {
		INFO("%s(%s) success", key, strval);
	}

	_preference_keynode_free(pKeyNode);

	END_TIME_CHECK

	return func_ret;
}

static int _preference_get_key_filesys(keynode_t *keynode, int* io_errno)
{
	char path[PATH_MAX] = {0,};
	int ret = -1;
	int func_ret = PREFERENCE_ERROR_NONE;
	char err_buf[100] = { 0, };
	int err_no = 0;
	int type = 0;
	FILE *fp = NULL;
	int retry_cnt = 0;
	int read_size = 0;
	size_t keyname_len = 0;

retry_open :
	errno = 0;
	func_ret = PREFERENCE_ERROR_NONE;

	ret = _preference_get_key_path(keynode, path);
	retv_if(ret != PREFERENCE_ERROR_NONE, ret);

	if( (fp = fopen(path, "r")) == NULL ) {
		func_ret = PREFERENCE_ERROR_FILE_OPEN;
		err_no = errno;
		goto out_return;
	}

retry :
	err_no = 0;
	func_ret = PREFERENCE_ERROR_NONE;

	ret = _preference_set_read_lock(fileno(fp));
	if (ret == -1) {
		func_ret = PREFERENCE_ERROR_FILE_LOCK;
		err_no = errno;
		goto out_return;
	}

	read_size = fread((void *)&keyname_len, sizeof(int), 1, fp);
	if ((read_size <= 0) || (read_size > sizeof(int))) {
		if(!ferror(fp)) {
			errno = ENODATA;
		}
		err_no = errno;
		func_ret = PREFERENCE_ERROR_FILE_FREAD;
		goto out_unlock;
	}

	ret = fseek(fp, keyname_len, SEEK_CUR);
	if (ret) {
		if(!ferror(fp)) {
			errno = ENODATA;
		}
		err_no = errno;
		func_ret = PREFERENCE_ERROR_FILE_FREAD;
		goto out_unlock;
	}

	read_size = fread((void *)&type, sizeof(int), 1, fp);
	if (read_size <= 0) {
		if(!ferror(fp)) {
			errno = ENODATA;
		}
		err_no = errno;
		func_ret = PREFERENCE_ERROR_FILE_FREAD;
		goto out_unlock;
	}

	/* read data value */
	switch(type)
	{
		case PREFERENCE_TYPE_INT:
		{
			int value_int = 0;
			read_size = fread((void*)&value_int, sizeof(int), 1, fp);
			if ((read_size <= 0) || (read_size > sizeof(int))) {
				if (!ferror(fp)) {
					LOGW("number of read items for value is wrong. err : %d", errno);
				}
				err_no = errno;
				func_ret = PREFERENCE_ERROR_FILE_FREAD;
				goto out_unlock;
			} else {
				_preference_keynode_set_value_int(keynode, value_int);
			}

			break;
		}
		case PREFERENCE_TYPE_DOUBLE:
		{
			double value_dbl = 0;
			read_size = fread((void*)&value_dbl, sizeof(double), 1, fp);
			if ((read_size <= 0) || (read_size > sizeof(double))) {
				if (!ferror(fp)) {
					LOGW("number of read items for value is wrong. err : %d", errno);
				}
				err_no = errno;
				func_ret = PREFERENCE_ERROR_FILE_FREAD;
				goto out_unlock;
			} else {
				_preference_keynode_set_value_double(keynode, value_dbl);
			}

			break;
		}
		case PREFERENCE_TYPE_BOOLEAN:
		{
			int value_int = 0;
			read_size = fread((void*)&value_int, sizeof(int), 1, fp);
			if ((read_size <= 0) || (read_size > sizeof(int))) {
				if (!ferror(fp)) {
					LOGW("number of read items for value is wrong. err : %d", errno);
				}
				err_no = errno;
				func_ret = PREFERENCE_ERROR_FILE_FREAD;
				goto out_unlock;
			} else {
				_preference_keynode_set_value_boolean(keynode, value_int);
			}

			break;
		}
		case PREFERENCE_TYPE_STRING:
		{
			char file_buf[BUF_LEN] = {0,};
			char *value = NULL;
			int value_size = 0;

			while(fgets(file_buf, sizeof(file_buf), fp))
			{
				if (value) {
					value_size = value_size + strlen(file_buf);
					value = (char *) realloc(value, value_size);
					if (value == NULL) {
						func_ret = PREFERENCE_ERROR_OUT_OF_MEMORY;
						break;
					}
					strncat(value, file_buf, strlen(file_buf));
				} else {
					value_size = strlen(file_buf) + 1;
					value = (char *)malloc(value_size);
					if (value == NULL) {
						func_ret = PREFERENCE_ERROR_OUT_OF_MEMORY;
						break;
					}
					memset(value, 0x00, value_size);
					strncpy(value, file_buf, strlen(file_buf));
				}
			}

			if (ferror(fp)) {
				err_no = errno;
				func_ret = PREFERENCE_ERROR_FILE_FGETS;
			} else {
				if (value) {
					_preference_keynode_set_value_string(keynode, value);
				} else {
					_preference_keynode_set_value_string(keynode, "");
				}
			}
			if (value)
				free(value);

			break;
		}
		default :
			func_ret = PREFERENCE_ERROR_WRONG_TYPE;
	}

out_unlock :
	ret = _preference_set_unlock(fileno(fp));
	if (ret == -1) {
		func_ret = PREFERENCE_ERROR_FILE_LOCK;
		err_no = errno;
		goto out_return;
	}


out_return :
	if (func_ret != PREFERENCE_ERROR_NONE) {
		strerror_r(err_no, err_buf, 100);

		if (_preference_check_retry_err(keynode, func_ret, err_no, PREFERENCE_OP_GET)) {
			if (retry_cnt < PREFERENCE_ERROR_RETRY_CNT) {
				retry_cnt++;
				usleep((retry_cnt)*PREFERENCE_ERROR_RETRY_SLEEP_UTIME);

				if (fp)
					goto retry;
				else
					goto retry_open;
			}
			else {
				ERR("_preference_get_key_filesys(%s) step(%d) faild(%d / %s) over the retry count.",
					keynode->keyname, func_ret, err_no, err_buf);
			}
		}
	}

	if (fp)
		fclose(fp);

	*io_errno = err_no;

	return func_ret;
}

int _preference_get_key(keynode_t *keynode)
{
	int ret = 0;
	int io_errno = 0;
	char err_buf[100] = {0,};

	ret = _preference_get_key_filesys(keynode, &io_errno);
	if (ret == PREFERENCE_ERROR_NONE) {
		g_posix_errno = PREFERENCE_ERROR_NONE;
		g_preference_errno = PREFERENCE_ERROR_NONE;
	}
	else {
		if (io_errno == ENOENT)
			ret = PREFERENCE_ERROR_NO_KEY;
		else
			ret = PREFERENCE_ERROR_IO_ERROR;

		strerror_r(io_errno, err_buf, 100);
		ERR("_preference_get_key(%s) step(%d) failed(%d / %s)\n", keynode->keyname, ret, io_errno, err_buf);
		g_posix_errno = io_errno;
		g_preference_errno = ret;
	}

	return ret;
}


/*
 * This function get the integer value of given key
 * @param[in]	key	key
 * @param[out]	intval output buffer
 * @return 0 on success, -1 on error
 */
API int preference_get_int(const char *key, int *intval)
{
	START_TIME_CHECK

	retvm_if(key == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: key is null");
	retvm_if(intval == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: output buffer is null");

	int func_ret = PREFERENCE_ERROR_IO_ERROR;
	keynode_t* pKeyNode = _preference_keynode_new();
	retvm_if(pKeyNode == NULL, PREFERENCE_ERROR_OUT_OF_MEMORY, "key malloc fail");

	func_ret = _preference_keynode_set_keyname(pKeyNode, key);
	if (func_ret != PREFERENCE_ERROR_NONE) {
		ERR("set key name error");
		_preference_keynode_free(pKeyNode);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	func_ret = _preference_get_key(pKeyNode);
	if (func_ret != PREFERENCE_ERROR_NONE) {
		ERR("preference_get_int(%d) : key(%s) error", getpid(), key);
	} else {
		*intval = pKeyNode->value.i;
		if (pKeyNode->type == PREFERENCE_TYPE_INT) {
			INFO("%s(%d) success", key, *intval);
			func_ret = PREFERENCE_ERROR_NONE;
		} else {
			ERR("The type(%d) of keynode(%s) is not INT", pKeyNode->type, pKeyNode->keyname);
			func_ret = PREFERENCE_ERROR_INVALID_PARAMETER;
		}
	}

	_preference_keynode_free(pKeyNode);

	END_TIME_CHECK

	return func_ret;
}

/*
 * This function get the boolean value of given key
 * @param[in]	key	key
 * @param[out]	boolval output buffer
 * @return 0 on success, -1 on error
 */
API int preference_get_boolean(const char *key, bool *boolval)
{
	START_TIME_CHECK

	retvm_if(key == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: key is null");
	retvm_if(boolval == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: output buffer is null");

	int func_ret = PREFERENCE_ERROR_IO_ERROR;
	keynode_t* pKeyNode = _preference_keynode_new();
	retvm_if(pKeyNode == NULL, PREFERENCE_ERROR_OUT_OF_MEMORY, "key malloc fail");

	func_ret = _preference_keynode_set_keyname(pKeyNode, key);
	if (func_ret != PREFERENCE_ERROR_NONE) {
		ERR("set key name error");
		_preference_keynode_free(pKeyNode);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	func_ret = _preference_get_key(pKeyNode);

	if (func_ret != PREFERENCE_ERROR_NONE) {
		ERR("preference_get_boolean(%d) : %s error", getpid(), key);
	} else {
		*boolval = !!(pKeyNode->value.b);
		if (pKeyNode->type == PREFERENCE_TYPE_BOOLEAN) {
			INFO("%s(%d) success", key, *boolval);
			func_ret = PREFERENCE_ERROR_NONE;
		} else {
			ERR("The type(%d) of keynode(%s) is not BOOL", pKeyNode->type, pKeyNode->keyname);
			func_ret = PREFERENCE_ERROR_INVALID_PARAMETER;
		}
	}

	_preference_keynode_free(pKeyNode);

	END_TIME_CHECK

	return func_ret;
}

/*
 * This function get the double value of given key
 * @param[in]	key	key
 * @param[out]	dblval output buffer
 * @return 0 on success, -1 on error
 */
API int preference_get_double(const char *key, double *dblval)
{
	START_TIME_CHECK

	retvm_if(key == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: key is null");
	retvm_if(dblval == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: output buffer is null");

	int func_ret = PREFERENCE_ERROR_IO_ERROR;
	keynode_t* pKeyNode = _preference_keynode_new();
	retvm_if(pKeyNode == NULL, PREFERENCE_ERROR_OUT_OF_MEMORY, "key malloc fail");

	func_ret = _preference_keynode_set_keyname(pKeyNode, key);
	if (func_ret != PREFERENCE_ERROR_NONE) {
		ERR("set key name error");
		_preference_keynode_free(pKeyNode);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	func_ret = _preference_get_key(pKeyNode);

	if (func_ret != PREFERENCE_ERROR_NONE) {
		ERR("preference_get_double(%d) : %s error", getpid(), key);
	} else {
		*dblval = pKeyNode->value.d;
		if (pKeyNode->type == PREFERENCE_TYPE_DOUBLE) {
			INFO("%s(%f) success", key, *dblval);
			func_ret = PREFERENCE_ERROR_NONE;
		} else {
			ERR("The type(%d) of keynode(%s) is not DBL", pKeyNode->type, pKeyNode->keyname);
			func_ret = PREFERENCE_ERROR_INVALID_PARAMETER;
		}
	}

	_preference_keynode_free(pKeyNode);

	END_TIME_CHECK

	return func_ret;
}

/*
 * This function get the string value of given key
 * @param[in]	key	key
 * @param[out]	value output buffer
 * @return 0 on success, -1 on error
 */
API int preference_get_string(const char *key, char **value)
{
	START_TIME_CHECK

	retvm_if(key == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: key is null");
	retvm_if(value == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: output buffer is null");

	int func_ret = PREFERENCE_ERROR_IO_ERROR;
	keynode_t* pKeyNode = _preference_keynode_new();
	retvm_if(pKeyNode == NULL, PREFERENCE_ERROR_OUT_OF_MEMORY, "key malloc fail");

	func_ret = _preference_keynode_set_keyname(pKeyNode, key);
	if (func_ret != PREFERENCE_ERROR_NONE) {
		ERR("set key name error");
		_preference_keynode_free(pKeyNode);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	char *tempstr = NULL;
	func_ret = _preference_get_key(pKeyNode);

	if (func_ret != PREFERENCE_ERROR_NONE) {
		ERR("preference_get_string(%d) : %s error", getpid(), key);
	} else {
		if (pKeyNode->type == PREFERENCE_TYPE_STRING)
			tempstr = pKeyNode->value.s;
		else {
			ERR("The type(%d) of keynode(%s) is not STR", pKeyNode->type, pKeyNode->keyname);
			func_ret = PREFERENCE_ERROR_INVALID_PARAMETER;
		}

		if(tempstr) {
			*value = strdup(tempstr);
			INFO("%s(%s) success", key, value);
		}
	}

	_preference_keynode_free(pKeyNode);

	END_TIME_CHECK

	return func_ret;
}

/*
 * This function unset given key
 * @param[in]	key	key
 * @return 0 on success, -1 on error
 */
API int preference_remove(const char *key)
{
	START_TIME_CHECK

	char path[PATH_MAX] = {0,};
	int ret = -1;
	int err_retry = PREFERENCE_ERROR_RETRY_CNT;
	int func_ret = PREFERENCE_ERROR_NONE;

	retvm_if(key == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: key is null");

	keynode_t* pKeyNode = _preference_keynode_new();
	retvm_if(pKeyNode == NULL, PREFERENCE_ERROR_OUT_OF_MEMORY, "key malloc fail");

	ret = _preference_keynode_set_keyname(pKeyNode, key);
	if (ret != PREFERENCE_ERROR_NONE) {
		ERR("set key name error");
		_preference_keynode_free(pKeyNode);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	ret = _preference_get_key_path(pKeyNode, path);
	if (ret != PREFERENCE_ERROR_NONE) {
		ERR("Invalid argument: key is not valid");
		_preference_keynode_free(pKeyNode);
		return PREFERENCE_ERROR_INVALID_PARAMETER;
	}

	if (access(path, F_OK) == -1) {
		ERR("Error : key(%s) is not exist", key);
		_preference_keynode_free(pKeyNode);
		return PREFERENCE_ERROR_NO_KEY;
	}

	do {
		ret = remove(path);
		if(ret == -1) {
			char err_buf[ERR_LEN] = {0,};
			strerror_r(errno, err_buf, sizeof(err_buf));
			ERR("preference_remove() failed. ret=%d(%s), key(%s)", errno, err_buf, key);
			func_ret = PREFERENCE_ERROR_IO_ERROR;
		} else {
			func_ret = PREFERENCE_ERROR_NONE;
			break;
		}
	} while(err_retry--);

	END_TIME_CHECK;

	_preference_keynode_free(pKeyNode);

	return func_ret;
}

API int preference_remove_all(void)
{
	START_TIME_CHECK

	int ret = -1;
	int err_retry = PREFERENCE_ERROR_RETRY_CNT;
	int func_ret = PREFERENCE_ERROR_NONE;
	DIR *dir;
	struct dirent dent;
	struct dirent *result = NULL;
	char *pref_dir_path = NULL;
	char err_buf[ERR_LEN] = {0,};

	pref_dir_path = _preference_get_pref_dir_path();
	if (!pref_dir_path)
	{
		LOGE("_preference_get_pref_dir_path() failed.");
		return PREFERENCE_ERROR_IO_ERROR;
	}

	dir = opendir(pref_dir_path);
	if (dir == NULL)
	{
		strerror_r(errno, err_buf, sizeof(err_buf));
		LOGE("opendir() failed. pref_path: %s, error: %d(%s)", pref_dir_path, errno, err_buf);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	keynode_t* pKeyNode = _preference_keynode_new();
	if (pKeyNode == NULL)
	{
		ERR("key malloc fail");
		closedir(dir);
		return PREFERENCE_ERROR_OUT_OF_MEMORY;
	}

	while (readdir_r(dir, &dent, &result) == 0 && result != NULL)
	{
		const char *entry = dent.d_name;
		char *keyname = NULL;
		char path[PATH_MAX] = {0,};

		if (entry[0] == '.') {
			continue;
		}

		snprintf(path, PATH_MAX-1, "%s%s", pref_dir_path, entry);

		ret = _preference_get_key_name(path, &keyname);
		if (ret != PREFERENCE_ERROR_NONE) {
			ERR("_preference_get_key_name() failed(%d)", ret);
			_preference_keynode_free(pKeyNode);
			closedir(dir);
			return PREFERENCE_ERROR_IO_ERROR;
		}

		ret = preference_unset_changed_cb(keyname);
		if (ret != PREFERENCE_ERROR_NONE) {
			ERR("preference_unset_changed_cb() failed(%d)", ret);
			_preference_keynode_free(pKeyNode);
			closedir(dir);
			free(keyname);
			return PREFERENCE_ERROR_IO_ERROR;
		}

		do {
			ret = remove(path);
			if (ret == -1) {
				strerror_r(errno, err_buf, sizeof(err_buf));
				ERR("preference_remove_all error: %d(%s)", errno, err_buf);
				func_ret = PREFERENCE_ERROR_IO_ERROR;
			} else {
				func_ret = PREFERENCE_ERROR_NONE;
				break;
			}
		} while(err_retry--);

		free(keyname);
	}

	_preference_keynode_free(pKeyNode);
	closedir(dir);

	END_TIME_CHECK

	return func_ret;
}

int preference_is_existing(const char *key, bool *exist)
{
	START_TIME_CHECK

	char path[PATH_MAX] = {0,};
	int ret = -1;
	int func_ret = PREFERENCE_ERROR_NONE;

	retvm_if(key == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: key is null");
	retvm_if(exist == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: key is null");

	keynode_t* pKeyNode = _preference_keynode_new();
	retvm_if(pKeyNode == NULL, PREFERENCE_ERROR_OUT_OF_MEMORY, "key malloc fail");

	ret = _preference_keynode_set_keyname(pKeyNode, key);
	if (ret != PREFERENCE_ERROR_NONE) {
		ERR("set key name error");
		_preference_keynode_free(pKeyNode);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	ret = _preference_get_key_path(pKeyNode, path);
	if (ret != PREFERENCE_ERROR_NONE) {
		_preference_keynode_free(pKeyNode);
		return ret;
	}

	ret = access(path, F_OK);
	if (ret == -1) {
		ERR("Error : key(%s) is not exist", key);
		*exist = 0;
	} else {
		*exist = 1;
	}

	_preference_keynode_free(pKeyNode);

	END_TIME_CHECK

	return func_ret;
}


API int preference_set_changed_cb(const char *key, preference_changed_cb callback, void *user_data)
{
	START_TIME_CHECK

	retvm_if(key == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: key is null");
	retvm_if(callback == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: cb(%p)", callback);

	int func_ret = PREFERENCE_ERROR_IO_ERROR;

	keynode_t* pKeyNode = _preference_keynode_new();
	retvm_if(pKeyNode == NULL, PREFERENCE_ERROR_OUT_OF_MEMORY, "key malloc fail");

	func_ret = _preference_keynode_set_keyname(pKeyNode, key);
	if (func_ret != PREFERENCE_ERROR_NONE) {
		ERR("set key name error");
		_preference_keynode_free(pKeyNode);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	if (_preference_kdb_add_notify(pKeyNode, callback, user_data)) {
		if (errno == ENOENT) {
			LOGE("NO_KEY(0x%08x) : fail to find given key(%s)", PREFERENCE_ERROR_NO_KEY, key);
			_preference_keynode_free(pKeyNode);
			return PREFERENCE_ERROR_NO_KEY;
		} else if(errno != 0) {
			ERR("preference_notify_key_changed : key(%s) add notify fail", key);
			_preference_keynode_free(pKeyNode);
			return PREFERENCE_ERROR_IO_ERROR;
		}
	}
	INFO("%s noti is added", key);
	_preference_keynode_free(pKeyNode);

	END_TIME_CHECK

	return PREFERENCE_ERROR_NONE;
}

API int preference_unset_changed_cb(const char *key)
{
	START_TIME_CHECK

	int func_ret = PREFERENCE_ERROR_IO_ERROR;

	retvm_if(key == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: key is null");

	keynode_t* pKeyNode = _preference_keynode_new();
	retvm_if(pKeyNode == NULL, PREFERENCE_ERROR_OUT_OF_MEMORY, "key malloc fail");

	func_ret = _preference_keynode_set_keyname(pKeyNode, key);
	if (func_ret != PREFERENCE_ERROR_NONE) {
		ERR("set key name error");
		_preference_keynode_free(pKeyNode);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	if (_preference_kdb_del_notify(pKeyNode)) {
		if (errno == ENOENT) {
			LOGE("NO_KEY(0x%08x) : fail to find given key(%s)", PREFERENCE_ERROR_NO_KEY, key);
			_preference_keynode_free(pKeyNode);
			return PREFERENCE_ERROR_NO_KEY;
		} else if (errno != 0) {
			char err_buf[ERR_LEN] = {0,};
			strerror_r(errno, err_buf, sizeof(err_buf));
			ERR("preference_unset_changed_cb() failed: key(%s) error(%d/%s)", key, errno, err_buf);
			_preference_keynode_free(pKeyNode);
			return PREFERENCE_ERROR_IO_ERROR;
		}
	}
	INFO("%s noti removed", key);
	_preference_keynode_free(pKeyNode);

	END_TIME_CHECK

	return PREFERENCE_ERROR_NONE;
}


API int preference_foreach_item(preference_item_cb callback, void *user_data)
{
	START_TIME_CHECK

	retvm_if(callback == NULL, PREFERENCE_ERROR_INVALID_PARAMETER, "Invalid argument: cb(%p)", callback);

	int ret = 0;
	DIR *dir;
	struct dirent dent;
	struct dirent *result = NULL;
	char *pref_dir_path = NULL;
	char err_buf[ERR_LEN] = {0,};

	pref_dir_path = _preference_get_pref_dir_path();
	if (!pref_dir_path) {
		LOGE("_preference_get_pref_dir_path() failed.");
		return PREFERENCE_ERROR_IO_ERROR;
	}

	dir = opendir(pref_dir_path);
	if (dir == NULL) {
		strerror_r(errno, err_buf, sizeof(err_buf));
		LOGE("opendir() failed. path: %s, error: %d(%s)", pref_dir_path, errno, err_buf);
		return PREFERENCE_ERROR_IO_ERROR;
	}

	while(readdir_r(dir, &dent, &result) == 0 && result != NULL) {
		const char *entry = dent.d_name;
		char *keyname = NULL;
		char path[PATH_MAX] = {0,};

		if (entry[0] == '.') {
			continue;
		}

		snprintf(path, PATH_MAX-1, "%s%s", pref_dir_path, entry);

		ret = _preference_get_key_name(path, &keyname);
		if (ret != PREFERENCE_ERROR_NONE) {
			ERR("_preference_get_key_name() failed(%d)", ret);
			closedir(dir);
			return PREFERENCE_ERROR_IO_ERROR;
		}

		callback(keyname, user_data);
		free(keyname);
	}

	closedir(dir);
	END_TIME_CHECK

	return PREFERENCE_ERROR_NONE;
}
