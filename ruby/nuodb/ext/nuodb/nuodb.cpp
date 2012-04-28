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

#include "nuodb/sqlapi/SqlResultSet.h"
#include "nuodb/sqlapi/SqlEnvironment.h"
#include "nuodb/sqlapi/SqlConnection.h"
#include "nuodb/sqlapi/SqlStatement.h"
#include "nuodb/sqlapi/SqlPreparedStatement.h"
#include "nuodb/sqlapi/SqlDatabaseMetaData.h"
#include "nuodb/sqlapi/SqlColumnMetaData.h"
#include "nuodb/sqlapi/SqlExceptions.h"

using nuodb::sqlapi::ErrorCodeException;
using nuodb::sqlapi::SqlColumnMetaData;
using nuodb::sqlapi::SqlConnection;
using nuodb::sqlapi::SqlDatabaseMetaData;
using nuodb::sqlapi::SqlEnvironment;
using nuodb::sqlapi::SqlOption;
using nuodb::sqlapi::SqlOptionArray;
using nuodb::sqlapi::SqlPreparedStatement;
using nuodb::sqlapi::SqlResultSet;
using nuodb::sqlapi::SqlStatement;
using nuodb::sqlapi::SqlType;
using nuodb::sqlapi::SQL_INTEGER;
using nuodb::sqlapi::SQL_DOUBLE;
using nuodb::sqlapi::SQL_STRING;
using nuodb::sqlapi::SQL_DATE;
using nuodb::sqlapi::SQL_TIME;
using nuodb::sqlapi::SQL_DATETIME;

#include <ruby.h>

//------------------------------------------------------------------------------
// class building macros

#define WRAPPER_COMMON(WT, RT)					\
  private:							\
  static VALUE type;						\
  RT* ptr;							\
public:								\
 WT(RT* arg) : ptr(arg) {}					\
 static VALUE getRubyType() { return type; }			\
 static void release(WT* self) {				\
   delete self->ptr;						\
   delete self;							\
 }								\
 static RT* asPtr(VALUE value) {				\
   Check_Type(value, T_DATA);					\
   return ((WT*) DATA_PTR(value))->ptr;				\
 }

#define WRAPPER_DEFINITION(WT)			\
  VALUE WT::type = 0;

//------------------------------------------------------------------------------
// utility function macros

#define RETURN_WRAPPER(WT, func)					\
  WT* w = new WT(func);							\
  VALUE obj = Data_Wrap_Struct(WT::getRubyType(), 0, WT::release, w);	\
  rb_obj_call_init(obj, 0, 0);						\
  return obj

#define SYMBOL_OF(value)			\
  ID2SYM(rb_intern(#value))

#define INIT_TYPE(name)						\
  type = rb_define_class_under(module, name, rb_cObject)

#define DEFINE_SINGLE(func, count)					\
  rb_define_singleton_method(type, #func, RUBY_METHOD_FUNC(func), count)

#define DEFINE_METHOD(func, count)				\
  rb_define_method(type, #func, RUBY_METHOD_FUNC(func), count)

//------------------------------------------------------------------------------

class SqlDatabaseMetaDataWrapper
{
  WRAPPER_COMMON(SqlDatabaseMetaDataWrapper, SqlDatabaseMetaData)

  static void init(VALUE module)
  {
    INIT_TYPE("SqlDatabaseMetaData");
    DEFINE_METHOD(getDatabaseVersion, 0);
  }

  static VALUE getDatabaseVersion(VALUE self)
  {
    try {
      return rb_str_new2(asPtr(self)->getDatabaseVersion());
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "getDatabaseVersion failed: %s", e.what());
    }
  }
};

WRAPPER_DEFINITION(SqlDatabaseMetaDataWrapper)

//------------------------------------------------------------------------------

class SqlColumnMetaDataWrapper
{
  WRAPPER_COMMON(SqlColumnMetaDataWrapper, SqlColumnMetaData)

  static void init(VALUE module)
  {
    INIT_TYPE("SqlColumnMetaData");
    DEFINE_METHOD(getColumnName, 0);
    DEFINE_METHOD(getType, 0);
  }

  static VALUE getColumnName(VALUE self)
  {
    try {
      return rb_str_new2(asPtr(self)->getColumnName());
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "getColumnName failed: %s", e.what());
    }
  }

  static VALUE getType(VALUE self)
  {
    SqlType t = asPtr(self)->getType();
    switch(t)
      {
      case SQL_INTEGER:
	return SYMBOL_OF(SQL_INTEGER);
      case SQL_DOUBLE:
	return SYMBOL_OF(SQL_DOUBLE);
      case SQL_STRING:
	return SYMBOL_OF(SQL_STRING);
      case SQL_DATE:
	return SYMBOL_OF(SQL_DATE);
      case SQL_TIME:
	return SYMBOL_OF(SQL_TIME);
      case SQL_DATETIME:
	return SYMBOL_OF(SQL_DATETIME);
      default:
	rb_raise(rb_eRuntimeError, "invalid sql type: %d", t);
      }
  }
};

WRAPPER_DEFINITION(SqlColumnMetaDataWrapper)

//------------------------------------------------------------------------------

class SqlResultSetWrapper
{
  WRAPPER_COMMON(SqlResultSetWrapper, SqlResultSet)

  static void init(VALUE module)
  {
    INIT_TYPE("SqlResultSet");
    DEFINE_METHOD(next, 0);
    DEFINE_METHOD(getColumnCount, 0);
    DEFINE_METHOD(getMetaData, 1);
    DEFINE_METHOD(getInteger, 1);
    DEFINE_METHOD(getDouble, 1);
    DEFINE_METHOD(getString, 1);
    DEFINE_METHOD(getDate, 1);
  }

  static VALUE next(VALUE self)
  {
    try {
      return asPtr(self)->next() ? Qtrue: Qfalse;
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "next failed: %s", e.what());
    }
  }

  static VALUE getColumnCount(VALUE self)
  {
    try {
      return UINT2NUM(asPtr(self)->getColumnCount());
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "getColumnCount failed: %s", e.what());
    }
  }

  static VALUE getMetaData(VALUE self, VALUE columnValue)
  {
    size_t column = NUM2UINT(columnValue);
    try {
      RETURN_WRAPPER(SqlColumnMetaDataWrapper, asPtr(self)->getMetaData(column));
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "getMetaData(%ld) failed: %s", column, e.what());
    }
  }

  static VALUE getInteger(VALUE self, VALUE columnValue)
  {
    size_t column = NUM2UINT(columnValue);
    try {
      return INT2NUM(asPtr(self)->getInteger(column));
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "getInteger(%ld) failed: %s", column, e.what());
    }
  }

  static VALUE getDouble(VALUE self, VALUE columnValue)
  {
    size_t column = NUM2UINT(columnValue);
    try {
      return rb_float_new(asPtr(self)->getDouble(column));
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "getDouble(%ld) failed: %s", column, e.what());
    }
  }

  static VALUE getString(VALUE self, VALUE columnValue)
  {
    size_t column = NUM2UINT(columnValue);
    try {
      return rb_str_new2(asPtr(self)->getString(column));
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "getString(%ld) failed: %s", column, e.what());
    }
  }

  static VALUE getDate(VALUE self, VALUE columnValue)
  {
    size_t column = NUM2UINT(columnValue);
    try {
      // TODO SqlDate const * getDate(size_t column) const;
      rb_notimplement();
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "getDate(%ld) failed: %s", column, e.what());
    }
  }
};

WRAPPER_DEFINITION(SqlResultSetWrapper)

//------------------------------------------------------------------------------

class SqlStatementWrapper
{
  WRAPPER_COMMON(SqlStatementWrapper, SqlStatement)

  static void init(VALUE module)
  {
    INIT_TYPE("SqlStatement");
    DEFINE_METHOD(execute, 1);
    DEFINE_METHOD(executeQuery, 1);
  }

  static VALUE execute(VALUE self, VALUE sqlValue)
  {
    const char* sql = StringValuePtr(sqlValue);
    try {
      asPtr(self)->execute(sql);
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "execute(\"%s\") failed: %s", sql, e.what());
    }
    return Qnil;
  }

  static VALUE executeQuery(VALUE self, VALUE sqlValue)
  {
    const char* sql = StringValuePtr(sqlValue);
    try {
      RETURN_WRAPPER(SqlResultSetWrapper, asPtr(self)->executeQuery(sql));
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "executeQuery(\"%s\") failed: %s", sql, e.what());
    }
  }
};

WRAPPER_DEFINITION(SqlStatementWrapper)

//------------------------------------------------------------------------------

class SqlPreparedStatementWrapper
{
  WRAPPER_COMMON(SqlPreparedStatementWrapper, SqlPreparedStatement)

  static void init(VALUE module)
  {
    INIT_TYPE("SqlPreparedStatement");
    DEFINE_METHOD(setInteger, 2);
    DEFINE_METHOD(setDouble, 2);
    DEFINE_METHOD(setString, 2);
    DEFINE_METHOD(execute, 0);
    DEFINE_METHOD(executeQuery, 0);
  }

  static VALUE setInteger(VALUE self, VALUE indexValue, VALUE valueValue)
  {
    size_t index = NUM2UINT(indexValue);
    int32_t value = NUM2INT(valueValue);
    try {
      asPtr(self)->setInteger(index, value);
      return Qnil;
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "setInteger(%ld, %d) failed: %s", index, value, e.what());
    }
  }

  static VALUE setDouble(VALUE self, VALUE indexValue, VALUE valueValue)
  {
    size_t index = NUM2UINT(indexValue);
    double value = NUM2DBL(valueValue);
    try {
      asPtr(self)->setDouble(index, value);
      return Qnil;
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "setDouble(%ld, %g) failed: %s", index, value, e.what());
    }
  }

  static VALUE setString(VALUE self, VALUE indexValue, VALUE valueValue)
  {
    size_t index = NUM2UINT(indexValue);
    char const* value = RSTRING_PTR(valueValue);
    try {
      asPtr(self)->setString(index, value);
      return Qnil;
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "setString(%ld, \"%s\") failed: %s", index, value, e.what());
    }
  }

  static VALUE execute(VALUE self)
  {
    try {
      asPtr(self)->execute();
      return Qnil;
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "execute failed: %s", e.what());
    }
  }

  static VALUE executeQuery(VALUE self)
  {
    try {
      RETURN_WRAPPER(SqlResultSetWrapper, asPtr(self)->executeQuery());
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "executeQuery failed: %s", e.what());
    }
  }
};

WRAPPER_DEFINITION(SqlPreparedStatementWrapper)

//------------------------------------------------------------------------------

class SqlConnectionWrapper
{
  WRAPPER_COMMON(SqlConnectionWrapper, SqlConnection)

  static void init(VALUE module)
  {
    INIT_TYPE("SqlConnection");
    DEFINE_METHOD(createStatement, 0);
    DEFINE_METHOD(createPreparedStatement, 1);
    DEFINE_METHOD(setAutoCommit, 1);
    DEFINE_METHOD(hasAutoCommit, 0);
    DEFINE_METHOD(commit, 0);
    DEFINE_METHOD(rollback, 0);
    DEFINE_METHOD(getMetaData, 0);
  }

  static VALUE createStatement(VALUE self)
  {
    try {
      RETURN_WRAPPER(SqlStatementWrapper, asPtr(self)->createStatement());
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "createStatement failed: %s", e.what());
    }
  }

  static VALUE createPreparedStatement(VALUE self, VALUE sqlValue)
  {
    const char* sql = StringValuePtr(sqlValue);
    try {
      RETURN_WRAPPER(SqlPreparedStatementWrapper, asPtr(self)->createPreparedStatement(sql));
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "createPreparedStatement(\"%s\") failed: %s", sql, e.what());
    }
  }

  static VALUE setAutoCommit(VALUE self, VALUE autoCommitValue)
  {
    bool autoCommit = !(RB_TYPE_P(autoCommitValue, T_FALSE) || 
			RB_TYPE_P(autoCommitValue, T_NIL));
    try {
      asPtr(self)->setAutoCommit(autoCommit);
      return Qnil;
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "setAutoCommit(%d) failed: %s", autoCommit, e.what());
    }
  }

  static VALUE hasAutoCommit(VALUE self)
  {
    try {
      return asPtr(self)->hasAutoCommit() ? Qtrue : Qfalse;
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "hasAutoCommit failed: %s", e.what());
    }
  }

  static VALUE commit(VALUE self)
  {
    try {
      asPtr(self)->commit();
      return Qnil;
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "commit failed: %s", e.what());
    }
  }

  static VALUE rollback(VALUE self)
  {
    try {
      asPtr(self)->rollback();
      return Qnil;
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "rollback failed: %s", e.what());
    }
  }

  static VALUE getMetaData(VALUE self)
  {
    try {
      RETURN_WRAPPER(SqlDatabaseMetaDataWrapper, asPtr(self)->getMetaData());
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "getMetaData failed: %s", e.what());
    }
  }
};

WRAPPER_DEFINITION(SqlConnectionWrapper)

//------------------------------------------------------------------------------

class SqlEnvironmentWrapper
{
  WRAPPER_COMMON(SqlEnvironmentWrapper, SqlEnvironment)

  static void init(VALUE module)
  {
    INIT_TYPE("SqlEnvironment");
    DEFINE_SINGLE(createSqlEnvironment, 0);
    DEFINE_METHOD(createSqlConnection, 4);
  }

  static VALUE createSqlEnvironment(VALUE self)
  {
    try {
      SqlOptionArray optsArray;
      optsArray.count = 0;

      RETURN_WRAPPER(SqlEnvironmentWrapper, SqlEnvironment::createSqlEnvironment(&optsArray));
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "createSqlEnvironment failed: %s", e.what());
    }
  }

  static VALUE createSqlConnection(VALUE self, VALUE databaseValue, VALUE schemaValue,
				   VALUE usernameValue, VALUE passwordValue)
  {
    char const* database = StringValuePtr(databaseValue);
    char const* schema = StringValuePtr(schemaValue);
    char const* username = StringValuePtr(usernameValue);
    char const* password = StringValuePtr(passwordValue);
    try {
      SqlOption options[4];

      options[0].option = "database";
      options[0].extra = (void*) database;

      options[1].option = "schema";
      options[1].extra = (void*) schema;

      options[2].option = "user";
      options[2].extra = (void*) username;

      options[3].option = "password";
      options[3].extra = (void*) password;

      SqlOptionArray optsArray;
      optsArray.count = 4;
      optsArray.array = options;

      RETURN_WRAPPER(SqlConnectionWrapper, asPtr(self)->createSqlConnection(&optsArray));
    } catch (ErrorCodeException & e) {
      rb_raise(rb_eRuntimeError, "createSqlConnection(\"%s\", \"%s\", \"%s\", ...) failed: %s",
	       database, schema, username, e.what());
    }
  }
};

WRAPPER_DEFINITION(SqlEnvironmentWrapper)

//------------------------------------------------------------------------------

extern "C"
void Init_nuodb(void)
{
  VALUE module = rb_define_module("Nuodb");
  SqlEnvironmentWrapper::init(module);
  SqlConnectionWrapper::init(module);
  SqlDatabaseMetaDataWrapper::init(module);
  SqlStatementWrapper::init(module);
  SqlPreparedStatementWrapper::init(module);
  SqlResultSetWrapper::init(module);
  SqlColumnMetaDataWrapper::init(module);
}

//------------------------------------------------------------------------------
