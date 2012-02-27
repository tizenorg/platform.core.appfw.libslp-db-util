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

#ifndef __CONVERT_H__
#define __CONVERT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <db-util-common.h>
#include <stdio.h>

	typedef unsigned char DB_UINT8;
	typedef unsigned short DB_UINT16;
	typedef unsigned int DB_UINT32;
	typedef unsigned short DB_WCHAR;

/**
* @defgroup StorageFW Storage Framework
* In order to support applications using DB
*/

/**
 * @defgroup DB_UTIL
 * @ingroup StorageFW
 * @{
 */
EXPORT_API int dbutil_utf8toucs2(DB_UINT8 *source, DB_UINT16 *dest,
					 int *pdestsize);
EXPORT_API int dbutil_ucs2toutf8(DB_UINT16 *source, DB_UINT8 *dest,
					 int *pdestsize);
/**
*@}
*/

#ifdef __cplusplus
}
#endif
#endif	/*      __CONVERT_H__ */
