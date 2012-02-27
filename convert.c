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

#include "convert.h"

/* For Convert UTF8 -> UCS2 */
#define conversionOK     (0)	/* conversion successful */
#define sourceoverbmp    (-1)	/* over bmp */
#define sourceIllegal    (-2)	/* source sequence is illegal/malformed */
#define targetExhausted  (-3)	/* insuff. room in target for conversion */

/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR (DB_UINT32)0x0000FFFD
#define UNI_MAX_BMP                      (DB_UINT32)0x0000FFFF
#define UNI_MAX_UTF16            (DB_UINT32)0x0010FFFF
#define UNI_MAX_UTF32            (DB_UINT32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32  (DB_UINT32)0x0010FFFF

#ifdef  CONVERSION_PRECISION_CHECK
#   define      __GetUTF8ByteSize_Internal(c)   \
	((((c)>>4) == 0x0e) ? 3 : (!((c)&0x80)) ? 1 : \
	(((c)>>5) == 0x06) ? 2 : (((c)>>3) == 0x1e) ? 4 : \
	(((c)>>2) == 0x3e) ? 5 : (((c)>>6) == 0x02) ? 6 : 0)
#else
#   define      __GetUTF8ByteSize_Internal(c)  ((!((c) & 0x80)) ? 1 : \
	(((c) >> 5) == 0x06) ? 2 : (((c) >> 4) == 0x0e) ? 3 : 0)
#endif				/* #ifdef  CONVERSION_PRECISION_CHECK */

#define MC_GETUTF8_BYTESEQUENCE_10XXXXXX(v) (((v) & 0xBF) | 0x80)
#define MC_GetUTF8ByteSize(c)  (__GetUTF8ByteSize_Internal((DB_UINT8)(c)))

int dbutil_utf8toucs2(DB_UINT8 *source, DB_UINT16 *dest, int *pdestsize)
{
	int i, nBytes, targetBufSize;
	const DB_UINT8 *p = source;
	DB_UINT32 u32;

	if (source == NULL) {
		*pdestsize = 0;
		return sourceoverbmp;
	}

	if (!dest) {
		targetBufSize = 1;
		while (*p) {
			nBytes = MC_GetUTF8ByteSize(*p);

			u32 = (DB_UINT32)*p;
			switch (nBytes) {
			case 1:
				break;
			case 2:	/* 110xxxxx 10xxxxxx */
				u32 &= 0x1F;
				break;
			case 3:	/* 1110xxxx 10xxxxxx 10xxxxxx */
				u32 &= 0x0F;
				break;
			default:	/* over UNI_MAX_BMP */
				return sourceoverbmp;
			}

			p++;
			while (--nBytes > 0) {
				u32 <<= 6;
				u32 |= (*p & 0x3f);
				p++;
			}

#ifdef  CONVERSION_PRECISION_CHECK
			if (MC_IsSurrogateArea(u32))
				return sourceIllegal;
			else if (u32 <= UNI_MAX_BMP)
				targetBufSize++;
			else
				return sourceoverbmp;
#else
			if (u32 <= UNI_MAX_BMP)
				targetBufSize++;
			else
				return sourceoverbmp;
#endif				/* if (u32 <= UNI_MAX_BMP) */
#if 0
			targetBufSize++;
#endif
		}

		*pdestsize = targetBufSize;
		return conversionOK;
	}
	/* conversion string */
	i = 0;
	targetBufSize = *pdestsize;

	while (*p) {
		nBytes = MC_GetUTF8ByteSize(*p);
		u32 = *p;
		switch (nBytes) {
		case 1:
			break;
		case 2:	/* 110xxxxx 10xxxxxx */
			u32 &= 0x1F;
			break;
		case 3:	/* 1110xxxx 10xxxxxx 10xxxxxx */
			u32 &= 0x0F;
			break;
		default:
			return sourceIllegal;
		}

		p++;
		while (--nBytes > 0) {
			u32 <<= 6;
			u32 |= (*p & 0x3f);
			p++;
		}
		if (i >= targetBufSize) {
			return targetExhausted;
		}
#ifdef  CONVERSION_PRECISION_CHECK
		if (MC_IsSurrogateArea(u32))
			return sourceIllegal;
		else if (u32 <= UNI_MAX_BMP)
			dest[i++] = (DB_UINT16)u32;	/* normal case */
		else
			return sourceoverbmp;
#else
		if (u32 <= UNI_MAX_BMP)
			dest[i++] = (DB_UINT16)u32;	/* normal case */
		else
			return sourceoverbmp;
#endif				/* #ifdef  CONVERSION_PRECISION_CHECK */
	}
	dest[i] = (DB_UINT16)0x00;
	*pdestsize = i + 1;
	return conversionOK;
}

int dbutil_ucs2toutf8(DB_UINT16 *source, DB_UINT8 *dest, int *pdestsize)
{
	int i, targetBufSize;
	DB_UINT16 *p = source;
	DB_UINT32 u32;
	int bytesToWrite = 0;
	const DB_UINT8 szFirstByteMark[7] = {
		0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
	};

	if (source == NULL) {
		*pdestsize = 0;
		return sourceoverbmp;
	}

	if (!dest) {
		targetBufSize = 1;	/* null */
		while (*p) {
			u32 = (DB_UINT32)*p;
#ifdef  CONVERSION_PRECISION_CHECK
			if (MC_IsSurrogateArea(u32))
				return sourceIllegal;
#endif				/* #ifdef  CONVERSION_PRECISION_CHECK */

			if (u32 < (DB_UINT32)0x80)
				targetBufSize++;
			else if (u32 < (DB_UINT32)0x800)
				targetBufSize += 2;
			else if (u32 < (DB_UINT32)0x10000)
				targetBufSize += 3;
			else	/* over UNI_MAX_BMP */
				return sourceoverbmp;

			p++;
		}

		*pdestsize = targetBufSize;
		return conversionOK;
	}
	/* conversion string */
	i = 0;
	targetBufSize = *pdestsize;

	while (*p) {
		u32 = (DB_UINT32)*p++;
#ifdef  CONVERSION_PRECISION_CHECK
		if (MC_IsSurrogateArea(u32))
			return sourceIllegal;
#endif				/* #ifdef  CONVERSION_PRECISION_CHECK */

		if (u32 < (DB_UINT32)0x80)
			bytesToWrite = 1;
		else if (u32 < (DB_UINT32)0x800)
			bytesToWrite = 2;
		else if (u32 < (DB_UINT32)0x10000)
			bytesToWrite = 3;
		else		/* over UNI_MAX_BMP */
			return sourceoverbmp;

		i += bytesToWrite;
		if (i >= targetBufSize)
			return targetExhausted;

		switch (bytesToWrite) {	/* note: everything falls through. */
		case 3:
			dest[--i] =
			    (DB_UINT8)MC_GETUTF8_BYTESEQUENCE_10XXXXXX(u32);
			u32 >>= 6;
		case 2:
			dest[--i] =
			    (DB_UINT8)MC_GETUTF8_BYTESEQUENCE_10XXXXXX(u32);
			u32 >>= 6;
		case 1:
			dest[--i] =
			    (DB_UINT8)(u32 | szFirstByteMark[bytesToWrite]);
		}

		i += bytesToWrite;
	}

	dest[i] = (DB_UINT8)0x00;
	*pdestsize = i + 1;
	return conversionOK;
}
