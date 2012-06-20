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

#include <time.h>
#include <sys/stat.h>
#ifdef WIN32
#include <sys/utime.h>
#endif

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <cctype>
#include <algorithm>

using namespace std;

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_nuodb.h"

using namespace NuoDB;

#ifdef ZEND_ENGINE_2
#include "zend_exceptions.h"
#endif

#define LOWER_COLNAMES	1

#define NUODB_ASSOC	(1 << 0)
#define NUODB_NUM	(1 << 1)
#define NUODB_BOTH	(NUODB_ASSOC | NUODB_NUM)

ZEND_DECLARE_MODULE_GLOBALS(nuodb)

/* {{{ nuodb_functions[]
 *
 * Every user visible function must have an entry in nuodb_functions[].
 */
const zend_function_entry nuodb_functions[] = {
	PHP_FE(nuodb_connect,	NULL)
	PHP_FE(nuodb_disconnect,	NULL)
	PHP_FE(nuodb_query,	NULL)
	PHP_FE(nuodb_error,	NULL)
	PHP_FE(nuodb_free_result,	NULL)
	PHP_FE(nuodb_fetch_row,	NULL)
	PHP_FE(nuodb_fetch_array,	NULL)
	PHP_FE(nuodb_fetch_object,	NULL)
	PHP_FE(nuodb_fetch_assoc,	NULL)
	PHP_FE(nuodb_autocommit,       NULL)
	{ NULL, NULL, NULL}
/*	PHP_FE_END	Must be the last line in nuodb_functions[] */
};
/* }}} */

#define STANDARD_NUODB_MODULE_PROPERTIES_EX 0, 0, NULL, 0, (char *)(ZEND_MODULE_BUILD_ID)
#define STANDARD_NUODB_MODULE_PROPERTIES \
        NO_MODULE_GLOBALS, NULL, STANDARD_NUODB_MODULE_PROPERTIES_EX

/* {{{ nuodb_module_entry
 */
zend_module_entry nuodb_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	(char *)"php_nuodb",
	nuodb_functions,
	PHP_MINIT(nuodb),
	PHP_MSHUTDOWN(nuodb),
	PHP_RINIT(nuodb),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(nuodb),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(nuodb),
#if ZEND_MODULE_API_NO >= 20010901
	(char *)"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_NUODB_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_NUODB
ZEND_GET_MODULE(nuodb)
#endif

char debug_entry[] = "nuodb.debug";
char trace_entry[] = "nuodb.trace";
char default_entry_val[] = "0";

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY(debug_entry, default_entry_val, PHP_INI_ALL, OnUpdateLong, debug, zend_nuodb_globals, nuodb_globals)
    STD_PHP_INI_ENTRY(trace_entry, default_entry_val, PHP_INI_ALL, OnUpdateLong, trace, zend_nuodb_globals, nuodb_globals)
PHP_INI_END()
/* }}} */


/* {{{ php_nuodb_init_globals
 */
static void php_nuodb_init_globals(zend_nuodb_globals *nuodb_globals)
{
     nuodb_globals->debug = 0;
     nuodb_globals->trace = 0;
}
 /* }}} */

// for connection the current statement and the current result pointers
// need to be stored for further freeing

struct nuodb_connection_t;

struct nuodb_result_t {
  nuodb_connection_t *nc;
  ResultSet *rs;
  Statement *stmt;
  vector <nuodb_result_t *>::iterator it; // pointer to connection r vector element
};

struct nuodb_connection_t {
  string hash, error;
  Connection *c;
  bool autocommit;
  // one connection can handle multiple statements and results
  vector <nuodb_result_t *> r;
};

typedef map <string, nuodb_connection_t *> conn_map_t;

conn_map_t conn_map;

typedef map <nuodb_result_t *, vector <nuodb_result_t *>::iterator> results_map_t;

results_map_t results_map;

nuodb_connection_t *default_connection;
string default_error;

static void nuodb_set_error(nuodb_connection_t *nc, string err)
{
 if (!nc) {
  if (default_connection)
    nc = default_connection;
  else
    default_error = err;
 } else
     nc->error = err;
}

static nuodb_connection_t *nuodb_create_connection(string hash,
  bool autocommit = true)
{
 TSRMLS_FETCH(); 
 nuodb_connection_t *nuodb_conn = new (nuodb_connection_t);
 nuodb_conn->hash = hash;
 nuodb_conn->c = createConnection();
 nuodb_conn->autocommit = autocommit;
 conn_map[hash] = nuodb_conn;
 if (NUODB_G(trace))
   php_error(E_NOTICE, "nuodb_create_connection: (%p), results size before (%ld)",
     nuodb_conn, nuodb_conn->r.size()); 
 return nuodb_conn;
}

// below is the correct result freeing order
// or you will have the transaction process crash

static void nuodb_free_result(nuodb_result_t *r, bool total_delete = true)
{
 TSRMLS_FETCH(); 
 // check if result is valid
 if (!r)
   return;
 results_map_t::iterator it = results_map.find(r);
 if (it == results_map.end())
   return;
 if (!r->nc || !r->nc->r.size())
   return;
 if (NUODB_G(trace))
   php_error(E_NOTICE, "nuodb_free_result: connection (%p), result (%p), total results (%ld)",
     r->nc, r, r->nc->r.size());
 try {
  if (r->rs) {
    r->rs->close();
    r->rs = 0;
  }
  if (r->nc->autocommit)
    r->nc->c->commit();
  if (r->stmt) {
    r->stmt->close();
    r->stmt = 0;
  }
  // delete result from results vector
  if (total_delete)
    r->nc->r.erase(r->it);
  results_map.erase(r);
  delete r;
 }
  catch (SQLException& xcp) {
    if (NUODB_G(debug))
      php_error(E_NOTICE, "%s: Got exception: %s", (__FUNCTION__ + 4), xcp.getText());
 }
  catch (...) {
   if (NUODB_G(debug))
       php_error(E_NOTICE, "%s: Got generic exception", (__FUNCTION__ + 4));
  }
}

static void nuodb_free_results(nuodb_connection_t *nc)
{
 TSRMLS_FETCH(); 
 if (NUODB_G(trace))
     zend_error(E_NOTICE, "nuodb_free_results: connection (%p), results count (%ld)", nc,
       nc->r.size());
 // iterate through all results in connection
 for (vector <nuodb_result_t *>::iterator it = nc->r.begin(); it != nc->r.end(); it++)
   nuodb_free_result(*it, false);
 nc->r.clear();
}

static void nuodb_add_result(nuodb_connection_t *nc, nuodb_result_t *r)
{
 r->it = nc->r.insert(nc->r.begin(), r);
 results_map[r] = r->it;
}

static void nuodb_free_connection(nuodb_connection_t *nc)
{
 TSRMLS_FETCH(); 
 if (NUODB_G(trace))
  php_error(E_NOTICE, "nuodb_free_connection: connection (%p)", nc);

 try {
  nuodb_free_results(nc);
  nc->c->close();
  conn_map.erase(nc->hash);
  delete nc;
 }
  catch (SQLException& xcp) {
    if (NUODB_G(debug))
      php_error(E_NOTICE, "%s: Got exception: %s", (__FUNCTION__ + 4), xcp.getText());
 }
  catch (...) {
   if (NUODB_G(debug))
       php_error(E_NOTICE, "%s: Got generic exception", (__FUNCTION__ + 4));
  }
}

static nuodb_connection_t *nuodb_find_connection_by_hash(string hash)
{
 conn_map_t::iterator it = conn_map.find(hash);
 if (it != conn_map.end())
   return conn_map[hash];
 return 0; 
}

static void php_nuodb_fetch_hash(INTERNAL_FUNCTION_PARAMETERS, int result_type, int expected_args,
  bool into_object)
{
        zval *res, *ctor_params = NULL;
        zend_class_entry *ce = NULL;

#ifdef ZEND_ENGINE_2
        if (into_object) {
                char *class_name = NULL;
                int class_name_len = 0;

                if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"z|sz", &res, &class_name, &class_name_len, &ctor_params) == FAILURE) {
                        return;
                }
                if (ZEND_NUM_ARGS() < 2) {
                        ce = zend_standard_class_def;
                } else {
                        ce = zend_fetch_class(class_name, class_name_len, ZEND_FETCH_CLASS_AUTO TSRMLS_CC);
                }
                if (!ce) {
                        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not find class '%s'", class_name);
                        return;
                }
                result_type = NUODB_ASSOC;
        } else
#endif
        {
                if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"r|l", &res, &result_type) == FAILURE) {
                        return;
                }
                if (!result_type) {
                        /* result_type might have been set outside, so only overwrite when not set */
                        result_type = NUODB_BOTH;
                }
        }

        if (result_type & ~NUODB_BOTH) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "The result type should be either NUODB_NUM, NUODB_ASSOC or NUODB_BOTH");
                result_type = NUODB_BOTH;
        }
        
        nuodb_result_t *r = (nuodb_result_t *) Z_RESVAL_P(res);
        if (!r) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "The nuodb_result must be specified !");
                RETURN_FALSE;
        }
  try {
        if (!r->rs->next())
          RETURN_FALSE;

        array_init(return_value);

        ResultSetMetaData *md = r->rs->getMetaData();
        unsigned long columns = md->getColumnCount();
        
        for (unsigned long i = 1; i <= columns; i++) {

	  string c_name(md->getColumnName(i));
#ifdef LOWER_COLNAMES
	  transform(c_name.begin(), c_name.end(), c_name.begin(), ::tolower);
#endif
          const char *col_name = c_name.c_str();
          
          const char *row_val = r->rs->getString(i);
          bool isNull = r->rs->wasNull();
 
                if (!isNull) { // not NULL
                        zval *data;
                        MAKE_STD_ZVAL(data);
                        ZVAL_STRING(data, row_val, 1);
                        // ZVAL_STRINGL(data, row_val, md->getCurrentColumnMaxLength(i), 1);
                        if (result_type & NUODB_NUM) {
                                add_index_zval(return_value, i, data);
                        }
                        if (result_type & NUODB_ASSOC) {
                                if (result_type & NUODB_NUM) {
                                        Z_ADDREF_P(data);
                                }
                                add_assoc_zval(return_value, col_name, data);
                        }
                } else {
                        /* NULL value. */
                        if (result_type & NUODB_NUM) {
                                add_index_null(return_value, i);
                        }

                        if (result_type & NUODB_ASSOC) {
                                add_assoc_null(return_value, col_name);
                        }
                }
        }
    }
      catch (SQLException& xcp) {
        r->nc->error = xcp.getText();
        if (NUODB_G(debug))
          php_error(E_NOTICE, "%s: Got exception: %s", (__FUNCTION__ + 4), xcp.getText());
     }
      catch (...) {
         r->nc->error = "Got generic exception";
         if (NUODB_G(debug))
           php_error(E_NOTICE, "%s: Got generic exception", (__FUNCTION__ + 4));
     }

#ifdef ZEND_ENGINE_2
        /* mysqlnd might return FALSE if no more rows */
        if (into_object && Z_TYPE_P(return_value) != IS_BOOL) {
                zval dataset = *return_value;
                zend_fcall_info fci;
                zend_fcall_info_cache fcc;
                zval *retval_ptr; 
        
                object_and_properties_init(return_value, ce, NULL);
                zend_merge_properties(return_value, Z_ARRVAL(dataset), 1 TSRMLS_CC);
        
                if (ce->constructor) {
                        fci.size = sizeof(fci);
                        fci.function_table = &ce->function_table;
                        fci.function_name = NULL;
                        fci.symbol_table = NULL;
                        fci.object_ptr = return_value;
                        fci.retval_ptr_ptr = &retval_ptr;
                        if (ctor_params && Z_TYPE_P(ctor_params) != IS_NULL) {
                                if (Z_TYPE_P(ctor_params) == IS_ARRAY) {
                                        HashTable *ht = Z_ARRVAL_P(ctor_params);
                                        Bucket *p;
        
                                        fci.param_count = 0;
                                        fci.params = (zval ***) safe_emalloc(sizeof(zval*), ht->nNumOfElements, 0);
                                        p = ht->pListHead;
                                        while (p != NULL) {
                                                fci.params[fci.param_count++] = (zval**)p->pData;
                                                p = p->pListNext;
                                        }
                                } else {
                                        /* Two problems why we throw exceptions here: PHP is typeless
                                         * and hence passing one argument that's not an array could be
                                         * by mistake and the other way round is possible, too. The 
                                         * single value is an array. Also we'd have to make that one
                                         * argument passed by reference.
                                         */
                                        zend_throw_exception(zend_exception_get_default(TSRMLS_C), (char *)"Parameter ctor_params must be an array", 0 TSRMLS_CC);
                                        return;
                                }
                        } else {
                                fci.param_count = 0;
                                fci.params = NULL;
                        }
                        fci.no_separation = 1;

                        fcc.initialized = 1;
                        fcc.function_handler = ce->constructor;
                        fcc.calling_scope = EG(scope);
                        fcc.called_scope = Z_OBJCE_P(return_value);
                        fcc.object_ptr = return_value;
                
                        if (zend_call_function(&fci, &fcc TSRMLS_CC) == FAILURE) {
                                zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC, (char *)"Could not execute %s::%s()", ce->name, ce->constructor->common.function_name);
                        } else {
                                if (retval_ptr) {
                                        zval_ptr_dtor(&retval_ptr);
                                }
                        }
                        if (fci.params) {
                                efree(fci.params);
                        }
                } else if (ctor_params) {
                        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC, (char *)"Class %s does not have a constructor hence you cannot use ctor_params", ce->name);
                }
        }
#endif
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(nuodb)
{
  ZEND_INIT_MODULE_GLOBALS(nuodb, php_nuodb_init_globals, NULL); 
  REGISTER_INI_ENTRIES();
  if (NUODB_G(trace))
    zend_error(E_NOTICE, "NuoDB MINIT called");
  // NUODB_G(num_persistent) = 0;
  // NUODB_G(connect_timeout) = 0;
  Z_TYPE(nuodb_module_entry) = type;
  return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(nuodb)
{
 if (NUODB_G(trace))
   zend_error(E_NOTICE, "NuoDB MSHUTDOWN called");
 UNREGISTER_INI_ENTRIES();
 return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(nuodb)
{
 if (NUODB_G(trace))
   zend_error(E_NOTICE, "NuoDB RINIT called");
 default_connection = 0;
 return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(nuodb)
{
  if (NUODB_G(trace))
    zend_error(E_NOTICE, "NuoDB RSHUTDOWN called");
  if (default_connection) {
    try {
     nuodb_free_connection(default_connection);
   }
    catch (SQLException& xcp) {
      if (NUODB_G(debug))
        zend_error(E_NOTICE, "%s: Got exception: %s", (__FUNCTION__ + 4), xcp.getText());
   }
    catch (...) {
       if (NUODB_G(debug))
         zend_error(E_NOTICE, "%s: Got generic exception", (__FUNCTION__ + 4));
    }
 }
  return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(nuodb)
{
 php_info_print_table_start();
 php_info_print_table_header(2, "nuodb support", "enabled");
 php_info_print_table_end();
 DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ proto  nuodb_connect()
    */
PHP_FUNCTION(nuodb_connect)
{
  hash <const char *> Hash;
  int user_len, passwd_len, database_len, schema_len;
	char *user=(char *)"", *passwd=(char *)"", *database = NULL, *schema = (char *)"";
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"s|sss", &database, &database_len,
	  &user, &user_len, &passwd, &passwd_len, &schema, &schema_len) == FAILURE) {
		RETURN_FALSE;
	}
	if (NUODB_G(debug))
  	  php_error(E_NOTICE, "nuodb_connect: db (%s), user (%s), pass (%s), schema (%s)",
	   database, user, passwd, schema);
	stringstream conn_hash;
	conn_hash << Hash(database);
	conn_hash << Hash(user);
	conn_hash << Hash(passwd);
	conn_hash << Hash(schema);
	nuodb_connection_t *nc = nuodb_find_connection_by_hash(conn_hash.str());
	// check if have already connection to this database
	if (!nc) { // add connection
	  try {
          nc = nuodb_create_connection(conn_hash.str());
          Properties *properties = nc->c->allocProperties();
          if (strlen(user))
  	    properties->putValue("user", user);
          if (strlen(passwd))
	    properties->putValue("password", passwd);
          if (strlen(schema))
  	    properties->putValue("schema", schema);
	    nc->c->openDatabase(database, properties);
	    if (NUODB_G(trace))
	      php_error(E_NOTICE, "nuodb_connect: new connection (%p)", nc);
	  } catch (SQLException& xcp) {
	        nuodb_set_error(0, xcp.getText());
	        if (NUODB_G(debug))
                  php_error(E_NOTICE, "%s: Got exception: %s", (__FUNCTION__ + 4), xcp.getText());
                if (nc)
                 nuodb_free_connection(nc);
	        RETURN_FALSE;
         }
           catch (...) {
             nuodb_set_error(0, "Got generic exception");
             if (NUODB_G(debug))
               php_error(E_NOTICE, "%s: Got generic exception", (__FUNCTION__ + 4));
             if (nc)
               nuodb_free_connection(nc);
	     RETURN_FALSE;
         }
          if (!default_connection)
            default_connection = nc;
	  Z_LVAL_P(return_value) = (long)nc;
	} else { // return connection
	  if (NUODB_G(trace))
  	    php_error(E_NOTICE, "nuodb_connect: old connection (%p)", nc);
	  Z_LVAL_P(return_value) = (long)nc;
      }
	Z_TYPE_P(return_value) = IS_RESOURCE;
}
/* }}} */

/* {{{ proto  nuodb_disconnect()
    */
PHP_FUNCTION(nuodb_disconnect)
{
  zval *nuodb_link=NULL;
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"r", &nuodb_link) == FAILURE) {
    return;
 }
  long conn = Z_RESVAL_P(nuodb_link);
  nuodb_connection_t *nc = (nuodb_connection_t *) conn;
  if (NUODB_G(debug))
    php_error(E_NOTICE, "nuodb_disconnect: connection (%p)", nc);
  if (nc == default_connection)
    default_connection = 0;
  nuodb_free_connection(nc);
}
/* }}} */

// nuodb_query(string query[, resource connection])

PHP_FUNCTION(nuodb_query)
{
 char *query;
 int query_len;
 zval *nuodb_link = NULL;
 nuodb_connection_t *nc;

 if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"s|r", &query,
   &query_len, &nuodb_link) == FAILURE) {
     RETURN_FALSE;
 }
 if (nuodb_link)
   nc = (nuodb_connection_t *) Z_RESVAL_P(nuodb_link);
 else
   nc = 0;
 if (!nc) {
   if (default_connection)
     nc = default_connection;
   else { // no connection
     if (NUODB_G(debug))
       php_error(E_NOTICE, "nuodb_query: No connection found !");
     RETURN_FALSE;
   }
 }
  if (NUODB_G(debug))
    php_error(E_NOTICE, "nuodb_query: (%s) on connection (%p)", query, nc);
  nuodb_result_t *r = 0;
  try {
    // check for statement
    if (!strncasecmp(query, "commit", 6)) {
       nc->c->commit();
       RETURN_TRUE;
    } else if (!strncasecmp(query, "rollback", 8)) {
       nc->c->rollback();
       RETURN_TRUE;
    } else if (!strncasecmp(query, "select", 6)) {
       PreparedStatement *st = nc->c->prepareStatement(query);
       r = new nuodb_result_t;
       r->nc = nc;
       r->rs = st->executeQuery();
       r->stmt = st;
    } else if (!strncasecmp(query, "drop", 4) ||
         !strncasecmp(query, "create", 6) || !strncasecmp(query, "insert", 6) ||
         !strncasecmp(query, "delete", 6)) {
       Statement *stmt = nc->c->createStatement();
       stmt->execute(query);
       if (nc->autocommit)
         nc->c->commit();
       stmt->close();
       // what to return here ?
       RETURN_TRUE;
   }
    nuodb_add_result(nc, r);
    Z_LVAL_P(return_value) = (long) r;
    if (NUODB_G(trace))
     php_error(E_NOTICE, "nuodb_query: new result (%p)", r);
    Z_TYPE_P(return_value) = IS_RESOURCE;
  }
  catch (SQLException& xcp) {
    if (r)
      delete r;
    nc->error = xcp.getText();
    if (NUODB_G(debug))
      php_error(E_NOTICE, "%s: Got exception: %s", (__FUNCTION__ + 4), nc->error.c_str());
    // if invalid connection we will get second exception and abort here
    if (strncasecmp("remote connection closed", nc->error.c_str(), 24))
      nc->c->rollback();
    RETURN_FALSE;
  }
  catch (...) {
     if (r)
       delete r;
     nc->error = "Got generic exception";
     if (NUODB_G(debug))
       php_error(E_NOTICE, "Got generic exception");
       RETURN_FALSE;
  }
}

// nuodb_free_result (resource result)

PHP_FUNCTION(nuodb_free_result)
{
 zval *res = NULL;
 if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"r", &res) == FAILURE)
   return;
 if (res) {
  if (NUODB_G(debug))
    php_error(E_NOTICE, "nuodb_free_result: res - (%p)", res);
   nuodb_result_t *r = (nuodb_result_t *) Z_RESVAL_P(res);
 try {
    nuodb_free_result(r);
 }
  catch (SQLException& xcp) {
    if (NUODB_G(debug))
      php_error(E_NOTICE, "%s: Got exception: %s", (__FUNCTION__ + 4), xcp.getText());
 }
  catch (...) {
     if (NUODB_G(debug))
       php_error(E_NOTICE, "%s: Got generic exception", (__FUNCTION__ + 4));
  }
 }
}

// nuodb_error([resource connection])

PHP_FUNCTION(nuodb_error)
{
  zval *nuodb_link=NULL;
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"|r", &nuodb_link) == FAILURE) {
    RETURN_FALSE;
 }
  nuodb_connection_t *nc;
  if (nuodb_link)
    nc = (nuodb_connection_t *) Z_RESVAL_P(nuodb_link);
  else {
    if (default_connection)
      nc = default_connection;
    else
      RETURN_STRING(default_error.c_str(), 1);
  }
  if (NUODB_G(debug))
    php_error(E_NOTICE, "nuodb_error: connection (%p), error (%s)", nc,
      nc->error.c_str());
  RETURN_STRING(nc->error.c_str(), 1); // why do we need to copy string here ?
}

// nuodb_autocommit (int val[, resource connection]

PHP_FUNCTION(nuodb_autocommit)
{
}

PHP_FUNCTION(nuodb_fetch_array)
{
  php_nuodb_fetch_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0, 2, 0);
}

PHP_FUNCTION(nuodb_fetch_row)
{
  php_nuodb_fetch_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU, NUODB_NUM, 1, 0);
}

PHP_FUNCTION(nuodb_fetch_object)
{
  php_nuodb_fetch_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU, NUODB_ASSOC, 2, 1);
  if (Z_TYPE_P(return_value) == IS_ARRAY)
    object_and_properties_init(return_value, ZEND_STANDARD_CLASS_DEF_PTR, Z_ARRVAL_P(return_value));
}

PHP_FUNCTION(nuodb_fetch_assoc)
{
  php_nuodb_fetch_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU, NUODB_ASSOC, 1, 0);
}
