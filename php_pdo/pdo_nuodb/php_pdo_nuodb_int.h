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

#define SHORT_MAX (1 << (8*sizeof(short)-1))

#define PDO_NUODB_SQLTYPE_BOOLEAN   1
#define PDO_NUODB_SQLTYPE_INTEGER   2
#define PDO_NUODB_SQLTYPE_BIGINT    3
#define PDO_NUODB_SQLTYPE_DOUBLE    4
#define PDO_NUODB_SQLTYPE_STRING    5
#define PDO_NUODB_SQLTYPE_DATE      6
#define PDO_NUODB_SQLTYPE_TIME      7
#define PDO_NUODB_SQLTYPE_DATETIME  8

typedef struct {
  short type;  // datatype 
  short scale; // scale factor
  short col_name_length; // length of column name
  char  col_name[32];
  short len; // length of data buffer
  char *data; // address of data buffer
} nuo_param; // XSQLVAR

typedef struct {
  short num_params;  // number of actual params (sqld)
  short num_alloc;  // number of allocated params (sqln)
  nuo_param params[1]; // address of first param 
} nuo_params; //XSQLDA

#define NUO_PARAMS_LENGTH(n)   (sizeof(nuo_params) + (n-1) * sizeof(nuo_param))

typedef struct {
	/* the connection handle */
    PdoNuoDbHandle *db;

	/* the last error that didn't come from the API */
	char const *last_app_error;

	/* prepend table names on column names in fetch */
	unsigned fetch_table_names:1;

} pdo_nuodb_db_handle;


typedef struct {

	/* the link that owns this statement */
	pdo_nuodb_db_handle *H;

	/* the statement handle */
    PdoNuoDbStatement *stmt;

	/* the name of the cursor (if it has one) */
	char name[32];

	/* the type of statement that was issued */
	char statement_type:8;
	
	/* whether EOF was reached for this statement */
	unsigned exhausted:1;

	/* successful execute opens a cursor */
	unsigned cursor_open:1;

	unsigned _reserved:22;

	/* the named params that were converted to ?'s by the driver */
	HashTable *named_params;

	/* allocated space to convert fields values to other types */
	char **fetch_buf;
	
	/* the input params */
	nuo_params *in_params;
	
	/* the output params */
	nuo_params *out_params; 
	
} pdo_nuodb_stmt;

extern pdo_driver_t pdo_nuodb_driver;

extern struct pdo_stmt_methods nuodb_stmt_methods;

void _nuodb_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, char const *file, long line TSRMLS_DC);

enum {
	PDO_NUODB_ATTR_DATE_FORMAT = PDO_ATTR_DRIVER_SPECIFIC,
	PDO_NUODB_ATTR_TIME_FORMAT,
	PDO_NUODB_ATTR_TIMESTAMP_FORMAT,
};

#endif	/* PHP_PDO_NUODB_INT_H */
