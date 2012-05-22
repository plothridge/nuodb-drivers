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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


extern "C" {
#include "php.h"
#ifdef ZEND_ENGINE_2
# include "zend_exceptions.h"
#endif
#include "php_ini.h"
#include "ext/standard/info.h"
#include "pdo/php_pdo.h"
#include "pdo/php_pdo_driver.h"
#include "php_pdo_nuodb.h"
}

#include "php_pdo_nuodb_cpp_int.h"
#include "php_pdo_nuodb_int.h"

char const * const NUODB_OPT_DATABASE = "database";
char const * const NUODB_OPT_USER = "user";
char const * const NUODB_OPT_PASSWORD = "password";
char const * const NUODB_OPT_SCHEMA = "schema";

static int nuodb_alloc_prepare_stmt(pdo_dbh_t*, const char*, long, PdoNuoDbStatement **s, HashTable* TSRMLS_DC);

/* map driver specific SQLSTATE error message to PDO error */
void _nuodb_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, char const *file, long line TSRMLS_DC) /* {{{ */
{
	pdo_nuodb_db_handle *H = stmt ? ((pdo_nuodb_stmt *)stmt->driver_data)->H
		: (pdo_nuodb_db_handle *)dbh->driver_data;
	pdo_error_type *const error_code = stmt ? &stmt->error_code : &dbh->error_code;
}
/* }}} */

#define RECORD_ERROR(dbh) _nuodb_error(dbh, NULL, __FILE__, __LINE__ TSRMLS_CC)

static int _commit_if_auto(pdo_dbh_t *dbh) {
	pdo_nuodb_db_handle *H = (pdo_nuodb_db_handle *)dbh->driver_data;
    if (H == NULL) return 0;
   	if (dbh->in_txn) {
		if (dbh->auto_commit) {
		    H->db->commit();
		    return 1;
		} else {
		    H->db->rollback();
		}
	}
    return 0;
}

/* called by PDO to close a db handle */
static int nuodb_handle_closer(pdo_dbh_t *dbh TSRMLS_DC) /* {{{ */
{
	pdo_nuodb_db_handle *H = (pdo_nuodb_db_handle *)dbh->driver_data;
	if (H == NULL) return 0;
	_commit_if_auto(dbh);
    H->db->closeConnection();
    delete H->db;
	pefree(H, dbh->is_persistent);
	return 1;
}
/* }}} */

/* called by PDO to prepare an SQL query */
static int nuodb_handle_preparer(pdo_dbh_t *dbh, const char *sql, long sql_len, /* {{{ */
	pdo_stmt_t *stmt, zval *driver_options TSRMLS_DC)
{
	zval **value;
	HashPosition iterator;
	char *string_key;
	ulong num_key;
	uint str_len;


	int ret = 0;
	pdo_nuodb_db_handle *H = (pdo_nuodb_db_handle *)dbh->driver_data;
	pdo_nuodb_stmt *S = NULL;
	HashTable *np;
	PdoNuoDbStatement *s;

    do {
        nuo_params params;
        char result[8];
        ALLOC_HASHTABLE(np);
        zend_hash_init(np, 8, NULL, NULL, 0);

	/* allocate and prepare statement */
	if (!nuodb_alloc_prepare_stmt(dbh, sql, sql_len, &s, np TSRMLS_CC)) {
	   break;
	}

    S = (pdo_nuodb_stmt *) ecalloc(1, sizeof(*S));
    S->H = H;
    S->stmt = s;
	S->fetch_buf = NULL; // TODO: Needed?
    S->named_params = np;
	S->in_params = NULL;
	S->out_params = NULL;

	// TODO: S->statement_type =

	// allocate input params
	int num_input_params = zend_hash_num_elements(np);
	int index = 0;
	if (num_input_params > 0) {
	  S->in_params = (nuo_params *) ecalloc(1, NUO_PARAMS_LENGTH(num_input_params));
	  S->in_params->num_alloc = S->in_params->num_params = num_input_params;
	  zend_hash_internal_pointer_reset_ex(np, &iterator);
	  while (zend_hash_get_current_data_ex(np, (void **) &value, &iterator) == SUCCESS) {
	    zend_hash_get_current_key_ex(np, &string_key, &str_len, &num_key, 0, &iterator);
	    memcpy(S->in_params->params[index].col_name, string_key, str_len+1);
	    S->in_params->params[index].len = 0;
	    S->in_params->params[index].data = NULL;
	  }
	}

	stmt->driver_data = S;
	stmt->methods = &nuodb_stmt_methods;
	stmt->supports_placeholders = PDO_PLACEHOLDER_POSITIONAL;

	return 1;

    } while(0);

	RECORD_ERROR(dbh);
	zend_hash_destroy(np);
	FREE_HASHTABLE(np);
	if (S) {
		efree(S);
	}
	return 0;
}
/* }}} */

/* called by PDO to execute a statement that doesn't produce a result set */
static long nuodb_handle_doer(pdo_dbh_t *dbh, const char *sql, long sql_len TSRMLS_DC) /* {{{ */
{
	pdo_nuodb_db_handle *H = (pdo_nuodb_db_handle *)dbh->driver_data;
	int in_txn_state = dbh->in_txn;
    try {
        PdoNuoDbStatement *stmt = H->db->createStatement(sql);
        dbh->in_txn = 1;
        stmt->execute();
        _commit_if_auto(dbh);
    } catch(...) {
        dbh->in_txn = in_txn_state;
        return -1;
    }
    dbh->in_txn = in_txn_state;
	return 1;
}
/* }}} */

/* called by the PDO SQL parser to add quotes to values that are copied into SQL */
static int nuodb_handle_quoter(pdo_dbh_t *dbh, const char *unquoted, int unquotedlen, /* {{{ */
	char **quoted, int *quotedlen, enum pdo_param_type paramtype TSRMLS_DC)
{
	int qcount = 0;
	char const *co, *l, *r;
	char *c;

	if (!unquotedlen) {
		*quotedlen = 2;
		*quoted = (char *) emalloc(*quotedlen+1);
		strcpy(*quoted, "''");
		return 1;
	}

	/* only require single quotes to be doubled if string lengths are used */
	/* count the number of ' characters */
	for (co = unquoted; (co = strchr(co,'\'')); qcount++, co++);

	*quotedlen = unquotedlen + qcount + 2;
	*quoted = c = (char *) emalloc(*quotedlen+1);
	*c++ = '\'';

	/* foreach (chunk that ends in a quote) */
	for (l = unquoted; (r = strchr(l,'\'')); l = r+1) {
		strncpy(c, l, r-l+1);
		c += (r-l+1);
		/* add the second quote */
		*c++ = '\'';
	}

	/* copy the remainder */
	strncpy(c, l, *quotedlen-(c-*quoted)-1);
	(*quoted)[*quotedlen-1] = '\'';
	(*quoted)[*quotedlen]   = '\0';

	return 1;
}
/* }}} */

/* called by PDO to start a transaction */
static int nuodb_handle_begin(pdo_dbh_t *dbh TSRMLS_DC) /* {{{ */
{
	pdo_nuodb_db_handle *H = (pdo_nuodb_db_handle *)dbh->driver_data;
	return 1;
}
/* }}} */

/* called by PDO to commit a transaction */
static int nuodb_handle_commit(pdo_dbh_t *dbh TSRMLS_DC) /* {{{ */
{
	pdo_nuodb_db_handle *H = (pdo_nuodb_db_handle *)dbh->driver_data;
	try {
        H->db->commit();
	} catch(...) {
	    // TODO:
	    return 0;
	}
	return 1;
}
/* }}} */

/* called by PDO to rollback a transaction */
static int nuodb_handle_rollback(pdo_dbh_t *dbh TSRMLS_DC) /* {{{ */
{
	pdo_nuodb_db_handle *H = (pdo_nuodb_db_handle *)dbh->driver_data;
	try {
        H->db->rollback();
	} catch(...) {
	    // TODO:
	    return 0;
	}
	return 1;
}
/* }}} */


/* used by prepare and exec to allocate a statement handle and prepare the SQL */
static int nuodb_alloc_prepare_stmt(pdo_dbh_t *dbh, const char *sql, long sql_len, /* {{{ */
				    PdoNuoDbStatement **s, HashTable *named_params TSRMLS_DC)
{
    pdo_nuodb_db_handle *H = (pdo_nuodb_db_handle *)dbh->driver_data;
    char *c, *new_sql, in_quote, in_param, pname[64], *ppname;
    long l, pindex = -1;

    *s = NULL;

	/* There is no max sql statement length in NuoDB - but use 1Mib for now */
	if (sql_len > 0x100000) {
		strcpy(dbh->error_code, "01004");
		return 0;
	}

	/* start a new transaction implicitly if auto_commit is enabled and no transaction is open */
	if (dbh->auto_commit && !dbh->in_txn) {
		if (!nuodb_handle_begin(dbh TSRMLS_CC)) {
			return 0;
		}
		dbh->in_txn = 1;
	}


	/* in order to support named params,
	   we need to replace :foo by ?, and store the name we just replaced */
	new_sql = c = (char *) emalloc(sql_len+1);

	for (l = in_quote = in_param = 0; l <= sql_len; ++l) {
		if ( !(in_quote ^= (sql[l] == '\''))) {
			if (!in_param) {
				switch (sql[l]) {
					case ':':
						in_param = 1;
						ppname = pname;
						*ppname++ = sql[l];
					case '?':
						*c++ = '?';
						++pindex;
					continue;
				}
			} else {
                                if ((in_param &= ((sql[l] >= 'A' && sql[l] <= 'Z') || (sql[l] >= 'a' && sql[l] <= 'z')
                                        || (sql[l] >= '0' && sql[l] <= '9') || sql[l] == '_' || sql[l] == '-'))) {


					*ppname++ = sql[l];
					continue;
				} else {
					*ppname++ = 0;
					if (named_params) {
						zend_hash_update(named_params, pname, (unsigned int)(ppname-pname),
							(void*)&pindex, sizeof(long)+1,NULL);
					}
				}
			}
		}
		*c++ = sql[l];
	}

	/* prepare the statement */
    try {
        *s = H->db->createStatement(new_sql);
    } catch(...) {
        RECORD_ERROR(dbh);
        efree(new_sql);
        return 0;
    }

	efree(new_sql);
	return 1;
}
/* }}} */

/* called by PDO to set a driver-specific dbh attribute */
static int nuodb_handle_set_attribute(pdo_dbh_t *dbh, long attr, zval *val TSRMLS_DC) /* {{{ */
{
	pdo_nuodb_db_handle *H = (pdo_nuodb_db_handle *)dbh->driver_data;

	switch (attr) {
		case PDO_ATTR_AUTOCOMMIT:
			convert_to_boolean(val);

			/* ignore if the new value equals the old one */
			if (dbh->auto_commit ^ Z_BVAL_P(val)) {
				if (dbh->in_txn) {
					if (Z_BVAL_P(val)) {
						/* turning on auto_commit with an open transaction is illegal, because
						   we won't know what to do with it */
						H->last_app_error = "Cannot enable auto-commit while a transaction is already open";
						return 0;
					} else {
						/* close the transaction */
						if (!nuodb_handle_commit(dbh TSRMLS_CC)) {
							break;
						}
						dbh->in_txn = 0;
					}
				}
				dbh->auto_commit = Z_BVAL_P(val);
			}
			return 1;

		case PDO_ATTR_FETCH_TABLE_NAMES:
			convert_to_boolean(val);
			H->fetch_table_names = Z_BVAL_P(val);
			return 1;

	}
	return 0;
}
/* }}} */


/* called by PDO to get a driver-specific dbh attribute */
static int nuodb_handle_get_attribute(pdo_dbh_t *dbh, long attr, zval *val TSRMLS_DC) /* {{{ */
{
	pdo_nuodb_db_handle *H = (pdo_nuodb_db_handle *)dbh->driver_data;

	switch (attr) {
		case PDO_ATTR_AUTOCOMMIT:
			ZVAL_LONG(val,dbh->auto_commit);
			return 1;

		case PDO_ATTR_CONNECTION_STATUS:
			return 1;

		case PDO_ATTR_CLIENT_VERSION:
			ZVAL_STRING(val,"NuoDB 1.0",1);
			return 1;

		case PDO_ATTR_SERVER_VERSION:
		case PDO_ATTR_SERVER_INFO:
            // TODO: call the NuoDB API to get the verson number.
			ZVAL_STRING(val,"NuoDB 1.0",1);
			return 1;

		case PDO_ATTR_FETCH_TABLE_NAMES:
			ZVAL_BOOL(val, H->fetch_table_names);
			return 1;
	}
	return 0;
}
/* }}} */

/* called by PDO to retrieve driver-specific information about an error that has occurred */
static int pdo_nuodb_fetch_error_func(pdo_dbh_t *dbh, pdo_stmt_t *stmt, zval *info TSRMLS_DC) /* {{{ */
{
	pdo_nuodb_db_handle *H = (pdo_nuodb_db_handle *)dbh->driver_data;
	// TODO:
	return 1;
}
/* }}} */

static struct pdo_dbh_methods nuodb_methods = { /* {{{ */
	nuodb_handle_closer,
	nuodb_handle_preparer,
	nuodb_handle_doer,
	nuodb_handle_quoter,
	nuodb_handle_begin,
	nuodb_handle_commit,
	nuodb_handle_rollback,
	nuodb_handle_set_attribute,
	NULL, /* last_id not supported */
	pdo_nuodb_fetch_error_func,
	nuodb_handle_get_attribute,
	NULL /* check_liveness */
};
/* }}} */

/* the driver-specific PDO handle constructor */
static int pdo_nuodb_handle_factory(pdo_dbh_t *dbh, zval *driver_options TSRMLS_DC) /* {{{ */
{
	struct pdo_data_src_parser vars[] = {
		{ NUODB_OPT_DATABASE, NULL, 0 }, // "database"
		{ NUODB_OPT_SCHEMA,  NULL,	0 }  // "schema"
 	};
	int i, ret = 0;
	short buf_len = 256, dpb_len;

	pdo_nuodb_db_handle *H = NULL;
    dbh->driver_data = pecalloc(1,sizeof(*H),dbh->is_persistent);
	H = (pdo_nuodb_db_handle *) dbh->driver_data;


	php_pdo_parse_data_source(dbh->data_source, dbh->data_source_len, vars, 2);

    SqlOption options[4];
    options[0].option = "database";
    options[0].extra = (void*) vars[0].optval;
    options[1].option = "user";
    options[1].extra = (void *) dbh->username;
    options[2].option = "password";
    options[2].extra = (void *) dbh->password;
    options[3].option = "schema";
    options[3].extra = (void *) vars[1].optval;
    SqlOptionArray optionsArray;
    optionsArray.count = 4;
    optionsArray.array = options;
    try {
        H->db = new PdoNuoDbHandle(&optionsArray);
        H->db->createConnection();
        dbh->methods = &nuodb_methods;
        dbh->native_case = PDO_CASE_NATURAL;  // TODO: the value should reflect how the database returns the names of the columns in result sets. If the name matches the case that was used in the query, set it to PDO_CASE_NATURAL (this is actually the default). If the column names are always returned in upper case, set it to PDO_CASE_UPPER. If the column names are always returned in lower case, set it to PDO_CASE_LOWER. The value you set is used to determine if PDO should perform case folding when the user sets the PDO_ATTR_CASE attribute.
        dbh->alloc_own_columns = 1;  // if true, the driver requires that memory be allocated explicitly for the columns that are returned
        ret = 1;
    } catch(ErrorCodeException &e) {
        zend_throw_exception_ex(php_pdo_get_exception(), e.errorCode() TSRMLS_CC, "SQLSTATE[%s] [%d] %s",
				"HY000", e.errorCode(), e.what());
    }

	for (i = 0; i < sizeof(vars)/sizeof(vars[0]); ++i) {
		if (vars[i].freeme) {
			efree(vars[i].optval);
		}
	}

	if (!ret) {
		nuodb_handle_closer(dbh TSRMLS_CC);
	}

	return ret;
}
/* }}} */


pdo_driver_t pdo_nuodb_driver = { /* {{{ */
	PDO_DRIVER_HEADER(nuodb),
	pdo_nuodb_handle_factory
};
/* }}} */

