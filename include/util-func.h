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

#ifndef __UTIL_FUNC_H__
#define __UTIL_FUNC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sqlite3.h>

#ifndef EXPORT_API
#define EXPORT_API __attribute__ ((visibility("default")))
#endif

#define DB_UTIL_REGISTER_HOOK_METHOD    0x00000001
#define DB_UTIL_LUCENE_INDEX            0x00000002

/**
* @defgroup StorageFW Storage Framework
* In order to support applications using DB
*/

/**
 * @defgroup DB_UTIL
 * @ingroup StorageFW
 * @{
 */
EXPORT_API int db_util_open(const char *pszFilePath, sqlite3 **ppDB,
				int nOption);
EXPORT_API int db_util_open_with_options(const char *pszFilePath,
						sqlite3 **ppDB, int flags,
						const char *zVfs);
EXPORT_API int db_util_close(sqlite3 *ppDB);
/**
*@}
*/

#ifdef __cplusplus
}
#endif
#endif	/* __UTIL_FUNC_H__ */
