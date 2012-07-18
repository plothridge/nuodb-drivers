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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include "php_pdo_nuodb_c_cpp_common.h"
#include "php_pdo_nuodb_cpp_int.h"

/* describes a column -- stolen from <path-to-php-sdk>/include/ext/pdo/php_pdo_driver.h */
struct pdo_column_data {
	char *name;
	int namelen;
	unsigned long maxlen;
	enum pdo_param_type param_type;
	unsigned long precision;

	/* don't touch this unless your name is dbdo */
	void *dbdo_data;
};


PdoNuoDbHandle::PdoNuoDbHandle(SqlOptionArray * options)
    : _con(NULL), _opts(NULL), _last_stmt(NULL)
{
    for (int i=0; i<4; i++)
    {
        _opt_arr[i].option = NULL;
        _opt_arr[i].extra = NULL;
    }
    setOptions(options);
}

PdoNuoDbHandle::~PdoNuoDbHandle()
{
    closeConnection();
    deleteOptions();
}

void PdoNuoDbHandle::deleteOptions()
{
    if (_opts == NULL)
    {
        return;
    }
    for (int i=0; i<4; i++)
    {
        if (_opt_arr[i].option != NULL)
        {
            delete _opt_arr[i].option;
            _opt_arr[i].option = NULL;
        }
        if (_opt_arr[i].extra != NULL)
        {
            delete (char *)_opt_arr[i].extra;
            _opt_arr[i].extra = NULL;
        }
    }
    delete _opts;
    _opts = NULL;
}

void PdoNuoDbHandle::setOptions(SqlOptionArray * options)
{
    deleteOptions();
    _opts = new SqlOptionArray;
    _opts->count = 4;
    _opts->array = _opt_arr;
    _opt_arr[0].option = (char const *) _strdup(options->array[0].option);
    _opt_arr[0].extra = (void *) _strdup((const char *)options->array[0].extra);
    _opt_arr[1].option = (char const *) _strdup(options->array[1].option);
    _opt_arr[1].extra = (void *) _strdup((const char *)options->array[1].extra);
    _opt_arr[2].option = (char const *) _strdup(options->array[2].option);
    _opt_arr[2].extra = (void *) _strdup((const char *)options->array[2].extra);
    _opt_arr[3].option = (char const *) _strdup(options->array[3].option);
    _opt_arr[3].extra = (void *) _strdup((const char *)options->array[3].extra);
}

NuoDB::Connection * PdoNuoDbHandle::createConnection()
{
    closeConnection();
    _con = NuoDB::Connection::create((const char *)_opts->array[0].extra,
                                     (const char *)_opts->array[1].extra,
                                     (const char *)_opts->array[2].extra,
                                     1,
                                     (const char *)_opts->array[3].option,
                                     (const char *)_opts->array[3].extra);
    //TODO add properties
    return _con;
}

NuoDB::Connection * PdoNuoDbHandle::getConnection()
{
    return _con;
}

PdoNuoDbStatement * PdoNuoDbHandle::createStatement(char const * sql)
{
    PdoNuoDbStatement * rval = NULL;
    if (sql == NULL)
    {
        return NULL;
    }
    rval = new PdoNuoDbStatement(this);
    rval->createStatement(sql);
    _last_stmt = rval;
    return rval;
}

void PdoNuoDbHandle::closeConnection()
{
    if (_con == NULL)
    {
        return;
    }
    _con->close();
    _con = NULL;
}

void PdoNuoDbHandle::commit()
{
    if (_con == NULL)
    {
        return;
    }
    _con->commit();
}

void PdoNuoDbHandle::rollback()
{
    if (_con == NULL)
    {
        return;
    }
    _con->rollback();
}

PdoNuoDbStatement::PdoNuoDbStatement(PdoNuoDbHandle * dbh) : _dbh(dbh), _stmt(NULL), _stmt_type(0), _rs(NULL)
{
    // empty
}

PdoNuoDbStatement::~PdoNuoDbStatement()
{
    if (_rs != NULL)
    {
        _rs->close();
    }
    _rs = NULL;
    if (_stmt != NULL)
    {
        _stmt->close();
    }
    _stmt = NULL;
}

NuoDB::PreparedStatement * PdoNuoDbStatement::createStatement(char const * sql)
{
    if (sql == NULL)
    {
        return NULL;
    }
    NuoDB::Connection * _con = NULL;
    _con = _dbh->getConnection();
    if (_con == NULL)
    {
        return NULL;
    }

    char up_sql[7];
    int i;
    for (i=0; i<6; i++) {
        if (sql[i] == '\0') break;
        up_sql[i] = toupper(sql[i]);
    }
    for (; i<7; i++) up_sql[i] = '\0';
    if (strncmp(up_sql, "SELECT", 6) == 0) _stmt_type = 1;
    if (strncmp(up_sql, "UPDATE", 6) == 0) _stmt_type = 2;

    try {
    _stmt = _con->prepareStatement(sql);
    _rs = NULL;
    } catch (NuoDB::SQLException & e) {
        int code = e.getSqlcode();
        const char *text = e.getText();
		nuodb_throw_zend_exception("HY000", code, text);
		
    } catch (...) {
		nuodb_throw_zend_exception("HY000", 0, "Unknown problem");
    }

    return _stmt;
}

void PdoNuoDbStatement::execute()
{
    if (_stmt == NULL)
    {
        return;
    }
    if (_stmt_type == 1) {
        _rs = _stmt->executeQuery();
    } else if (_stmt_type == 2) {
        _stmt->executeUpdate();
    } else {
        _stmt->execute();
    }
}

void PdoNuoDbStatement::executeQuery()
{
    if (_stmt == NULL)
    {
        return;
    }
    _rs = _stmt->executeQuery();
}

bool PdoNuoDbStatement::hasResultSet()
{
    return (_rs != NULL);
}

bool PdoNuoDbStatement::next()
{
    if (_rs == NULL)
    {
        return false;
    }
    return _rs->next();
}

size_t PdoNuoDbStatement::getColumnCount()
{
    if (_rs == NULL)
    {
        return false;
    }
    NuoDB::ResultSetMetaData * md = _rs->getMetaData();
    return md->getColumnCount();
}

char const * PdoNuoDbStatement::getColumnName(size_t column)
{
    char const * rval = NULL;
    if (_rs == NULL)
    {
        return NULL;
    }
    try
    {
        NuoDB::ResultSetMetaData * md = _rs->getMetaData();
        rval = md->getColumnName(column+1);
    }
    catch (NuoDB::SQLException & e)
    {
        printf("Failed: %s", e.getText());
    }
    return rval;
}

int PdoNuoDbStatement::getSqlType(size_t column)
{
    if (_rs == NULL)
    {
        return 0;
    }
    NuoDB::ResultSetMetaData * md = _rs->getMetaData();
    int sqlType = md->getColumnType(column+1);
    switch (sqlType)
    {
    case NuoDB::NUOSQL_BOOLEAN:
        return PDO_NUODB_SQLTYPE_BOOLEAN;
    case NuoDB::NUOSQL_INTEGER:
        return PDO_NUODB_SQLTYPE_INTEGER;
    case NuoDB::NUOSQL_BIGINT:
        return PDO_NUODB_SQLTYPE_BIGINT;
    case NuoDB::NUOSQL_DOUBLE:
        return PDO_NUODB_SQLTYPE_DOUBLE;
    case NuoDB::NUOSQL_VARCHAR:
        return PDO_NUODB_SQLTYPE_STRING;
    case NuoDB::NUOSQL_DATE:
        return PDO_NUODB_SQLTYPE_DATE;
    case NuoDB::NUOSQL_TIME:
        return PDO_NUODB_SQLTYPE_TIME;
    case NuoDB::NUOSQL_TIMESTAMP:
        return PDO_NUODB_SQLTYPE_TIMESTAMP;
    }
    return 0;
}

char const * PdoNuoDbStatement::getString(size_t column)
{
    if (_rs == NULL)
    {
        return NULL;
    }
    return _rs->getString(column+1);
}

unsigned int PdoNuoDbStatement::getInteger(size_t column)
{
    if (_rs == NULL)
    {
        return 0;
    }
    return _rs->getInt(column+1);
}

unsigned long PdoNuoDbStatement::getLong(size_t column)
{
    if (_rs == NULL)
    {
        return 0;
    }
    return _rs->getLong(column+1);
}

unsigned long PdoNuoDbStatement::getTimestamp(size_t column)
{
    if (_rs == NULL)
    {
        return 0;
    }
    NuoDB::Timestamp *ts = _rs->getTimestamp(column+1);
    return ts->getSeconds();
}

unsigned long PdoNuoDbStatement::getTime(size_t column)
{
    if (_rs == NULL)
    {
        return 0;
    }
    NuoDB::Time *time = _rs->getTime(column+1);
    return time->getSeconds();
}

unsigned long PdoNuoDbStatement::getDate(size_t column)
{
    if (_rs == NULL)
    {
        return 0;
    }
    NuoDB::Date *date = _rs->getDate(column+1);
    return date->getSeconds();
}

size_t PdoNuoDbStatement::getNumberOfParameters()
{
    if (_stmt == NULL) {
        return 0;
    }
    NuoDB::ParameterMetaData *pmd = _stmt->getParameterMetaData();
    if (pmd == NULL) {
        return 0;
    }
    return pmd->getParameterCount();
}

void PdoNuoDbStatement::setInteger(size_t index, int value)
{
    if (_stmt == NULL) {
        return;
    }
    _stmt->setInt(index+1, value);
    return;
}

void PdoNuoDbStatement::setString(size_t index, const char *value)
{
    if (_stmt == NULL) {
        return;
    }
    _stmt->setString(index+1, value);
    return;
}


// C/C++ jump functions

extern "C" {
int pdo_nuodb_db_handle_commit(pdo_nuodb_db_handle *H) {
	try {
		PdoNuoDbHandle *db = (PdoNuoDbHandle *) (H->db);
		db->commit();
	} catch (...) {
        return 0;
    }
    return 1;
}

int pdo_nuodb_db_handle_rollback(pdo_nuodb_db_handle *H) {
	try {
		PdoNuoDbHandle *db = (PdoNuoDbHandle *) (H->db);
		db->rollback();
	} catch (...) {
        return 0;
    }
    return 1;
}

int pdo_nuodb_db_handle_close_connection(pdo_nuodb_db_handle *H) {
	try {
		PdoNuoDbHandle *db = (PdoNuoDbHandle *) (H->db);
		db->closeConnection();
	} catch (...) {
        return 0;
    }
    return 1;
}

int pdo_nuodb_db_handle_delete(pdo_nuodb_db_handle *H) {
	try {
		delete H->db;
		H->db = NULL;
	} catch (...) {
		H->db = NULL;
        return 0;
    }
    return 1;
}

int pdo_nuodb_db_handle_commit_if_auto(pdo_nuodb_db_handle * H, int in_txn, int auto_commit)
{
    if (H == NULL) {
        return 0;
    }
    if (in_txn)
    {
        if (auto_commit)
        {
            pdo_nuodb_db_handle_commit(H); //H->db->commit();
            return 1;
        }
        else
        {
            pdo_nuodb_db_handle_rollback(H);  //H->db->rollback();
        }
    }
    return 0;
}

void *pdo_nuodb_db_handle_create_statement(pdo_nuodb_db_handle * H, const char * sql) 
{
	PdoNuoDbStatement *stmt = NULL;
    try
    {
		PdoNuoDbHandle *db = (PdoNuoDbHandle *) (H->db);
		stmt = db->createStatement(sql);
    }
    catch (...)
    {
		stmt = NULL;
    }
	return (void *)stmt;
}

long pdo_nuodb_db_handle_doer(pdo_nuodb_db_handle * H, void *dbh_opaque, const char * sql, unsigned in_txn, unsigned auto_commit, void (*pt2pdo_dbh_t_set_in_txn)(void *dbh_opaque, unsigned in_txn))  
{
	unsigned in_txn_state = in_txn;
    try
    {
        PdoNuoDbStatement * stmt = (PdoNuoDbStatement *) pdo_nuodb_db_handle_create_statement(H, sql);
        (*pt2pdo_dbh_t_set_in_txn)(dbh_opaque, 1);
        stmt->execute();
        pdo_nuodb_db_handle_commit_if_auto(H, in_txn, auto_commit);
    }
    catch (...)
    {
        (*pt2pdo_dbh_t_set_in_txn)(dbh_opaque, in_txn_state);
        return -1;
    }
    (*pt2pdo_dbh_t_set_in_txn)(dbh_opaque, in_txn_state);
    return 1;
}

int pdo_nuodb_db_handle_factory(pdo_nuodb_db_handle * H, SqlOptionArray *optionsArray) {
	try {
		PdoNuoDbHandle *db = new PdoNuoDbHandle(optionsArray);
        H->db = (void *) db;
        db->createConnection();
	} catch (...) {
        return 0;
    }
    return 1;
}

void pdo_nuodb_db_handle_set_last_app_error(pdo_nuodb_db_handle *H, const char *err_msg) {
	H->last_app_error = err_msg;
}

int pdo_nuodb_stmt_delete(pdo_nuodb_stmt * S) {
	try {
		if (S == NULL) {
			return 1;
		}
		if (S->stmt == NULL) {
			return 1;
		}
		free(S->sql);
		delete S->stmt;
		S->stmt = NULL;
	} catch (...) {
        return 0;
    }
    return 1;
}

int pdo_nuodb_stmt_execute(pdo_nuodb_stmt * S, int *column_count, long *row_count) {
    pdo_nuodb_db_handle * H = (pdo_nuodb_db_handle *) S->H;
    if (!H) {
        return 0;
    }
    unsigned long affected_rows = 0;
    if (!S->stmt) {
        return 0;
    }

    // TODO: check that (!stmt->executed) here?

    try
    {
		PdoNuoDbStatement *pdo_stmt = (PdoNuoDbStatement *) S->stmt;
        pdo_stmt->execute();
        S->cursor_open = pdo_stmt->hasResultSet();
        *column_count = pdo_stmt->getColumnCount();
    }
    catch (NuoDB::SQLException & e)
    {
        return 0;
    }
    *row_count = affected_rows;

    // TODO: commit here?

	pdo_nuodb_db_handle_commit(H);//    H->db->commit();
    S->exhausted = !S->cursor_open;

    return 1;
}

int pdo_nuodb_stmt_fetch(pdo_nuodb_stmt * S, long *row_count) {
    pdo_nuodb_db_handle * H = (pdo_nuodb_db_handle *)S->H;

    if (!S->exhausted)
    {
		PdoNuoDbStatement *pdo_stmt = (PdoNuoDbStatement *) S->stmt;
		if (pdo_stmt->next())
        {
            (*row_count)++;
            return 1;
        }
        else
        {
            S->exhausted = 1;
            return 0;
        }
    }
    return 0;

}

char const *pdo_nuodb_stmt_get_column_name(pdo_nuodb_stmt * S, int colno) {
	PdoNuoDbStatement *pdo_stmt = (PdoNuoDbStatement *) S->stmt;
    char const * column_name = pdo_stmt->getColumnName(colno);
	return column_name;
}

int pdo_nuodb_stmt_get_sql_type(pdo_nuodb_stmt * S, int colno) {
	PdoNuoDbStatement *pdo_stmt = (PdoNuoDbStatement *) S->stmt;
    int sql_type = pdo_stmt->getSqlType(colno);
	return sql_type;
}

int pdo_nuodb_stmt_set_integer(pdo_nuodb_stmt *S, int paramno, long int_val) 
{
	PdoNuoDbStatement *pdo_stmt = (PdoNuoDbStatement *) S->stmt;
	pdo_stmt->setInteger(paramno,  int_val);
	return 1;
}

int pdo_nuodb_stmt_set_string(pdo_nuodb_stmt *S, int paramno, char *str_val)
{
	PdoNuoDbStatement *pdo_stmt = (PdoNuoDbStatement *) S->stmt;
	pdo_stmt->setString(paramno,  str_val);
	return 1;
}

unsigned int pdo_nuodb_stmt_get_integer(pdo_nuodb_stmt *S, int colno) 
{
	PdoNuoDbStatement *pdo_stmt = (PdoNuoDbStatement *) S->stmt;
	return pdo_stmt->getInteger(colno);
}

unsigned long pdo_nuodb_stmt_get_long(pdo_nuodb_stmt *S, int colno) 
{
	PdoNuoDbStatement *pdo_stmt = (PdoNuoDbStatement *) S->stmt;
	return pdo_stmt->getLong(colno);
}

const char *pdo_nuodb_stmt_get_string(pdo_nuodb_stmt *S, int colno) 
{
	PdoNuoDbStatement *pdo_stmt = (PdoNuoDbStatement *) S->stmt;
	return pdo_stmt->getString(colno);
}

unsigned long pdo_nuodb_stmt_get_date(pdo_nuodb_stmt *S, int colno) 
{
	PdoNuoDbStatement *pdo_stmt = (PdoNuoDbStatement *) S->stmt;
	return pdo_stmt->getDate(colno);
}

unsigned long pdo_nuodb_stmt_get_time(pdo_nuodb_stmt *S, int colno) 
{
	PdoNuoDbStatement *pdo_stmt = (PdoNuoDbStatement *) S->stmt;
	return pdo_stmt->getTime(colno);
}

unsigned long pdo_nuodb_stmt_get_timestamp(pdo_nuodb_stmt *S, int colno) 
{
	PdoNuoDbStatement *pdo_stmt = (PdoNuoDbStatement *) S->stmt;
	return pdo_stmt->getTimestamp(colno);
}

} // end of extern "C"