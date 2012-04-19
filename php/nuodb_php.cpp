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

using namespace std;

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_nuodb.h"

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
	PHP_FE(nuodb_fetch_array,	NULL)
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
      php_error(E_NOTICE, "Got exception: %s", xcp.getText());
 }
  catch (...) {
   if (NUODB_G(debug))
       php_error(E_NOTICE, "Got generic exception");
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
 if (NUODB_G(trace))
   php_error(E_NOTICE, "nuodb_free_connection: connection (%p)", nc);
 TSRMLS_FETCH(); 
 try {
  nuodb_free_results(nc);
  nc->c->close();
  conn_map.erase(nc->hash);
  delete nc;
 }
  catch (SQLException& xcp) {
    if (NUODB_G(debug))
      php_error(E_NOTICE, "Got exception: %s", xcp.getText());
 }
  catch (...) {
   if (NUODB_G(debug))
       php_error(E_NOTICE, "Got generic exception");
  }
}

static nuodb_connection_t *nuodb_find_connection_by_hash(string hash)
{
 conn_map_t::iterator it = conn_map.find(hash);
 if (it != conn_map.end())
   return conn_map[hash];
 return 0; 
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
        zend_error(E_NOTICE, "Got exception: %s", xcp.getText());
   }
    catch (...) {
       if (NUODB_G(debug))
         zend_error(E_NOTICE, "Got generic exception");
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
		return;
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
	        nc->error = xcp.getText();
                if (NUODB_G(debug))
	          php_error(E_NOTICE, "Got exception: %s", xcp.getText());
	        return;
         }
           catch (...) {
             nc->error = "Got generic exception";
             if (NUODB_G(debug))
               php_error(E_NOTICE, "Got generic exception");
	     return;
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
      php_error(E_NOTICE, "Got exception: %s", nc->error.c_str());
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
   php_error(E_NOTICE, "nuodb_free_result: res - (%p)", res);
   nuodb_result_t *r = (nuodb_result_t *) Z_RESVAL_P(res);
 try {
    nuodb_free_result(r);
 }
  catch (SQLException& xcp) {
    if (NUODB_G(debug))
      php_error(E_NOTICE, "Got exception: %s", xcp.getText());
 }
  catch (...) {
     if (NUODB_G(debug))
       php_error(E_NOTICE, "Got generic exception");
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
      RETURN_FALSE;
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

// nuodb_fetch_array (resource result)

PHP_FUNCTION(nuodb_fetch_array)
{
 zval *res = NULL;
 if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"r", &res) == FAILURE)
   RETURN_FALSE;
 if (res) {
   nuodb_result_t *r = (nuodb_result_t *) Z_RESVAL_P(res);
 try {
    if (NUODB_G(debug))
      php_error(E_NOTICE, "nuodb_fetch_array: connection (%p), result (%p)",
        r->nc, r);
    ResultSetMetaData *md = r->rs->getMetaData();
    if (r->rs->next()) {
      unsigned long columns = md->getColumnCount();
      if (NUODB_G(trace))
        php_error(E_NOTICE, "nuodb_fetch_array: columns (%ld)", columns);
      array_init(return_value);
      for (unsigned long i = 1; i <= columns; i++) {
        const char *col = md->getColumnName(i);
        zval *data;
        MAKE_STD_ZVAL(data);
        const char *row_val = r->rs->getString(i);
        bool isNull = r->rs->wasNull();
        if (NUODB_G(trace))
         php_error(E_NOTICE, "nuodb_fetch_array: %ld column (%s), val (%s)", i, col,
           isNull ? "NULL" : row_val);
        if (isNull)
          add_assoc_null(return_value, col);
        else {
          ZVAL_STRING(data, row_val, 1);
          add_assoc_zval(return_value, col, data);
        }
      }
    } else {
        if (NUODB_G(trace))
          php_error(E_NOTICE, "nuodb_fetch_array: no data to fetch for connection (%p) and result (%p)",
           r->nc, r);
        nuodb_free_result(r);
        RETURN_FALSE;
      }
 }
  catch (SQLException& xcp) {
    r->nc->error = xcp.getText();
    if (NUODB_G(debug))
      php_error(E_NOTICE, "Got exception: %s", xcp.getText());
 }
  catch (...) {
     r->nc->error = "Got generic exception";
     if (NUODB_G(debug))
       php_error(E_NOTICE, "Got generic exception");
  }
 }
}
