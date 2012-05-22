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

#include <time.h>

#define RECORD_ERROR(stmt) _nuodb_error(NULL, stmt,  __FILE__, __LINE__ TSRMLS_CC)
#define CHAR_BUF_LEN 24

static void _release_PdoNuoDbStatement(pdo_nuodb_stmt *S) {
    if (S == NULL) return;
    if (S->stmt == NULL) return;
    delete S->stmt;
    S->stmt = NULL;
}

/* called by PDO to clean up a statement handle */
static int nuodb_stmt_dtor(pdo_stmt_t *stmt TSRMLS_DC) /* {{{ */
{
    int result = 1;
    int i;
	pdo_nuodb_stmt *S = (pdo_nuodb_stmt*)stmt->driver_data;
    _release_PdoNuoDbStatement(S); /* release the statement */

	/* clean up the fetch buffers if they have been used */
	if (S->fetch_buf != NULL) {
		for (i = 0; i < S->out_params->num_params; ++i) {
			if (S->fetch_buf[i] != NULL) {
				efree(S->fetch_buf[i]);
			}
		}
		efree(S->fetch_buf);
		S->fetch_buf = NULL;
	}


	zend_hash_destroy(S->named_params);
	FREE_HASHTABLE(S->named_params);

	/* clean up input params */
	if (S->in_params != NULL) {
		efree(S->in_params);
	}

	efree(S);
	return result;
}
/* }}} */

/* called by PDO to execute a prepared query */
static int nuodb_stmt_execute(pdo_stmt_t *stmt TSRMLS_DC) /* {{{ */
{
	pdo_nuodb_stmt *S = (pdo_nuodb_stmt*)stmt->driver_data;
	if (!S) return 0;
	pdo_nuodb_db_handle *H = S->H;
	if (!H) return 0;
	unsigned long affected_rows = 0;
    if (!S->stmt) return 0;

	// TODO: check that (!stmt->executed) here?

    try {
        S->stmt->executeQuery();
        S->cursor_open = S->stmt->hasResultSet();
        stmt->column_count = S->stmt->getColumnCount();
    } catch(ErrorCodeException &e) {
        RECORD_ERROR(stmt);
        return 0;
    }
    stmt->row_count = affected_rows;

    // TODO: commit here?
    H->db->commit();
	S->exhausted = !S->cursor_open;

	return 1;

}
/* }}} */

/* called by PDO to fetch the next row from a statement */
static int nuodb_stmt_fetch(pdo_stmt_t *stmt, /* {{{ */
	enum pdo_fetch_orientation ori, long offset TSRMLS_DC)
{
	pdo_nuodb_stmt *S = (pdo_nuodb_stmt*)stmt->driver_data;
	pdo_nuodb_db_handle *H = S->H;

	if (!stmt->executed) {
		strcpy(stmt->error_code, "HY000");
		H->last_app_error = "Cannot fetch from a closed cursor";
	} else if (!S->exhausted) {
	    if (S->stmt->next()) {
            stmt->row_count++;
            return 1;
	    } else {
	        S->exhausted = 1;
	        return 0;
	    }
	}
	return 0;
}
/* }}} */

/* called by PDO to retrieve information about the fields being returned */
static int nuodb_stmt_describe(pdo_stmt_t *stmt, int colno TSRMLS_DC) /* {{{ */
{
	pdo_nuodb_stmt *S = (pdo_nuodb_stmt*)stmt->driver_data;
	struct pdo_column_data *col = &stmt->columns[colno];
    char *cp;

    col->precision = 0;
    col->maxlen = 0;

    char const *column_name = S->stmt->getColumnName(colno);
    if (column_name == NULL) return 0;
    int colname_len = strlen(column_name);
    col->namelen = colname_len;
	col->name = cp = (char *) emalloc(colname_len + 1);
    memmove(cp, column_name, colname_len);
	*(cp+colname_len) = '\0';
    int sqlTypeNumber = S->stmt->getSqlType(colno);
    switch(sqlTypeNumber) {
        case PDO_NUODB_SQLTYPE_BOOLEAN:
        {
            col->param_type = PDO_PARAM_BOOL;
            break;
        }
        case PDO_NUODB_SQLTYPE_INTEGER:
        {
            col->maxlen = 24;
            col->param_type = PDO_PARAM_STR;
            break;
        }
        case PDO_NUODB_SQLTYPE_BIGINT:
        {
            col->maxlen = 24;
            col->param_type = PDO_PARAM_STR;
            break;
        }
        case PDO_NUODB_SQLTYPE_DOUBLE:
        {
            col->param_type = PDO_PARAM_STR;
            break;
        }
        case PDO_NUODB_SQLTYPE_STRING:
        {
            col->param_type = PDO_PARAM_STR;
            break;
        }
        case PDO_NUODB_SQLTYPE_DATE:
        {
            col->param_type = PDO_PARAM_STR;
            break;
        }
        case PDO_NUODB_SQLTYPE_TIME:
        {
            col->param_type = PDO_PARAM_STR;
            break;
        }
        case PDO_NUODB_SQLTYPE_DATETIME:
        {
            col->param_type = PDO_PARAM_STR;
            break;
        }
    }

	return 1;
}
/* }}} */



static int nuodb_stmt_get_col(pdo_stmt_t *stmt, int colno, char **ptr,  /* {{{ */
	unsigned long *len, int *caller_frees TSRMLS_DC)
{
	pdo_nuodb_stmt *S = (pdo_nuodb_stmt*)stmt->driver_data;
	*len = 0;
	*ptr = NULL;
    int sqlTypeNumber = S->stmt->getSqlType(colno);
    switch(sqlTypeNumber) {
        case PDO_NUODB_SQLTYPE_BOOLEAN:
        {
            break;
        }
        case PDO_NUODB_SQLTYPE_INTEGER:
        {
			*ptr = (char *)emalloc(CHAR_BUF_LEN);
            *len = slprintf(*ptr, CHAR_BUF_LEN, "%d", S->stmt->getInteger(colno));
            break;
        }
        case PDO_NUODB_SQLTYPE_BIGINT:
        {
			*ptr = (char *)emalloc(CHAR_BUF_LEN);
			*len = slprintf(*ptr, CHAR_BUF_LEN, "%ld", S->stmt->getLong(colno));
            break;
        }
        case PDO_NUODB_SQLTYPE_DOUBLE:
        {
            break;
        }
        case PDO_NUODB_SQLTYPE_STRING:
        {
            const char *str = S->stmt->getString(colno);
            if (str == NULL) break;
            int str_len = strlen(str);
            *ptr = (char *) emalloc(str_len+1);
            memmove(*ptr, str, str_len);
            *((*ptr)+str_len)= 0;
            *len = str_len;
            break;
        }
        case PDO_NUODB_SQLTYPE_DATE:
        {
            break;
        }
        case PDO_NUODB_SQLTYPE_TIME:
        {
            break;
        }
        case PDO_NUODB_SQLTYPE_DATETIME:
        {
            break;
        }
    }

	return 1;
}
/* }}} */


static int nuodb_stmt_param_hook(pdo_stmt_t *stmt, struct pdo_bound_param_data *param, /* {{{ */
	enum pdo_param_event event_type TSRMLS_DC)
{
	pdo_nuodb_stmt *S = (pdo_nuodb_stmt*)stmt->driver_data;
	nuo_params *nuodb_params = param->is_param ? S->in_params : S->out_params;
	nuo_param *nuodb_param = NULL;

	if (event_type == PDO_PARAM_EVT_FREE) { /* not used */
		return 1;
	}

	if (!nuodb_params || param->paramno >= nuodb_params->num_params) {
		strcpy(stmt->error_code, "HY093");
		S->H->last_app_error = "Invalid parameter index";
		return 0;
	}

	if (param->is_param && param->paramno == -1) {
	    long *index;

		/* try to determine the index by looking in the named_params hash */
		if (SUCCESS == zend_hash_find(S->named_params, param->name, param->namelen+1, (void**)&index)) {
			param->paramno = *index;
		} else {
			// TODO: or by looking in the input descriptor
			// for now, just return an error
  			strcpy(stmt->error_code, "HY000");
			S->H->last_app_error = "Unable to determine the parameter index";
			return 0;
		}

	}

	return 1;

}
/* }}} */

static int nuodb_stmt_set_attribute(pdo_stmt_t *stmt, long attr, zval *val TSRMLS_DC) /* {{{ */
{
	pdo_nuodb_stmt *S = (pdo_nuodb_stmt*)stmt->driver_data;

	switch (attr) {
		default:
			return 0;
		case PDO_ATTR_CURSOR_NAME:
			convert_to_string(val);
			strlcpy(S->name, Z_STRVAL_P(val), sizeof(S->name));
			break;
	}
	return 1;
}
/* }}} */

static int nuodb_stmt_get_attribute(pdo_stmt_t *stmt, long attr, zval *val TSRMLS_DC) /* {{{ */
{
	pdo_nuodb_stmt *S = (pdo_nuodb_stmt*)stmt->driver_data;

	switch (attr) {
		default:
			return 0;
		case PDO_ATTR_CURSOR_NAME:
			if (*S->name) {
				ZVAL_STRING(val,S->name,1);
			} else {
				ZVAL_NULL(val);
			}
			break;
	}
	return 1;
}
/* }}} */

static int nuodb_stmt_cursor_closer(pdo_stmt_t *stmt TSRMLS_DC) /* {{{ */
{
	pdo_nuodb_stmt *S = (pdo_nuodb_stmt*)stmt->driver_data;

	if ((*S->name || S->cursor_open)) {
	    _release_PdoNuoDbStatement(S);
	}
	*S->name = 0;
	S->cursor_open = 0;
	return 1;
}
/* }}} */


struct pdo_stmt_methods nuodb_stmt_methods = { /* {{{ */
	nuodb_stmt_dtor,
	nuodb_stmt_execute,
	nuodb_stmt_fetch,
	nuodb_stmt_describe,
	nuodb_stmt_get_col,
	nuodb_stmt_param_hook,
	nuodb_stmt_set_attribute,
	nuodb_stmt_get_attribute,
	NULL, /* get_column_meta_func */
	NULL, /* next_rowset_func */
	nuodb_stmt_cursor_closer
};
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
