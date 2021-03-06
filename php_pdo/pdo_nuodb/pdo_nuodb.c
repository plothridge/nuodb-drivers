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
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/pdo/php_pdo.h"
#include "ext/pdo/php_pdo_driver.h"
#include "php_pdo_nuodb.h"

#include "php_pdo_nuodb_c_cpp_common.h"
#include "php_pdo_nuodb_int.h"


/* If you declare any globals in php_pdo_nuodb.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(pdo_nuodb)
*/

/* True global resources - no need for thread safety here */
static int le_pdo_nuodb;

/* {{{ pdo_nuodb_functions[]
*
* Every user visible function must have an entry in pdo_nuodb_functions[].
*/
const zend_function_entry pdo_nuodb_functions[] =
{
    PHP_FE(confirm_pdo_nuodb_compiled,	NULL)		/* TODO: For testing, remove later. */
    PHP_FE_END	/* Must be the last line in pdo_nuodb_functions[] */
};
/* }}} */

/* {{{ pdo_nuodb_deps
*/
#if ZEND_MODULE_API_NO >= 20050922
static const zend_module_dep pdo_nuodb_deps[] =
{
    ZEND_MOD_REQUIRED("pdo")
    ZEND_MOD_END
};
#endif
/* }}} */

/* {{{ pdo_nuodb_module_entry
*/
zend_module_entry pdo_nuodb_module_entry =
{
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER_EX, NULL,
    pdo_nuodb_deps,
#endif
    "pdo_nuodb",
    pdo_nuodb_functions,
    PHP_MINIT(pdo_nuodb),
    PHP_MSHUTDOWN(pdo_nuodb),
    PHP_RINIT(pdo_nuodb),		/* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(pdo_nuodb),	/* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(pdo_nuodb),
#if ZEND_MODULE_API_NO >= 20010901
    "0.1", /* Replace with version number for your extension */
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PDO_NUODB
ZEND_GET_MODULE(pdo_nuodb)
#endif

/* {{{ PHP_INI
*/
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
STD_PHP_INI_ENTRY("pdo_nuodb.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_pdo_nuodb_globals, pdo_nuodb_globals)
STD_PHP_INI_ENTRY("pdo_nuodb.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_pdo_nuodb_globals, pdo_nuodb_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_pdo_nuodb_init_globals
*/
/* Uncomment this function if you have INI entries
static void php_pdo_nuodb_init_globals(zend_pdo_nuodb_globals *pdo_nuodb_globals)
{
pdo_nuodb_globals->global_value = 0;
pdo_nuodb_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
*/
PHP_MINIT_FUNCTION(pdo_nuodb)
{
    /* If you have INI entries, uncomment these lines
    REGISTER_INI_ENTRIES();
    */
    php_pdo_register_driver(&pdo_nuodb_driver);
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(pdo_nuodb)
{
    /* uncomment this line if you have INI entries
    UNREGISTER_INI_ENTRIES();
    */
    php_pdo_unregister_driver(&pdo_nuodb_driver);
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
*/
PHP_RINIT_FUNCTION(pdo_nuodb)
{
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
*/
PHP_RSHUTDOWN_FUNCTION(pdo_nuodb)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(pdo_nuodb)
{
    php_info_print_table_start();
    // TODO: we should display version of the driver and version of NuoDB
    php_info_print_table_header(2, "PDO Driver for NuoDB", "enabled");
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}
/* }}} */


/* Remove the following function when you have succesfully modified config.m4
so that your module can be compiled into PHP, it exists only for testing
purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_pdo_nuodb_compiled(string arg)
Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_pdo_nuodb_compiled)
{
    char * arg = NULL;
    int arg_len, len;
    char * strg;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE)
    {
        return;
    }

    len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "pdo_nuodb", arg);
    RETURN_STRINGL(strg, len, 0);
}
/* }}} */
