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

#include "php.h"
#ifdef ZEND_ENGINE_2
# include "zend_exceptions.h"
#endif
#include "php_ini.h"
#include "ext/standard/info.h"
#include "pdo/php_pdo.h"
#include "pdo/php_pdo_driver.h"
#include "php_pdo_nuodb.h"

#include "php_pdo_nuodb_c_cpp_common.h"
#include "php_pdo_nuodb_int.h"

#include <time.h>

#define RECORD_ERROR(stmt) _nuodb_error(NULL, stmt,  __FILE__, __LINE__ TSRMLS_CC)
#define CHAR_BUF_LEN 24

static void _release_PdoNuoDbStatement(pdo_nuodb_stmt * S)
{
	pdo_nuodb_stmt_delete(S);
}

/* called by PDO to clean up a statement handle */
static int nuodb_stmt_dtor(pdo_stmt_t * stmt TSRMLS_DC) /* {{{ */
{
    int result = 1;
    int i;
    pdo_nuodb_stmt * S = (pdo_nuodb_stmt *)stmt->driver_data;
    _release_PdoNuoDbStatement(S); /* release the statement */

    /* clean up the fetch buffers if they have been used */
    if (S->fetch_buf != NULL)
    {
        for (i = 0; i < S->out_params->num_params; ++i)
        {
            if (S->fetch_buf[i] != NULL)
            {
                efree(S->fetch_buf[i]);
            }
        }
        efree(S->fetch_buf);
        S->fetch_buf = NULL;
    }


    zend_hash_destroy(S->named_params);
    FREE_HASHTABLE(S->named_params);

    /* clean up input params */
    if (S->in_params != NULL)
    {
        efree(S->in_params);
    }

    efree(S);
    return result;
}
/* }}} */

/* called by PDO to execute a prepared query */
static int nuodb_stmt_execute(pdo_stmt_t * stmt TSRMLS_DC) /* {{{ */
{
	int status; 

    pdo_nuodb_stmt * S = (pdo_nuodb_stmt *)stmt->driver_data;
    if (!S) {
        return 0;
    }

	status = pdo_nuodb_stmt_execute(S, &stmt->column_count, &stmt->row_count);
	if (status == 0) {
        RECORD_ERROR(stmt);
        return 0;
    }
    return 1;
}
/* }}} */

/* called by PDO to fetch the next row from a statement */
static int nuodb_stmt_fetch(pdo_stmt_t * stmt, /* {{{ */
                            enum pdo_fetch_orientation ori, long offset TSRMLS_DC)
{
    pdo_nuodb_stmt * S = (pdo_nuodb_stmt *)stmt->driver_data;
    if (!stmt->executed)
    {
        strcpy_s(stmt->error_code, 5, "HY000");
        pdo_nuodb_db_handle_set_last_app_error(S->H, "Cannot fetch from a closed cursor");
		return 0;
    }
	return pdo_nuodb_stmt_fetch(S, &stmt->row_count);
}
/* }}} */

/* called by PDO to retrieve information about the fields being returned */
static int nuodb_stmt_describe(pdo_stmt_t * stmt, int colno TSRMLS_DC) /* {{{ */
{
    pdo_nuodb_stmt *S = (pdo_nuodb_stmt *)stmt->driver_data;
    struct pdo_column_data *col = &stmt->columns[colno];
    char *cp;
	char const *column_name;
	int colname_len;
	int sqlTypeNumber;

    col->precision = 0;
    col->maxlen = 0;

    column_name = pdo_nuodb_stmt_get_column_name(S, colno);
    if (column_name == NULL)
    {
        return 0;
    }
    colname_len = strlen(column_name);
    col->namelen = colname_len;
    col->name = cp = (char *) emalloc(colname_len + 1);
    memmove(cp, column_name, colname_len);
    *(cp+colname_len) = '\0';
    sqlTypeNumber = pdo_nuodb_stmt_get_sql_type(S, colno);
    switch (sqlTypeNumber)
    {
    case PDO_NUODB_SQLTYPE_BOOLEAN:
    {
        col->param_type = PDO_PARAM_BOOL;
        break;
    }
    case PDO_NUODB_SQLTYPE_INTEGER:
    {
        col->maxlen = 24;
        col->param_type = PDO_PARAM_STR;  // TODO: Is this correct?  Shouldn't it be a long?
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
        col->param_type = PDO_PARAM_INT;
        break;
    }
    case PDO_NUODB_SQLTYPE_TIME:
    {
        col->param_type = PDO_PARAM_INT;
        break;
    }
    case PDO_NUODB_SQLTYPE_TIMESTAMP:
    {
        col->param_type = PDO_PARAM_INT;
        break;
    }
    }

    return 1;

}
/* }}} */

static int nuodb_stmt_get_col(pdo_stmt_t * stmt, int colno, char ** ptr, /* {{{ */
                              unsigned long * len, int * caller_frees TSRMLS_DC)
{
    pdo_nuodb_stmt * S = (pdo_nuodb_stmt *)stmt->driver_data;
    int sqlTypeNumber = pdo_nuodb_stmt_get_sql_type(S, colno);

	*len = 0;
    *ptr = NULL;
    switch (sqlTypeNumber)
    {
        case PDO_NUODB_SQLTYPE_BOOLEAN:
        {
            break;
        }
        case PDO_NUODB_SQLTYPE_INTEGER:
        {
			unsigned int val = pdo_nuodb_stmt_get_integer(S, colno);
            *ptr = (char *)emalloc(CHAR_BUF_LEN);
            *len = slprintf(*ptr, CHAR_BUF_LEN, "%ld", val);
            break;
        }
        case PDO_NUODB_SQLTYPE_BIGINT:
        {
			unsigned long val = pdo_nuodb_stmt_get_long(S, colno);
            *ptr = (char *)emalloc(CHAR_BUF_LEN);
            *len = slprintf(*ptr, CHAR_BUF_LEN, "%ld", val);
            break;
        }
        case PDO_NUODB_SQLTYPE_DOUBLE:
        {
            break;
        }
        case PDO_NUODB_SQLTYPE_STRING:
        {
			int str_len;
            const char * str = pdo_nuodb_stmt_get_string(S, colno);
            if (str == NULL)
            {
                break;
            }
            str_len = strlen(str);
            *ptr = (char *) emalloc(str_len+1);
            memmove(*ptr, str, str_len);
            *((*ptr)+str_len)= 0;
            *len = str_len;
            break;
        }
        case PDO_NUODB_SQLTYPE_DATE:
        {
            long d = pdo_nuodb_stmt_get_date(S, colno);
            *len = sizeof(long);
            *ptr = (char *)emalloc(*len);
            memmove(*ptr, &d, *len);
            break;
        }
        case PDO_NUODB_SQLTYPE_TIME:
        {
            unsigned long t = pdo_nuodb_stmt_get_time(S, colno);
            *len = sizeof(long);
            *ptr = (char *)emalloc(*len);
            memmove(*ptr, &t, *len);
            break;
        }
        case PDO_NUODB_SQLTYPE_TIMESTAMP:
        {
            unsigned long ts = pdo_nuodb_stmt_get_timestamp(S, colno);
            *len = sizeof(long);
            *ptr = (char *)emalloc(*len);
            memmove(*ptr, &ts, *len);
            break;
        }
    }

    return 1;

}
/* }}} */

static int nuodb_stmt_param_hook(pdo_stmt_t * stmt, struct pdo_bound_param_data * param, /* {{{ */
                                 enum pdo_param_event event_type TSRMLS_DC)
{
    pdo_nuodb_stmt * S = (pdo_nuodb_stmt *)stmt->driver_data;
    nuo_params * nuodb_params = param->is_param ? S->in_params : S->out_params;
    nuo_param * nuodb_param = NULL;

    if (event_type == PDO_PARAM_EVT_FREE)   /* not used */
    {
        return 1;
    }

    if (!nuodb_params || param->paramno >= nuodb_params->num_params)
    {
        strcpy_s(stmt->error_code, 5, "HY093");
        pdo_nuodb_db_handle_set_last_app_error(S->H, "Invalid parameter index");
        return 0;
    }

    if (param->is_param && param->paramno == -1)
    {
        long * index;

        /* try to determine the index by looking in the named_params hash */
        if (SUCCESS == zend_hash_find(S->named_params, param->name, param->namelen+1, (void **)&index))
        {
            param->paramno = *index;
        }
        else
        {
            // TODO: or by looking in the input descriptor
            // for now, just return an error
            strcpy_s(stmt->error_code, 5, "HY000");
            pdo_nuodb_db_handle_set_last_app_error(S->H, "Unable to determine the parameter index");
            return 0;
        }
    }

    nuodb_param = &nuodb_params->params[param->paramno];

	switch (event_type) {
		char *value;
		unsigned long value_len;
		int caller_frees;

		case PDO_PARAM_EVT_ALLOC:
			if (param->is_param) {
				/* allocate the parameter */
				if (nuodb_param->data) {
					efree(nuodb_param->data);
				}
				nuodb_param->data = (char *) emalloc(nuodb_param->len);
			}
			break;

		case PDO_PARAM_EVT_FREE:
		case PDO_PARAM_EVT_EXEC_POST:
		case PDO_PARAM_EVT_FETCH_PRE:
			/* do nothing */
			break;

		case PDO_PARAM_EVT_NORMALIZE:
			if (!param->is_param) {
				break;
			}
            if (param->paramno == -1) {
                return 0;
            }
            switch(param->param_type) {
                case PDO_PARAM_INT:
                {
                    nuodb_param->sqltype = PDO_NUODB_SQLTYPE_INTEGER;
                    break;
                }
                case PDO_PARAM_STR:
                {
                    nuodb_param->sqltype = PDO_NUODB_SQLTYPE_STRING;
                    break;
                }
            }
            break;

		case PDO_PARAM_EVT_EXEC_PRE:
			if (!param->is_param) {
				break;
			}
            if (param->paramno == -1) {
                return 0;
            }

			//*nuodb_param->sqlind = 0;

			switch (nuodb_param->sqltype & ~1) {
				case PDO_NUODB_SQLTYPE_ARRAY:
					strcpy_s(stmt->error_code, 5, "HY000");
					pdo_nuodb_db_handle_set_last_app_error(S->H, "Cannot bind to array field");
					return 0;

			}

			switch (nuodb_param->sqltype) {
			    case PDO_NUODB_SQLTYPE_INTEGER: {
                    pdo_nuodb_stmt_set_integer(S, param->paramno,  Z_LVAL_P(param->parameter));
                    break;
			    }
			    case PDO_NUODB_SQLTYPE_STRING: {
                    pdo_nuodb_stmt_set_string(S, param->paramno,  Z_STRVAL_P(param->parameter));
                    break;
			    }
			    default: {
					strcpy_s(stmt->error_code, 5, "HY000");
					pdo_nuodb_db_handle_set_last_app_error(S->H, "Cannot bind unsupported type!");
					return 0;
					break;
			    }
			}

#if 0  //TODO - Shutoff NULL stuff for now
			/* check if a NULL should be inserted */
			switch (Z_TYPE_P(param->parameter)) {
				int force_null;

				case IS_LONG:
					nuodb_param->sqltype = sizeof(long) == 8 ? SQL_INT64 : SQL_LONG;
					nuodb_param->data = (void*)&Z_LVAL_P(param->parameter);
					nuodb_param->len = sizeof(long);
					break;
				case IS_DOUBLE:
					nuodb_param->sqltype = SQL_DOUBLE;
					nuodb_param->data = (void*)&Z_DVAL_P(param->parameter);
					nuodb_param->len = sizeof(double);
					break;
				case IS_STRING:
					force_null = 0;

					/* for these types, an empty string can be handled like a NULL value */
					switch (nuodb_param->sqltype & ~1) {
						case SQL_SHORT:
						case SQL_LONG:
						case SQL_INT64:
						case SQL_FLOAT:
						case SQL_DOUBLE:
						case SQL_TIMESTAMP:
						case SQL_TYPE_DATE:
						case SQL_TYPE_TIME:
							force_null = (Z_STRLEN_P(param->parameter) == 0);
					}
					if (!force_null) {
						nuodb_param->sqltype = SQL_TEXT;
						nuodb_param->data = Z_STRVAL_P(param->parameter);
						nuodb_param->len	 = Z_STRLEN_P(param->parameter);
						break;
					}
				case IS_NULL:
					/* complain if this field doesn't allow NULL values */
					if (~nuodb_param->sqltype & 1) {
						strcpy(stmt->error_code, "HY105");
						S->H->last_app_error = "Parameter requires non-null value";
						return 0;
					}
					//*nuodb_param->sqlind = -1;
					break;
				default:
					strcpy(stmt->error_code, "HY105");
					S->H->last_app_error = "Binding arrays/objects is not supported";
					return 0;
			}
#endif // end of #if 0  //TODO - Shutoff NULL stuff for now

			break;

		case PDO_PARAM_EVT_FETCH_POST:
                        if (param->paramno == -1) {
                            return 0;
                        }
			if (param->is_param) {
				break;
			}
			value = NULL;
			value_len = 0;
			caller_frees = 0;

			if (nuodb_stmt_get_col(stmt, param->paramno, &value, &value_len, &caller_frees TSRMLS_CC)) {
				switch (PDO_PARAM_TYPE(param->param_type)) {
					case PDO_PARAM_STR:
						if (value) {
							ZVAL_STRINGL(param->parameter, value, value_len, 1);
							break;
						}
					case PDO_PARAM_INT:
						if (value) {
							ZVAL_LONG(param->parameter, *(long*)value);
							break;
						}
                    case PDO_PARAM_EVT_NORMALIZE:
                        if (!param->is_param) {
                            char *s = param->name;
                            while (*s != '\0') {
                               *s = toupper(*s);
                               s++;
                            }
                        }
                        break;
					default:
						ZVAL_NULL(param->parameter);
				}
				if (value && caller_frees) {
					efree(value);
				}
				return 1;
			}
			return 0;
		default:
			;
	}

    return 1;
}
/* }}} */

static int nuodb_stmt_set_attribute(pdo_stmt_t * stmt, long attr, zval * val TSRMLS_DC) /* {{{ */
{
    pdo_nuodb_stmt * S = (pdo_nuodb_stmt *)stmt->driver_data;

    switch (attr)
    {
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

static int nuodb_stmt_get_attribute(pdo_stmt_t * stmt, long attr, zval * val TSRMLS_DC) /* {{{ */
{
    pdo_nuodb_stmt * S = (pdo_nuodb_stmt *)stmt->driver_data;

    switch (attr)
    {
    default:
        return 0;
    case PDO_ATTR_CURSOR_NAME:
        if (*S->name)
        {
            ZVAL_STRING(val,S->name,1);
        }
        else
        {
            ZVAL_NULL(val);
        }
        break;
    }
    return 1;
}
/* }}} */

static int nuodb_stmt_cursor_closer(pdo_stmt_t * stmt TSRMLS_DC) /* {{{ */
{
    pdo_nuodb_stmt * S = (pdo_nuodb_stmt *)stmt->driver_data;

    if ((*S->name || S->cursor_open))
    {
        _release_PdoNuoDbStatement(S);
    }
    *S->name = 0;
    S->cursor_open = 0;
    return 1;
}
/* }}} */


struct pdo_stmt_methods nuodb_stmt_methods =   /* {{{ */
{
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

