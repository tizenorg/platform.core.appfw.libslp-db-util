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

#include "util-func.h"
#include "collation.h"
#include "db-util-debug.h"

static int __db_util_busyhandler(void *pData, int count)
{
	if(5 - count > 0) {
		DB_UTIL_TRACE_DEBUG("Busy Handler Called! : PID(%d) / CNT(%d)\n", getpid(), count+1);
		usleep((count+1)*100000);
		return 1;
	} else {
		DB_UTIL_TRACE_DEBUG("Busy Handler will be returned SQLITE_BUSY error : PID(%d) \n", getpid());
		return 0;
	}
}

void __db_util_collation_cb(void* pArg, sqlite3* pDB, int eTextRep, const char* szName)
{
	if (eTextRep == SQLITE_UTF8 && !strcmp(szName, "localized"))
		db_util_create_collation(pDB, DB_UTIL_COL_LS_AS_CI,
								DB_UTIL_COL_UTF8, "localized");
}

static int __db_util_open(sqlite3 *pDB)
{
	int rc = 0;

	if(pDB == NULL) {
		DB_UTIL_TRACE_WARNING("Invalid input param error");
		return DB_UTIL_ERROR;
	}

	/* Register Busy handler */
	rc = sqlite3_busy_handler(pDB, __db_util_busyhandler, NULL);
	if (SQLITE_OK != rc) {
		DB_UTIL_TRACE_WARNING("Fail to register busy handler\n");
		sqlite3_close(pDB);
		return rc;
	}

#ifdef SET_PERSIST_JOURNAL_MODE
	/* Code to change default journal mode of sqlite3 is enabled so this option is disabled */
	/* Enable persist journal mode */
	rc = sqlite3_exec(pDB, "PRAGMA journal_mode = PERSIST",
			NULL, NULL, &pszErrorMsg);
	if (SQLITE_OK != rc) {
		DB_UTIL_TRACE_WARNING("Fail to change journal mode: %d, %d, %s, %s\n",
								sqlite3_errcode(pDB),
								sqlite3_extended_errcode(pDB),
								pszErrorMsg,
								sqlite3_errmsg(pDB));
		sqlite3_free(pszErrorMsg);
		sqlite3_close(pDB);
		return rc;
	}
#endif

	sqlite3_collation_needed(pDB, NULL, __db_util_collation_cb);

#if 0
	if (DB_UTIL_OK != rc) {
		DB_UTIL_TRACE_WARNING("Fail to create collation");
		return rc;
	}
#endif

	return DB_UTIL_OK;
}

int db_util_open(const char *pszFilePath, sqlite3 **ppDB, int nOption)
{
	if((pszFilePath == NULL) || (ppDB == NULL)) {
		DB_UTIL_TRACE_WARNING("Invalid input param error");
		return DB_UTIL_ERROR;
	}

	/* Open DB */
	int rc = sqlite3_open(pszFilePath, ppDB);
	if (SQLITE_OK != rc) {
		return rc;
	}

	rc = __db_util_open(*ppDB);

	return rc;
}

int db_util_open_with_options(const char *pszFilePath, sqlite3 **ppDB,
				int flags, const char *zVfs)
{
	if((pszFilePath == NULL) || (ppDB == NULL)) {
		DB_UTIL_TRACE_WARNING("sqlite3 handle null error");
		return DB_UTIL_ERROR;
	}

	/* Open DB */
	int rc = sqlite3_open_v2(pszFilePath, ppDB, flags, zVfs);
	if (SQLITE_OK != rc) {
		return rc;
	}

	rc = __db_util_open(*ppDB);

	return rc;
}

int db_util_close(sqlite3 *pDB)
{
	char *pszErrorMsg = NULL;

	/* Close DB */
	int rc = sqlite3_close(pDB);
	if (SQLITE_OK != rc) {
		DB_UTIL_TRACE_WARNING("Fail to change journal mode: %s\n", pszErrorMsg);
		sqlite3_free(pszErrorMsg);
		return rc;
	}

	return DB_UTIL_OK;
}


