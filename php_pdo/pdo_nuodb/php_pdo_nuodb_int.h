/****************************************************************************
 * Copyright (c) 2012, NuoDB, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of NuoDB, Inc. nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NUODB, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************/

#ifndef PHP_PDO_NUODB_INT_H
#define PHP_PDO_NUODB_INT_H


#define PDO_NUODB_VERSION 1

#define PDO_NUODB_DEF_DATE_FMT "%Y-%m-%d"
#define PDO_NUODB_DEF_TIME_FMT "%H:%M:%S"
#define PDO_NUODB_DEF_TIMESTAMP_FMT PDO_FB_DEF_DATE_FMT " " PDO_FB_DEF_TIME_FMT

#define SHORT_MAX (1 << (8*sizeof(short)-1))

#if SIZEOF_LONG == 8
# define LL_MASK "l"
# define LL_LIT(lit) lit ## L
#else
# ifdef PHP_WIN32
#  define LL_MASK "I64"
#  define LL_LIT(lit) lit ## I64
# else
#  define LL_MASK "ll"
#  define LL_LIT(lit) lit ## LL
# endif
#endif

#define const_cast(s) ((char*)(s))

#ifdef PHP_WIN32
typedef void (__stdcall *info_func_t)(char*);
#else
typedef void (*info_func_t)(char*);
#endif

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#define PDO_NUODB_SQLDA_VERSION 1


#define PDO_NUODB_SQLTYPE_BOOLEAN   1
#define PDO_NUODB_SQLTYPE_INTEGER   2
#define PDO_NUODB_SQLTYPE_BIGINT    3
#define PDO_NUODB_SQLTYPE_DOUBLE    4
#define PDO_NUODB_SQLTYPE_STRING    5
#define PDO_NUODB_SQLTYPE_DATE      6
#define PDO_NUODB_SQLTYPE_TIME      7
#define PDO_NUODB_SQLTYPE_DATETIME  8

typedef struct {
	/* the connection handle */

	///* NuoDB SqlConnection */ void *db;
    PdoNuoDbHandle *db;


	/* the transaction handle */
	int tr;

	/* the last error that didn't come from the API */
	char const *last_app_error;


	/* prepend table names on column names in fetch */
	unsigned fetch_table_names:1;

	unsigned _reserved:31;

} pdo_nuodb_db_handle;


typedef struct {

	/* the link that owns this statement */
	pdo_nuodb_db_handle *H;

	/* the statement handle */
    PdoNuoDbStatement *stmt;

	/* the name of the cursor (if it has one) */
	char name[32];

	/* whether EOF was reached for this statement */
	unsigned exhausted:1;

	/* successful isc_dsql_execute opens a cursor */
	unsigned cursor_open:1;

	unsigned _reserved:22;

	/* the named params that were converted to ?'s by the driver */
	HashTable *named_params;


} pdo_nuodb_stmt;

extern pdo_driver_t pdo_nuodb_driver;

extern struct pdo_stmt_methods nuodb_stmt_methods;

void _nuodb_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, char const *file, long line TSRMLS_DC);

enum {
	PDO_NUODB_ATTR_DATE_FORMAT = PDO_ATTR_DRIVER_SPECIFIC,
	PDO_NUODB_ATTR_TIME_FORMAT,
	PDO_NUODB_ATTR_TIMESTAMP_FORMAT,
};



// Everything below is borrowed

#if 0

#if defined(__GNUC__) || defined (__HP_cc) || defined (__HP_aCC)
#include <inttypes.h>
#else

#if !defined(_INTPTR_T_DEFINED)
#if defined(_WIN64)
typedef __int64 intptr_t;
typedef unsigned __int64 uintptr_t;
#else
typedef long intptr_t;
typedef unsigned long uintptr_t;
#endif
#endif

#endif

/******************************************************************/
/* API handles                                                    */
/******************************************************************/

#if defined(_LP64) || defined(__LP64__) || defined(__arch64__) || defined(_WIN64)
typedef unsigned int	FB_API_HANDLE;
#else
typedef void*		FB_API_HANDLE;
#endif

/******************************************************************/
/* Status vector                                                  */
/******************************************************************/

typedef intptr_t ISC_STATUS;

#define ISC_STATUS_LENGTH	20
typedef ISC_STATUS ISC_STATUS_ARRAY[ISC_STATUS_LENGTH];

/* SQL State as defined in the SQL Standard. */
#define FB_SQLSTATE_LENGTH	5
#define FB_SQLSTATE_SIZE	(FB_SQLSTATE_LENGTH + 1)
typedef char FB_SQLSTATE_STRING[FB_SQLSTATE_SIZE];

/******************************************************************/
/* Define type, export and other stuff based on c/c++ and Windows */
/******************************************************************/
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
	#define  ISC_EXPORT	__stdcall
	#define  ISC_EXPORT_VARARG	__cdecl
#else
	#define  ISC_EXPORT
	#define  ISC_EXPORT_VARARG
#endif

/*
 * It is difficult to detect 64-bit long from the redistributable header
 * we do not care of 16-bit platforms anymore thus we may use plain "int"
 * which is 32-bit on all platforms we support
 *
 * We'll move to this definition in future API releases.
 *
 */

#if defined(_LP64) || defined(__LP64__) || defined(__arch64__)
typedef	int		ISC_LONG;
typedef	unsigned int	ISC_ULONG;
#else
typedef	signed long	ISC_LONG;
typedef	unsigned long	ISC_ULONG;
#endif

typedef	signed short	ISC_SHORT;
typedef	unsigned short	ISC_USHORT;

typedef	unsigned char	ISC_UCHAR;
typedef char		ISC_SCHAR;

/*******************************************************************/
/* 64 bit Integers                                                 */
/*******************************************************************/

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__)) && !defined(__GNUC__)
typedef __int64				ISC_INT64;
typedef unsigned __int64	ISC_UINT64;
#else
typedef long long int			ISC_INT64;
typedef unsigned long long int	ISC_UINT64;
#endif

/*******************************************************************/
/* Time & Date support                                             */
/*******************************************************************/

#ifndef ISC_TIMESTAMP_DEFINED
typedef int			ISC_DATE;
typedef unsigned int	ISC_TIME;
typedef struct
{
	ISC_DATE timestamp_date;
	ISC_TIME timestamp_time;
} ISC_TIMESTAMP;
#define ISC_TIMESTAMP_DEFINED
#endif	/* ISC_TIMESTAMP_DEFINED */

/*******************************************************************/
/* Blob Id support                                                 */
/*******************************************************************/

struct GDS_QUAD_t {
	ISC_LONG gds_quad_high;
	ISC_ULONG gds_quad_low;
};

typedef struct GDS_QUAD_t GDS_QUAD;
typedef struct GDS_QUAD_t ISC_QUAD;

#define	isc_quad_high	gds_quad_high
#define	isc_quad_low	gds_quad_low

typedef int (*FB_SHUTDOWN_CALLBACK)(const int reason, const int mask, void* arg);


/* Definitions for DSQL free_statement routine */

#define DSQL_close		1
#define DSQL_drop		2
#define DSQL_unprepare	4

/* Declare the extended SQLDA */

typedef struct
{
	ISC_SHORT	sqltype;			/* datatype of field */
	ISC_SHORT	sqlscale;			/* scale factor */
	ISC_SHORT	sqlsubtype;			/* datatype subtype - currently BLOBs only */
	ISC_SHORT	sqllen;				/* length of data area */
	ISC_SCHAR*	sqldata;			/* address of data */
	ISC_SHORT*	sqlind;				/* address of indicator variable */
	ISC_SHORT	sqlname_length;		/* length of sqlname field */
	ISC_SCHAR	sqlname[32];		/* name of field, name length + space for NULL */
	ISC_SHORT	relname_length;		/* length of relation name */
	ISC_SCHAR	relname[32];		/* field's relation name + space for NULL */
	ISC_SHORT	ownname_length;		/* length of owner name */
	ISC_SCHAR	ownname[32];		/* relation's owner name + space for NULL */
	ISC_SHORT	aliasname_length;	/* length of alias name */
	ISC_SCHAR	aliasname[32];		/* relation's alias name + space for NULL */
} XSQLVAR;

#define SQLDA_VERSION1		1

typedef struct
{
	ISC_SHORT	version;			/* version of this XSQLDA */
	ISC_SCHAR	sqldaid[8];			/* XSQLDA name field */
	ISC_LONG	sqldabc;			/* length in bytes of SQLDA */
	ISC_SHORT	sqln;				/* number of fields allocated */
	ISC_SHORT	sqld;				/* actual number of fields */
	XSQLVAR	sqlvar[1];			/* first field address */
} XSQLDA;

#define XSQLDA_LENGTH(n)	(sizeof (XSQLDA) + (n - 1) * sizeof (XSQLVAR))

#define SQL_TEXT                           452
#define SQL_VARYING                        448
#define SQL_SHORT                          500
#define SQL_LONG                           496
#define SQL_FLOAT                          482
#define SQL_DOUBLE                         480
#define SQL_D_FLOAT                        530
#define SQL_TIMESTAMP                      510
#define SQL_BLOB                           520
#define SQL_ARRAY                          540
#define SQL_QUAD                           550
#define SQL_TYPE_TIME                      560
#define SQL_TYPE_DATE                      570
#define SQL_INT64                          580
#define SQL_NULL                         32766

/* Historical alias for pre v6 code */
#define SQL_DATE                           SQL_TIMESTAMP

/***************************/
/* SQL Dialects            */
/***************************/

#define SQL_DIALECT_V5				1	/* meaning is same as DIALECT_xsqlda */
#define SQL_DIALECT_V6_TRANSITION	2	/* flagging anything that is delimited
										   by double quotes as an error and
										   flagging keyword DATE as an error */
#define SQL_DIALECT_V6				3	/* supports SQL delimited identifier,
										   SQLDATE/DATE, TIME, TIMESTAMP,
										   CURRENT_DATE, CURRENT_TIME,
										   CURRENT_TIMESTAMP, and 64-bit exact
										   numeric type */
#define SQL_DIALECT_CURRENT		SQL_DIALECT_V6	/* latest IB DIALECT */

#endif

#endif	/* PHP_PDO_NUODB_INT_H */
