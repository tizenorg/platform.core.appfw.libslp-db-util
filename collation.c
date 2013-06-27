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
#include <math.h>
#include <wchar.h>
#include <wctype.h>

#include <unistd.h>
#include <glib.h>

#include <dlfcn.h>

#include <unicode/utypes.h>
#include <unicode/ucol.h>
#include <unicode/uiter.h>
#include <unicode/ustring.h>

#include "collation.h"

#include "db-util-debug.h"

#define DB_UTIL_RETV_IF(cond,ret) \
			do {\
				if(cond) {\
					DB_UTIL_TRACE_WARNING("\x1b[33m[%s()][RET_IF]\x1b[0m",__FUNCTION__);\
					return ret;\
				}\
			}while(0)

#define DB_UTIL_RET_IF(cond) \
			do {\
				if(cond) {\
					DB_UTIL_TRACE_WARNING("\x1b[33m[%s()][RET_IF]\x1b[0m",__FUNCTION__);\
					return;\
				}\
			}while(0)

#define DB_UTIL_ERR_COL_FUNC_RET DB_UTIL_ERROR

enum {
	DB_UTIL_ERR_DLOPEN = -10,
	DB_UTIL_ERR_DLSYM,
	DB_UTIL_ERR_ENV,
	DB_UTIL_ERR_ICU,
	DB_UTIL_ERR_PARAM
};

enum {
	DB_UTIL_CMP_SC,
	/* Loop comparison */
	DB_UTIL_CMP_LC
};

#define ICU_FUNC_CNT 8

typedef UCollator* (*ICU_UCOL_OPEN)(const char *, UErrorCode *);
typedef void (*ICU_UCOL_CLOSE)(UCollator *);
typedef void (*ICU_UCOL_SETSTRENGTH)(UCollator *, UCollationStrength);
typedef UCollationResult (*ICU_UCOL_STRCOLL)(const UCollator *, const UChar *, int32_t, const UChar *, int32_t);
typedef UCollationResult (*ICU_UCOL_STRCOLLITER)(const UCollator *, UCharIterator *, UCharIterator *, UErrorCode *);
typedef void (*ICU_UITER_SETUTF8)(UCharIterator *, const char *, int32_t);
typedef void (*ICU_ULOC_SETDEFAULT)(const char* localeID, UErrorCode* status);
typedef const char* (*ICU_ULOC_GETDEFAULT)(void);

typedef struct {
	ICU_UCOL_OPEN icu_ucol_open;
	ICU_UCOL_CLOSE icu_ucol_close;
	ICU_UCOL_STRCOLL icu_ucol_strcoll;
	ICU_UCOL_STRCOLLITER icu_ucol_strcollIter;
	ICU_UCOL_SETSTRENGTH icu_ucol_setStrength;
	ICU_UITER_SETUTF8 icu_uiter_setUTF8;
	ICU_ULOC_SETDEFAULT icu_uloc_setDefault;
	ICU_ULOC_GETDEFAULT icu_uloc_getDefault;
} db_util_icu_func_t;

db_util_icu_func_t icu_symbol;

void *g_dl_icu_handle = NULL;

#ifdef DB_UTIL_ENABLE_DEVDEBUG
static char *strtoutf8(const UChar * unichars, int len);
#endif

static int __db_util_dl_load_icu()
{
	void *handle = NULL;
	void *icu_handle[ICU_FUNC_CNT] = { 0 };
	char *dl_error;
	int i = 0;

	const char *ICU_API[] = {
		"ucol_open",
		"ucol_close",
		"ucol_strcoll",
		"ucol_strcollIter",
		"ucol_setStrength",
		"uiter_setUTF8",
		"uloc_setDefault",
		"uloc_getDefault"
	};

	if(g_dl_icu_handle == NULL) {
		g_dl_icu_handle = dlopen("libicui18n.so", RTLD_LAZY | RTLD_GLOBAL);
		if(g_dl_icu_handle == NULL) {
			DB_UTIL_TRACE_WARNING("dlopen icu so fail");
			return DB_UTIL_ERR_DLOPEN;
		}
	}

	for (i = 0; i < ICU_FUNC_CNT; i++) {
		handle = dlsym(g_dl_icu_handle, ICU_API[i]);
		if ((dl_error = dlerror()) != NULL)  {
			DB_UTIL_TRACE_WARNING("dlsym(%s) is failed for %s",
								dl_error, ICU_API[i]);
			return DB_UTIL_ERR_DLSYM;
		}
		icu_handle[i] = handle;
	}

	memcpy((void*)&icu_symbol, (const void*)icu_handle, sizeof(icu_handle));

	return DB_UTIL_OK;
}

/* The collating function must return an integer that is negative, zero or positive */
static int __db_util_collate_icu_16(void *ucol, int str1_len, const void *str1, int str2_len, const void *str2)
{
#ifdef DB_UTIL_ENABLE_DEVDEBUG
	DB_UTIL_TRACE_DEBUG("__db_util_collate_icu_16 func start \n");

	UChar* tmp_v1 = (UChar *)str1;
	UChar* tmp_v2 = (UChar *)str2;
	char* utf8_v1 = strtoutf8(tmp_v1, str1_len);
	char* utf8_v2 = strtoutf8(tmp_v2, str2_len);

	DB_UTIL_TRACE_DEBUG("v1(%d) : %s\n", str1_len, utf8_v1);
	DB_UTIL_TRACE_DEBUG("v2(%d) : %s\n", str2_len, utf8_v2);
#endif

	DB_UTIL_RETV_IF(icu_symbol.icu_ucol_strcoll == NULL, DB_UTIL_ERR_COL_FUNC_RET);

	UCollationResult result = icu_symbol.icu_ucol_strcoll(
								(UCollator *) ucol,
								(const UChar *) str1, str1_len,
								(const UChar *) str2, str2_len);

#ifdef DB_UTIL_ENABLE_DEVDEBUG
	if(utf8_v1)
		free(utf8_v1);
	if(utf8_v2)
		free(utf8_v2);

	if (result == UCOL_LESS) {
		DB_UTIL_TRACE_DEBUG("less \n");
	} else if (result == UCOL_GREATER) {
		DB_UTIL_TRACE_DEBUG("greater \n");
	} else {
		DB_UTIL_TRACE_DEBUG("equal \n");
	}
#endif

	return result;
}

/* The collating function must return an integer that is negative, zero or positive */
static int __db_util_collate_icu_16_lc(void *ucol, int str1_len, const void *str1, int str2_len, const void *str2)
{
#ifdef DB_UTIL_ENABLE_DEVDEBUG
	DB_UTIL_TRACE_DEBUG("__db_util_collate_icu_16_lc func start \n");

	UChar* tmp_v1 = (UChar *)str1;
	UChar* tmp_v2 = (UChar *)str2;

	char* utf8_v1 = strtoutf8(tmp_v1, str1_len);
	char* utf8_v2 = strtoutf8(tmp_v2, str2_len);

	DB_UTIL_TRACE_DEBUG("v1(%d) : %s\n", str1_len, utf8_v1);
	DB_UTIL_TRACE_DEBUG("v2(%d) : %s\n", str2_len, utf8_v2);

	if(utf8_v1)
		free(utf8_v1);
	if(utf8_v2)
		free(utf8_v2);
#endif

	UCollationResult result = 0;

	UChar* str_to = (UChar *)str1;
	UChar* str_from = (UChar *)str1;

	int i;

	DB_UTIL_RETV_IF(icu_symbol.icu_ucol_strcoll == NULL, DB_UTIL_ERR_COL_FUNC_RET);

	if(str1_len> str2_len) {
		for(i=0;i<str2_len;i=i+2) {
			str_to++;
		}

		while((int)(str_to-(UChar*)str1) <= str1_len) {
			result = icu_symbol.icu_ucol_strcoll(
						(UCollator *) ucol,
						(UChar *) str_from, str_to-str_from,
						(const UChar *) str2, str2_len);

			if (result == UCOL_EQUAL)
			{
#ifdef DB_UTIL_ENABLE_DEVDEBUG
				DB_UTIL_TRACE_DEBUG("equal \n");
#endif
				return UCOL_EQUAL;
			}

			str_to++;
			str_from++;
		}
	} else {
		result = icu_symbol.icu_ucol_strcoll(
					(UCollator *) ucol,
					(const UChar *) str1, str1_len,
					(const UChar *) str2, str2_len);
	}

#ifdef DB_UTIL_ENABLE_DEVDEBUG
	if (result == UCOL_LESS) {
		DB_UTIL_TRACE_DEBUG("less \n");
	} else if (result == UCOL_GREATER) {
		DB_UTIL_TRACE_DEBUG("greater \n");
	} else if (result == UCOL_EQUAL) {
		DB_UTIL_TRACE_DEBUG("equal \n");
	} else {
		DB_UTIL_TRACE_DEBUG("compare error : %d \n", result);
	}
#endif

	return result;
}

/* The collating function must return an integer that is negative, zero or positive */
static int __db_util_collate_icu_8(void *ucol, int str1_len, const void *str1, int str2_len, const void *str2)
{
	UCharIterator uiter1, uiter2;
	UErrorCode error = U_ZERO_ERROR;

#ifdef DB_UTIL_ENABLE_DEVDEBUG
	DB_UTIL_TRACE_DEBUG("__db_util_collate_icu_8 func start \n");
	DB_UTIL_TRACE_DEBUG("v1(%d) : %s, v2(%d) : %s \n", str1_len, (char*)str1, str2_len, (char*)str2);
#endif

	DB_UTIL_RETV_IF(icu_symbol.icu_uiter_setUTF8 == NULL, DB_UTIL_ERR_COL_FUNC_RET);
	DB_UTIL_RETV_IF(icu_symbol.icu_ucol_strcollIter == NULL, DB_UTIL_ERR_COL_FUNC_RET);

	icu_symbol.icu_uiter_setUTF8(&uiter1, (const char *) str1, str1_len);
	icu_symbol.icu_uiter_setUTF8(&uiter2, (const char *) str2, str2_len);

	UCollationResult result = icu_symbol.icu_ucol_strcollIter(
								(UCollator *) ucol,
								&uiter1,
								&uiter2,
								&error);
	if(U_FAILURE(error)) {
		DB_UTIL_TRACE_ERROR("__db_util_collate_icu_8 ucol_strcollIter error: %d\n", error);
		return DB_UTIL_ERR_COL_FUNC_RET;
	}

#ifdef DB_UTIL_ENABLE_DEVDEBUG
	if (result == UCOL_LESS) {
		DB_UTIL_TRACE_DEBUG("less \n");
	} else if (result == UCOL_GREATER) {
		DB_UTIL_TRACE_DEBUG("greater \n");
	} else {
		DB_UTIL_TRACE_DEBUG("equal \n");
	}
#endif

	return result;
}

static int __db_util_collate_icu_8_lc(void *ucol, int str1_len, const void *str1, int str2_len, const void *str2)
{
	UCharIterator uiter1, uiter2;
	UErrorCode error = U_ZERO_ERROR;
	UCollationResult result = 0;
	char* str_from = (char*)str1;
	char* str_to = (char*)str1;
	glong v1_char_len, v2_char_len;
	int i;

#ifdef DB_UTIL_ENABLE_DEVDEBUG
	DB_UTIL_TRACE_DEBUG("__db_util_collate_icu_8_lc func start \n");
	DB_UTIL_TRACE_DEBUG("v1(%d) : %s, v2(%d) : %s \n", str1_len, (char*)str1, str2_len, (char*)str2);
#endif

	DB_UTIL_RETV_IF(icu_symbol.icu_uiter_setUTF8 == NULL, DB_UTIL_ERR_COL_FUNC_RET);
	DB_UTIL_RETV_IF(icu_symbol.icu_ucol_strcollIter	== NULL, DB_UTIL_ERR_COL_FUNC_RET);

	icu_symbol.icu_uiter_setUTF8(&uiter2, (const char *) str2, str2_len);

	v1_char_len = g_utf8_strlen((gchar *)str1,-1);
	v2_char_len = g_utf8_strlen((gchar *)str2,-1);

	if(v1_char_len > v2_char_len) {
		for(i=0;i<v2_char_len;i++) {
			str_to = g_utf8_next_char(str_to);
		}

		while((int)(str_to-(char*)str1) <= str1_len) {
			icu_symbol.icu_uiter_setUTF8(
						&uiter1,
						(const char *) str_from,
						str_to - str_from);

			result = icu_symbol.icu_ucol_strcollIter(
						(UCollator *) ucol,
						&uiter1,
						&uiter2,
						&error);
			if (U_FAILURE(error)) {
				DB_UTIL_TRACE_ERROR("__db_util_collate_icu_8_lc ucol_strcollIter error: %d\n", error);
				return DB_UTIL_ERR_COL_FUNC_RET;
			}

			if(result == UCOL_EQUAL)
#ifdef DB_UTIL_ENABLE_DEVDEBUG
			{
				DB_UTIL_TRACE_DEBUG("equal \n");
				return UCOL_EQUAL;
			}
#else
				return UCOL_EQUAL;
#endif

			str_to = g_utf8_next_char(str_to);
			str_from = g_utf8_next_char(str_from);
		}
	}
	else
	{
		icu_symbol.icu_uiter_setUTF8(
					&uiter1,
					(const char *) str1,
					str1_len);

		result = icu_symbol.icu_ucol_strcollIter(
					(UCollator *) ucol,
					&uiter1,
					&uiter2,
					&error);
		if (U_FAILURE(error)) {
			DB_UTIL_TRACE_ERROR("__db_util_collate_icu_8_lc ucol_strcollIter error: %d\n", error);
			return DB_UTIL_ERR_COL_FUNC_RET;
		}
	}

#ifdef DB_UTIL_ENABLE_DEVDEBUG
	if (result == UCOL_LESS)
	{
		DB_UTIL_TRACE_DEBUG("less \n");
	}
	else if (result == UCOL_GREATER)
	{
		DB_UTIL_TRACE_DEBUG("greater \n");
	}
	else if(result == UCOL_EQUAL)
	{
		DB_UTIL_TRACE_DEBUG("equal \n");
	}
	else
	{
		DB_UTIL_TRACE_DEBUG("compare error : %d\n", result);
	}
#endif

	return result;
}

static void __db_util_collate_icu_close(void* ucol)
{
	DB_UTIL_TRACE_DEBUG("close icu collator\n");

	DB_UTIL_RET_IF(icu_symbol.icu_ucol_close == NULL);

    icu_symbol.icu_ucol_close((UCollator *) ucol);
}

static int __db_util_collation_create(sqlite3* db_handle, char* locale, char* collator_name, UCollationStrength ucol_strength_value, int utf_type, int cmp_type)
{
	int err;
	UErrorCode status = U_ZERO_ERROR;

	DB_UTIL_RETV_IF(icu_symbol.icu_ucol_open == NULL, DB_UTIL_ERR_DLSYM);
	DB_UTIL_RETV_IF(icu_symbol.icu_ucol_setStrength == NULL, DB_UTIL_ERR_DLSYM);

	UCollator* ucol = icu_symbol.icu_ucol_open(locale, &status);
	if(status == U_USING_DEFAULT_WARNING) {
		DB_UTIL_TRACE_ERROR("ucol_open success with default collate option\n");
	} else if (U_FAILURE(status)) {
		DB_UTIL_TRACE_ERROR("ucol_open fail : %d \n", status);
		return DB_UTIL_ERR_ICU;
	}
#ifdef DB_UTIL_ENABLE_DEVDEBUG
	else
	{
		DB_UTIL_TRACE_DEBUG("ucol_open success : %d \n", status);
	}
#endif

	if(ucol_strength_value) {
		icu_symbol.icu_ucol_setStrength(ucol, ucol_strength_value);
		if (U_FAILURE(status)) {
			DB_UTIL_TRACE_ERROR("ucol_setStrength fail : %d \n", status);
			return DB_UTIL_ERR_ICU;
		} else {
			DB_UTIL_TRACE_DEBUG("ucol_setStrength success \n");
		}
	}

	if(utf_type == DB_UTIL_COL_UTF8) {
		if(cmp_type == DB_UTIL_CMP_LC) {
			err = sqlite3_create_collation_v2(db_handle, collator_name, SQLITE_UTF8, ucol,
                __db_util_collate_icu_8_lc, (void(*)(void*))__db_util_collate_icu_close);
		} else {
			err = sqlite3_create_collation_v2(db_handle, collator_name, SQLITE_UTF8, ucol,
                __db_util_collate_icu_8, (void(*)(void*))__db_util_collate_icu_close);
		}
	} else if(utf_type == DB_UTIL_COL_UTF16) {
		if(cmp_type == DB_UTIL_CMP_LC) {
			err = sqlite3_create_collation_v2(db_handle, collator_name, SQLITE_UTF16, ucol,
                __db_util_collate_icu_16_lc, (void(*)(void*))__db_util_collate_icu_close);
		} else {
			err = sqlite3_create_collation_v2(db_handle, collator_name, SQLITE_UTF16, ucol,
                __db_util_collate_icu_16, (void(*)(void*))__db_util_collate_icu_close);
		}
	} else {
		DB_UTIL_TRACE_ERROR("wrong utf_type param value : %d\n", utf_type);
		return DB_UTIL_ERR_PARAM;
	}

	if (err != SQLITE_OK) {
		DB_UTIL_TRACE_ERROR("sqlite3_create_collation_v2 fail : %d \n", err);
		__db_util_collate_icu_close((void*)ucol);
		return err;
	} else {
		DB_UTIL_TRACE_DEBUG("sqlite3_create_collation_v2 success \n");
		return DB_UTIL_OK;
	}
}

int db_util_create_collation(
	PARAM_IN sqlite3 *db_handle,
	PARAM_IN db_util_collate_type type,
	PARAM_IN db_util_collate_textrep text_type,
	PARAM_IN char* col_name)
{
	int ret = DB_UTIL_OK;
	UErrorCode status = U_ZERO_ERROR;
	const char* locale = NULL;

	DB_UTIL_TRACE_DEBUG("db_util_create_collation start");

	ret = __db_util_dl_load_icu();
	DB_UTIL_RETV_IF(ret != DB_UTIL_OK, DB_UTIL_ERROR);

	/* get current locale */
	icu_symbol.icu_uloc_setDefault((const char*)getenv("LC_COLLATE"), &status);
	locale = icu_symbol.icu_uloc_getDefault();
	if(locale == NULL) {
		DB_UTIL_TRACE_WARNING("Fail to get current locale : %d", DB_UTIL_ERR_ENV);
		return DB_UTIL_ERROR;
	}
	DB_UTIL_TRACE_DEBUG("locale : %s", locale);

#ifdef ENABLE_COL_KO_IC
	if((db_util_collate_type == DB_UTIL_COL_KO_IC) ||
		(db_util_collate_type == DB_UTIL_COL_KO_IC_LC)) {
		if(strncmp(locale, "ko", 2) != 0) {
			DB_UTIL_TRACE_WARNING("collate type is not match with current locale : %d", DB_UTIL_ERR_ENV);
			return DB_UTIL_ERROR;
		}
	}
#endif

	switch(type) {
		case DB_UTIL_COL_UCA :
			ret = __db_util_collation_create(db_handle, NULL, col_name, UCOL_SECONDARY, text_type, DB_UTIL_CMP_SC);
			break;
		case DB_UTIL_COL_LS_AS_CI :
			ret = __db_util_collation_create(db_handle, (char*)locale, col_name, UCOL_SECONDARY, text_type, DB_UTIL_CMP_SC);
			break;
		case DB_UTIL_COL_LS_AI_CI :
			ret = __db_util_collation_create(db_handle, (char*)locale, col_name, UCOL_PRIMARY, text_type, DB_UTIL_CMP_SC);
			break;
		case DB_UTIL_COL_LS_AI_CI_LC :
			ret = __db_util_collation_create(db_handle, (char*)locale, col_name, UCOL_PRIMARY, text_type, DB_UTIL_CMP_LC);
			break;
#ifdef ENABLE_COL_KO_IC
		case DB_UTIL_COL_KO_IC :
			ret = __db_util_collation_create(db_handle, (char*)locale, col_name, UCOL_PRIMARY, text_type, DB_UTIL_CMP_SC);
			break;
		case DB_UTIL_COL_KO_IC_LC :
			ret = __db_util_collation_create(db_handle, (char*)locale, col_name, UCOL_PRIMARY, text_type, DB_UTIL_CMP_LC);
			break;
#endif
		default :
			DB_UTIL_TRACE_WARNING("wrong collate input type");
	}

	if(ret != DB_UTIL_OK)
		ret = DB_UTIL_ERROR;

	return ret;
}

#ifdef DB_UTIL_ENABLE_DEVDEBUG

static char *strtoutf8(const UChar * unichars, int len)
{
	DB_UTIL_TRACE_WARNING("strtoutf8 start");

	int lenstr, lenutf8;
	char *pstr = NULL;
	UErrorCode status = U_ZERO_ERROR;

	lenstr = lenutf8 = 0;
	lenstr = sizeof(char) * 4 * (len + 1);
	pstr = (char *)malloc(lenstr);
	if (!pstr)return NULL;
	u_strToUTF8(pstr, lenstr, &lenutf8, unichars, len, &status);
	if (U_FAILURE(status)) {
		DB_UTIL_TRACE_WARNING("u_strToUTF8 failed in strtoutf8 :%s\n",
		       u_errorName(status));
		return NULL;
	}
	DB_UTIL_TRACE_WARNING("strtoutf8 out : %s", pstr);
	return pstr;
}

#endif

