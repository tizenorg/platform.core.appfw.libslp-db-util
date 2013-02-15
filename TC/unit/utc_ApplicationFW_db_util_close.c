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

#include <tet_api.h>
#include <db-util.h>

static void startup(void);
static void cleanup(void);

void (*tet_startup)(void) = startup;
void (*tet_cleanup)(void) = cleanup;

static void utc_ApplicationFW_db_util_close_func_01(void);
static void utc_ApplicationFW_db_util_close_func_02(void);

static sqlite3* db_hd = NULL;
char* db_path = "/opt/test.db";

enum {
	POSITIVE_TC_IDX = 0x01,
	NEGATIVE_TC_IDX,
};

struct tet_testlist tet_testlist[] = {
	{ utc_ApplicationFW_db_util_close_func_01, POSITIVE_TC_IDX },
	{ utc_ApplicationFW_db_util_close_func_02, NEGATIVE_TC_IDX },
	{ NULL, 0 }
};

static void startup(void)
{
}

static void cleanup(void)
{
}

/**
 * @brief Positive test case of db_util_close()
 */
static void utc_ApplicationFW_db_util_close_func_01(void)
{
	int r = 0;

	r = db_util_open(db_path, &db_hd, 0);
	if (r != SQLITE_OK) {
		tet_infoline("db_util_close() failed in positive test case");
		tet_result(TET_FAIL);
		return;
	}

	r = db_util_close(db_hd);
	if (r != SQLITE_OK) {
		tet_infoline("db_util_close() failed in positive test case");
		tet_result(TET_FAIL);
		return;
	}
	db_hd = NULL;

	tet_result(TET_PASS);
}

/**
 * @brief Negative test case of ug_init db_util_close()
 */
static void utc_ApplicationFW_db_util_close_func_02(void)
{
	int r = 0;
	sqlite3_stmt *db_stmt = NULL;

	db_hd = NULL;

	r = db_util_open(db_path, &db_hd, 0);
	if (r != SQLITE_OK) {
		tet_infoline("db_util_close() failed in negative test case");
		tet_result(TET_FAIL);
		return;
	}

	r = sqlite3_prepare(db_hd, "create table test(id int);", -1, &db_stmt, NULL);
	if (r != SQLITE_OK) {
		tet_infoline("db_util_close() failed in negative test case");
		tet_result(TET_FAIL);
		return;
	}

	r = db_util_close(db_hd);
	if (r == SQLITE_OK) {
		tet_infoline("db_util_close() failed in negative test case");
		tet_result(TET_FAIL);
		return;
	}

	tet_result(TET_PASS);
}

