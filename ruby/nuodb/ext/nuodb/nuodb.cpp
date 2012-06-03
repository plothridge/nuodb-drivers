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

/*
 * NuoDB Adapter
 */

// TODO it would be better to do this in extconf.rb
#if !defined(WIN32) && !defined(_WIN32)
#define HAVE_STDINT_H 1
#endif

#include <ruby.h>

//------------------------------------------------------------------------------
// class building macros

#define WRAPPER_COMMON(WT, RT)                  \
    private:                                    \
    static VALUE type;                          \
    RT* ptr;									\
public:                                         \
WT(RT* arg) : ptr(arg) {}                       \
static VALUE getRubyType() { return type; }     \
static void release(WT* self)					\
{												\
	/*delete self->ptr;*/						\
    delete self;                                \
}                                               \
static RT* asPtr(VALUE value)					\
{												\
    Check_Type(value, T_DATA);                  \
    return ((WT*) DATA_PTR(value))->ptr;        \
}

#define WRAPPER_DEFINITION(WT)                  \
    VALUE WT::type = 0;

//------------------------------------------------------------------------------
// utility function macros

#define RETURN_WRAPPER(WT, func)                                        \
    WT* w = new WT(func);                                               \
    VALUE obj = Data_Wrap_Struct(WT::getRubyType(), 0, WT::release, w); \
    rb_obj_call_init(obj, 0, 0);                                        \
    return obj

#define SYMBOL_OF(value)                        \
    ID2SYM(rb_intern(#value))

#define INIT_TYPE(name)                                     \
    type = rb_define_class_under(module, name, rb_cObject)

#define DEFINE_SINGLE(func, count)                                      \
    rb_define_singleton_method(type, #func, RUBY_METHOD_FUNC(func), count)

#define DEFINE_METHOD(func, count)                                  \
    rb_define_method(type, #func, RUBY_METHOD_FUNC(func), count)

#include "Connection.h"
#include "BigDecimal.h"
#include "Blob.h"
#include "Bytes.h"
#include "CallableStatement.h"
#include "Clob.h"
#include "DatabaseMetaData.h"
#include "DateClass.h"
#include "ParameterMetaData.h"
#include "PreparedStatement.h"
#include "Properties.h"
#include "ResultList.h"
#include "ResultSet.h"
#include "ResultSetMetaData.h"
#include "SQLException.h"
#include "Savepoint.h"
#include "Statement.h"
#include "TimeClass.h"
#include "Timestamp.h"

using namespace NuoDB;

class SqlDatabaseMetaData
{
    WRAPPER_COMMON(SqlDatabaseMetaData, DatabaseMetaData)

    static void init(VALUE module);
    static VALUE getDatabaseVersion(VALUE self);
    static VALUE getIndexInfo(VALUE self, VALUE schemaValue, VALUE tableValue, VALUE uniqueValue, VALUE approxValue);
	static VALUE getColumns(VALUE self, VALUE schemaValue, VALUE tablePattern);
	static VALUE getTables(VALUE self, VALUE schemaValue, VALUE table);
};

WRAPPER_DEFINITION(SqlDatabaseMetaData);

//------------------------------------------------------------------------------

class SqlResultSetMetaData
{
    WRAPPER_COMMON(SqlResultSetMetaData, ResultSetMetaData);

    static void init(VALUE module);
	static VALUE getColumnCount(VALUE self);
    static VALUE getColumnName(VALUE self, VALUE columnValue);
    static VALUE getScale(VALUE self, VALUE columnValue);
	static VALUE getPrecision(VALUE self, VALUE columnValue);
	static VALUE isNullable(VALUE self, VALUE columnValue);
    static VALUE getColumnTypeName(VALUE self, VALUE columnValue);
    static VALUE getType(VALUE self, VALUE columnValue);
};

WRAPPER_DEFINITION(SqlResultSetMetaData);

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
class SqlResultSet
{
	WRAPPER_COMMON(SqlResultSet, ResultSet)

	static void init(VALUE module);
	static VALUE next(VALUE self);
	static VALUE getColumnCount(VALUE self);
	static VALUE getMetaData(VALUE self);
	static VALUE getInteger(VALUE self, VALUE columnValue);
	static VALUE getDouble(VALUE self, VALUE columnValue);
	static VALUE getString(VALUE self, VALUE columnValue);
	static VALUE getDate(VALUE self, VALUE columnValue);
	static VALUE getChar(VALUE self, VALUE columnValue);
};

WRAPPER_DEFINITION(SqlResultSet)

//------------------------------------------------------------------------------

class SqlStatement
{
    WRAPPER_COMMON(SqlStatement, Statement);

    static void init(VALUE module);
    static VALUE execute(VALUE self, VALUE sqlValue);
    static VALUE executeQuery(VALUE self, VALUE sqlValue);
};

WRAPPER_DEFINITION(SqlStatement);

//------------------------------------------------------------------------------

class SqlPreparedStatement
{
    WRAPPER_COMMON(SqlPreparedStatement, PreparedStatement);

    static void init(VALUE module);
    static VALUE setInteger(VALUE self, VALUE indexValue, VALUE valueValue);
    static VALUE setDouble(VALUE self, VALUE indexValue, VALUE valueValue);
    static VALUE setString(VALUE self, VALUE indexValue, VALUE valueValue);
    static VALUE execute(VALUE self);
    static VALUE executeQuery(VALUE self);
};

WRAPPER_DEFINITION(SqlPreparedStatement);

//------------------------------------------------------------------------------

class SqlConnection
{
    WRAPPER_COMMON(SqlConnection, Connection);

    static void init(VALUE module);
    static VALUE ping(VALUE self);
    static VALUE createStatement(VALUE self);
    static VALUE createPreparedStatement(VALUE self, VALUE sqlValue);
    static VALUE setAutoCommit(VALUE self, VALUE autoCommitValue);
    static VALUE hasAutoCommit(VALUE self);
    static VALUE commit(VALUE self);
    static VALUE rollback(VALUE self);
    static VALUE getMetaData(VALUE self);
	static VALUE getSchema(VALUE self);
	static VALUE createSqlConnection(VALUE self,
									 VALUE databaseValue, 
									 VALUE schemaValue,
									 VALUE usernameValue, 
									 VALUE passwordValue);
};

WRAPPER_DEFINITION(SqlConnection);

void SqlDatabaseMetaData::init(VALUE module)
{
	INIT_TYPE("DatabaseMetaData");
	DEFINE_METHOD(getDatabaseVersion, 0);
	DEFINE_METHOD(getIndexInfo, 4);
	DEFINE_METHOD(getColumns, 2);
	DEFINE_METHOD(getTables, 2);
}

VALUE SqlDatabaseMetaData::getDatabaseVersion(VALUE self)
{
	try 
		{
		return rb_str_new2(asPtr(self)->getDatabaseProductVersion());
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "getDatabaseVersion failed: %s", e.getText());
		}
}

VALUE SqlDatabaseMetaData::getIndexInfo(VALUE self, VALUE schemaValue, VALUE tableValue, VALUE uniqueValue, VALUE approxValue)
{
	const char* schema = StringValuePtr(schemaValue);
	const char* table = StringValuePtr(tableValue);
	bool unique = !(RB_TYPE_P(uniqueValue, T_FALSE) || 
					RB_TYPE_P(uniqueValue, T_NIL));
	bool approx = !(RB_TYPE_P(approxValue, T_FALSE) || 
					RB_TYPE_P(approxValue, T_NIL));
	try 
		{
		RETURN_WRAPPER(SqlResultSet, asPtr(self)->getIndexInfo(NULL, schema, table, unique, approx));
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "getIndexInfo failed: %s", e.getText());
		}
}

VALUE SqlDatabaseMetaData::getColumns(VALUE self, VALUE schemaValue, VALUE tableValue)
{
	const char* schema = StringValuePtr(schemaValue);
	const char* table = StringValuePtr(tableValue);
	try 
		{
		RETURN_WRAPPER(SqlResultSet, asPtr(self)->getColumns(NULL, schema, table, NULL));
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "getColumns failed: %s", e.getText());
		}
}

VALUE SqlDatabaseMetaData::getTables(VALUE self, VALUE schemaValue, VALUE tableValue)
{
	const char* schema = StringValuePtr(schemaValue);
	const char* table = StringValuePtr(tableValue);

	try 
		{
		RETURN_WRAPPER(SqlResultSet, asPtr(self)->getTables(NULL, schema, table, 0, NULL));
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "getTables failed: %s", e.getText());
		}
}

void SqlResultSet::init(VALUE module)
{
	INIT_TYPE("ResultSet");
	DEFINE_METHOD(next, 0);
	DEFINE_METHOD(getMetaData, 0);
	DEFINE_METHOD(getInteger, 1);
	DEFINE_METHOD(getDouble, 1);
	DEFINE_METHOD(getString, 1);
	DEFINE_METHOD(getDate, 1);
	DEFINE_METHOD(getChar, 1);
}

VALUE SqlResultSet::next(VALUE self)
{
	try 
		{
		return asPtr(self)->next() ? Qtrue: Qfalse;
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "next failed: %s", e.getText());
		}
}

VALUE SqlResultSet::getMetaData(VALUE self)
{
	try 
		{
		RETURN_WRAPPER(SqlResultSetMetaData, asPtr(self)->getMetaData());
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "getMetaData failed: %s", e.getText());
		}
}

VALUE SqlResultSet::getInteger(VALUE self, VALUE columnValue)
{
	int column = NUM2UINT(columnValue);
	try 
		{
		return INT2NUM(asPtr(self)->getInt(column));
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "getInteger(%d) failed: %s", column, e.getText());
		}
}

VALUE SqlResultSet::getDouble(VALUE self, VALUE columnValue)
{
	int column = NUM2UINT(columnValue);
	try {
	return rb_float_new(asPtr(self)->getDouble(column));
	} catch (SQLException & e) {
	rb_raise(rb_eRuntimeError, "getDouble(%d) failed: %s", column, e.getText());
	}
}

VALUE SqlResultSet::getString(VALUE self, VALUE columnValue)
{
	int column = NUM2UINT(columnValue);
	try {
	return rb_str_new2(asPtr(self)->getString(column));
	} catch (SQLException & e) {
	rb_raise(rb_eRuntimeError, "getString(%d) failed: %s", column, e.getText());
	}
}

VALUE SqlResultSet::getDate(VALUE self, VALUE columnValue)
{
	int column = NUM2UINT(columnValue);
	try 
		{
		Date* date = asPtr(self)->getDate(column);
		return  ULL2NUM(date->getMilliseconds());
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "getDate(%d) dsfsadfas failed: %s", column, e.getText());
		}
}

VALUE SqlResultSet::getChar(VALUE self, VALUE columnValue)
{
	int column = NUM2UINT(columnValue);
	try 
		{
		return rb_str_new2(asPtr(self)->getString(column));
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "getChar(%d) failed: %s", column, e.getText());
		}
}

//------------------------------------------------------------------------------

void SqlResultSetMetaData::init(VALUE module)
{
	INIT_TYPE("ResultMetaData");
	DEFINE_METHOD(getColumnCount, 0);
	DEFINE_METHOD(getColumnName, 1);
	DEFINE_METHOD(getType, 1);
	DEFINE_METHOD(getColumnTypeName, 1);
	DEFINE_METHOD(getScale, 1);
	DEFINE_METHOD(getPrecision, 1);
	DEFINE_METHOD(isNullable, 1);
}

VALUE SqlResultSetMetaData::getColumnCount(VALUE self)
{
	try 
		{
		return UINT2NUM(asPtr(self)->getColumnCount());
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "getColumnCount failed: %s", e.getText());
		}
}

VALUE SqlResultSetMetaData::getColumnName(VALUE self, VALUE columnValue)
{
	int column = NUM2UINT(columnValue);
	try 
		{
		return rb_str_new2(asPtr(self)->getColumnName(column));
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "getColumnName failed: %s", e.getText());
		}
}

VALUE SqlResultSetMetaData::getScale(VALUE self, VALUE columnValue)
{
	int column = NUM2UINT(columnValue);
	try
		{
		return UINT2NUM(asPtr(self)->getScale(column));
		}
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "getScale failed: %s", e.getText());
		}
}

VALUE SqlResultSetMetaData::getPrecision(VALUE self, VALUE columnValue)
{
	int column = NUM2UINT(columnValue);
	try
		{
		return UINT2NUM(asPtr(self)->getPrecision(column));
		}
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "getPrecision failed: %s", e.getText());
		}
}

VALUE SqlResultSetMetaData::isNullable(VALUE self, VALUE columnValue)
{
	int column = NUM2UINT(columnValue);
	try
		{
		return asPtr(self)->isNullable(column) ? Qtrue : Qfalse;
		}
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "isNullable failed: %s", e.getText());
		}
}

VALUE SqlResultSetMetaData::getColumnTypeName(VALUE self, VALUE columnValue)
{
	int column = NUM2UINT(columnValue);
	try 
		{
		return rb_str_new2(asPtr(self)->getColumnTypeName(column));
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "getColumnTypeName(%d) failed: %s", column, e.getText());
		}
}

VALUE SqlResultSetMetaData::getType(VALUE self, VALUE columnValue)
{
	int column = NUM2UINT(columnValue);
	SqlType t = (SqlType)asPtr(self)->getColumnType(column);

	switch(t)
		{
		case NUOSQL_NULL:
			return SYMBOL_OF(SQL_NULL);
		case NUOSQL_BIT:
			return SYMBOL_OF(SQL_BIT);
		case NUOSQL_TINYINT:
			return SYMBOL_OF(SQL_TINYINT);
		case NUOSQL_SMALLINT:
			return SYMBOL_OF(SQL_SMALLINT);
		case NUOSQL_INTEGER:
			return SYMBOL_OF(SQL_INTEGER);
		case NUOSQL_BIGINT:
			return SYMBOL_OF(SQL_BIGINT);
		case NUOSQL_FLOAT:
			return SYMBOL_OF(SQL_FLOAT);
		case NUOSQL_DOUBLE:
			return SYMBOL_OF(SQL_DOUBLE);
		case NUOSQL_CHAR:
			return SYMBOL_OF(SQL_CHAR);
		case NUOSQL_VARCHAR:
			return SYMBOL_OF(SQL_STRING);
		case NUOSQL_LONGVARCHAR:
			return SYMBOL_OF(SQL_LONGVARCHAR);
		case NUOSQL_DATE:
			return SYMBOL_OF(SQL_DATE);
		case NUOSQL_TIME:
			return SYMBOL_OF(SQL_TIME);
		case NUOSQL_TIMESTAMP:
			return SYMBOL_OF(SQL_TIMESTAMP);
		case NUOSQL_BLOB:
			return SYMBOL_OF(SQL_BLOB);
		case NUOSQL_CLOB:
			return SYMBOL_OF(SQL_CLOB);
		case NUOSQL_NUMERIC:
			return SYMBOL_OF(SQL_NUMERIC);
		case NUOSQL_DECIMAL:
			return SYMBOL_OF(SQL_DECIMAL);
		case NUOSQL_BOOLEAN:
			return SYMBOL_OF(SQL_BOOLEAN);
		case NUOSQL_BINARY:
			return SYMBOL_OF(SQL_BINARY);
		case NUOSQL_LONGVARBINARY:
			return SYMBOL_OF(SQL_LONGVARBINARY);
		default:
			rb_raise(rb_eRuntimeError, "invalid sql type: %d", t);
		}
}

//------------------------------------------------------------------------------

void SqlStatement::init(VALUE module)
{
	INIT_TYPE("Statement");
	DEFINE_METHOD(execute, 1);
	DEFINE_METHOD(executeQuery, 1);
}

VALUE SqlStatement::execute(VALUE self, VALUE sqlValue)
{
	const char* sql = StringValuePtr(sqlValue);
	try 
		{
		asPtr(self)->execute(sql);
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "execute(\"%s\") failed: %s", sql, e.getText());
		}
	return Qnil;
}

VALUE SqlStatement::executeQuery(VALUE self, VALUE sqlValue)
{
	const char* sql = StringValuePtr(sqlValue);
	try 
		{
		RETURN_WRAPPER(SqlResultSet, asPtr(self)->executeQuery(sql));
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "executeQuery(\"%s\") failed: %s", sql, e.getText());
		}
}

//------------------------------------------------------------------------------

void SqlPreparedStatement::init(VALUE module)
{
	INIT_TYPE("PreparedStatement");
	DEFINE_METHOD(setInteger, 2);
	DEFINE_METHOD(setDouble, 2);
	DEFINE_METHOD(setString, 2);
	DEFINE_METHOD(execute, 0);
	DEFINE_METHOD(executeQuery, 0);
}

VALUE SqlPreparedStatement::setInteger(VALUE self, VALUE indexValue, VALUE valueValue)
{
	int32_t index = NUM2UINT(indexValue);
	int32_t value = NUM2INT(valueValue);
	try 
		{
		asPtr(self)->setInt(index, value);
		return Qnil;
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "setInteger(%d, %d) failed: %s", index, value, e.getText());
		}
}

VALUE SqlPreparedStatement::setDouble(VALUE self, VALUE indexValue, VALUE valueValue)
{
	int32_t index = NUM2UINT(indexValue);
	double value = NUM2DBL(valueValue);
	try 
		{
		asPtr(self)->setDouble(index, value);
		return Qnil;
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "setDouble(%d, %g) failed: %s", index, value, e.getText());
		}
}

VALUE SqlPreparedStatement::setString(VALUE self, VALUE indexValue, VALUE valueValue)
{
	int32_t index = NUM2UINT(indexValue);
	char const* value = RSTRING_PTR(valueValue);
	try 
		{
		asPtr(self)->setString(index, value);
		return Qnil;
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "setString(%d, \"%s\") failed: %s", index, value, e.getText());
		}
}

VALUE SqlPreparedStatement::execute(VALUE self)
{
	try 
		{
		asPtr(self)->execute();
		return Qnil;
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "execute failed: %s", e.getText());
		}
}

VALUE SqlPreparedStatement::executeQuery(VALUE self)
{
	try 
		{
		RETURN_WRAPPER(SqlResultSet, asPtr(self)->executeQuery());
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "executeQuery failed: %s", e.getText());
		}
}

//------------------------------------------------------------------------------

void SqlConnection::init(VALUE module)
{
	INIT_TYPE("Connection");
	DEFINE_SINGLE(createSqlConnection, 4);
	DEFINE_METHOD(createStatement, 0);
	DEFINE_METHOD(createPreparedStatement, 1);
	DEFINE_METHOD(setAutoCommit, 1);
	DEFINE_METHOD(hasAutoCommit, 0);
	DEFINE_METHOD(commit, 0);
	DEFINE_METHOD(rollback, 0);
	DEFINE_METHOD(getMetaData, 0);
	DEFINE_METHOD(ping, 0);
	DEFINE_METHOD(getSchema, 0);
}

VALUE SqlConnection::getSchema(VALUE self)
{
    try 
		{
		return rb_str_new2(asPtr(self)->getSchema());
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "getSchema() failed: %s", e.getText());
		}
}

VALUE SqlConnection::ping(VALUE self)
{
	try 
		{
		asPtr(self)->ping();
		return Qtrue;
		} 
	catch (SQLException & e) 
		{
		return Qfalse;
		}
}

VALUE SqlConnection::createStatement(VALUE self)
{
	try 
		{
		RETURN_WRAPPER(SqlStatement, asPtr(self)->createStatement());
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "createStatement failed: %s", e.getText());
		}
}

VALUE SqlConnection::createPreparedStatement(VALUE self, VALUE sqlValue)
{
	const char* sql = StringValuePtr(sqlValue);
	try 
		{
		RETURN_WRAPPER(SqlPreparedStatement, asPtr(self)->prepareStatement(sql));
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "createPreparedStatement(\"%s\") failed: %s", sql, e.getText());
		}
}

VALUE SqlConnection::setAutoCommit(VALUE self, VALUE autoCommitValue)
{
	bool autoCommit = !(RB_TYPE_P(autoCommitValue, T_FALSE) || 
						RB_TYPE_P(autoCommitValue, T_NIL));
	try 
		{
		asPtr(self)->setAutoCommit(autoCommit);
		return Qnil;
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "setAutoCommit(%d) failed: %s", autoCommit, e.getText());
		}
}

VALUE SqlConnection::hasAutoCommit(VALUE self)
{
	try 
		{
		return asPtr(self)->getAutoCommit() ? Qtrue : Qfalse;
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "hasAutoCommit failed: %s", e.getText());
		}
}

VALUE SqlConnection::commit(VALUE self)
{
	try 
		{
		asPtr(self)->commit();
		return Qnil;
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "commit failed: %s", e.getText());
		}
}

VALUE SqlConnection::rollback(VALUE self)
{
	try 
		{
		asPtr(self)->rollback();
		return Qnil;
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "rollback failed: %s", e.getText());
		}
}

VALUE SqlConnection::getMetaData(VALUE self)
{
	try 
		{
		RETURN_WRAPPER(SqlDatabaseMetaData, asPtr(self)->getMetaData());
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, "getMetaData failed: %s", e.getText());
		}
}

VALUE SqlConnection::createSqlConnection(VALUE self,
										 VALUE databaseValue, 
										 VALUE schemaValue,
										 VALUE usernameValue, 
										 VALUE passwordValue)
{
	char const* database = StringValuePtr(databaseValue);
	char const* schema = StringValuePtr(schemaValue);
	char const* username = StringValuePtr(usernameValue);
	char const* password = StringValuePtr(passwordValue);
           
	try 
		{
		RETURN_WRAPPER(SqlConnection, getDatabaseConnection(database, 
															username, 
															password, 
															1, 
															"schema",
															schema));
		} 
	catch (SQLException & e) 
		{
		rb_raise(rb_eRuntimeError, 
				 "createSqlConnection(\"%s\", \"%s\", \"%s\", ...) failed: %s",
				 database, 
				 schema, 
				 username, 
				 e.getText());
		}
}

//------------------------------------------------------------------------------

extern "C"
void Init_nuodb(void)
{
    VALUE module = rb_define_module("Nuodb");
    SqlConnection::init(module);
    SqlDatabaseMetaData::init(module);
    SqlStatement::init(module);
    SqlPreparedStatement::init(module);
    SqlResultSet::init(module);
    SqlResultSetMetaData::init(module);
}

//------------------------------------------------------------------------------
