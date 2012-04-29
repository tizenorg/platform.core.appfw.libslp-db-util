/*
 * libslp-db-util
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Hakjoo Ko <hakjoo.ko@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
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
 *
 */

#include <string.h>
#include <unistd.h>

#include "db-util-common.h"
#include "util-func.h"
#include "db-util-debug.h"

#define BUFSIZE		512
static char _szDBPath[BUFSIZE] = { 0, };

static int db_util_busyhandler(void *pData, int count)
{
	if(5 - count > 0) {
		DBUTIL_TRACE_DEBUG("Busy Handler Called! : PID(%d) / CNT(%d)\n", getpid(), count+1);	
		usleep((count+1)*100000);
		return 1;
	} else {
		DBUTIL_TRACE_DEBUG("Busy Handler will be returned SQLITE_BUSY error : PID(%d) \n", getpid());
		return 0;
	}
}

int db_util_open(const char *pszFilePath, sqlite3 **ppDB, int nOption)
{
	char *pszErrorMsg = NULL;

	if((pszFilePath == NULL) || (ppDB == NULL)) {
		DBUTIL_TRACE_WARNING("sqlite3 handle null error");
		return -1;
	}
	
	/* Save DB Path & Option */
	strncpy(_szDBPath, pszFilePath, strlen(pszFilePath));
	_szDBPath[strlen(pszFilePath)] = '\0';
	
	/* Open DB */
	int rc = sqlite3_open(pszFilePath, ppDB);
	if (SQLITE_OK != rc) {
		return rc;
	}
	
	/* Register Busy handler */
	rc = sqlite3_busy_handler(*ppDB, db_util_busyhandler, NULL);
	if (SQLITE_OK != rc) {
		printf("Fail to register busy handler\n");
		sqlite3_close(*ppDB);
		return rc;
	}

#if SET_PERSIST_JOURNAL_MODE
	/* Enable persist journal mode */
	rc = sqlite3_exec(*ppDB, "PRAGMA journal_mode = PERSIST", 
			NULL, NULL, &pszErrorMsg);
	if (SQLITE_OK != rc) {
		DBUTIL_TRACE_WARNING("Fail to change journal mode: %d, %d, %s, %s\n", sqlite3_errcode(*ppDB), sqlite3_extended_errcode(*ppDB), pszErrorMsg, sqlite3_errmsg(*ppDB));
		sqlite3_free(pszErrorMsg);
		sqlite3_close(*ppDB);
		return rc;
	}
#endif
	return rc;
}

int db_util_open_with_options(const char *pszFilePath, sqlite3 **ppDB,
				int flags, const char *zVfs)
{
	char *pszErrorMsg = NULL;

	if((pszFilePath == NULL) || (ppDB == NULL)) {
		DBUTIL_TRACE_WARNING("sqlite3 handle null error");
		return -1;
	}
	
	/* Save DB Path & Option */
	strncpy(_szDBPath, pszFilePath, strlen(pszFilePath));
	_szDBPath[strlen(pszFilePath)] = '\0';
	
	/* Open DB */
	int rc = sqlite3_open_v2(pszFilePath, ppDB, flags, zVfs);
	if (SQLITE_OK != rc) {
		return rc;
	}
	
	/* Register Busy handler */
	rc = sqlite3_busy_handler(*ppDB, db_util_busyhandler, NULL);
	if (SQLITE_OK != rc) {
		printf("Fail to register busy handler\n");
		sqlite3_close(*ppDB);
		return rc;
	}
	
#if SET_PERSIST_JOURNAL_MODE
	/* Enable persist journal mode */
	rc = sqlite3_exec(*ppDB, "PRAGMA journal_mode = PERSIST", 
			NULL, NULL, &pszErrorMsg);
	if (SQLITE_OK != rc) {
		DBUTIL_TRACE_WARNING("Fail to change journal mode: %s\n", pszErrorMsg);
		sqlite3_free(pszErrorMsg);
		sqlite3_close(*ppDB);
		return rc;
	}
#endif
	return rc;
}

int db_util_close(sqlite3 *ppDB)
{
	/* Close DB */
	int rc = sqlite3_close(ppDB);
	return rc;
}


