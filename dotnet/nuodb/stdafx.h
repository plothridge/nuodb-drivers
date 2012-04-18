/*
	Copyright (c) 2012, NuoDB, Inc.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		 * Redistributions of source code must retain the above copyright
			notice, this list of conditions and the following disclaimer.
		 * Redistributions in binary form must reproduce the above copyright
			notice, this list of conditions and the following disclaimer in the
			documentation and/or other materials provided with the distribution.
		 * Neither the name of NuoDB, Inc. nor the names of its contributors may
			be used to endorse or promote products derived from this software
			without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL NUODB, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
	OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
	OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <msclr/marshal.h>

#include "Connection.h"

using namespace System;

#define NUODB_NAMESPACE_BEGIN namespace NuoDb {
#define NUODB_NAMESPACE_END }

#define NUODB_DEFAULT_COMMAND_TIMEOUT 90

NUODB_NAMESPACE_BEGIN

public enum class NuoDbType
{
	Null = NuoDB::NUOSQL_NULL,
	Bit = NuoDB::NUOSQL_BIT,
	TinyInt = NuoDB::NUOSQL_TINYINT, // byte
	SmallInt = NuoDB::NUOSQL_SMALLINT,
	Integer = NuoDB::NUOSQL_INTEGER,
	BigInt = NuoDB::NUOSQL_BIGINT,
	Float = NuoDB::NUOSQL_FLOAT,
	Double = NuoDB::NUOSQL_DOUBLE,
	Char = NuoDB::NUOSQL_CHAR,
	VarChar = NuoDB::NUOSQL_VARCHAR,
	LongVarChar = NuoDB::NUOSQL_LONGVARCHAR,
	Date = NuoDB::NUOSQL_DATE,
	Time = NuoDB::NUOSQL_TIME,
	Timestamp = NuoDB::NUOSQL_TIMESTAMP,
	Blob = NuoDB::NUOSQL_BLOB,
	Clob = NuoDB::NUOSQL_CLOB,
	Numeric = NuoDB::NUOSQL_NUMERIC,
	Decimal = NuoDB::NUOSQL_DECIMAL,
	Boolean = NuoDB::NUOSQL_BOOLEAN,
	Binary = NuoDB::NUOSQL_BINARY,
	LongVarBinary = NuoDB::NUOSQL_LONGVARBINARY
};

public enum class NuoSqlErrorCode
{
	SyntaxError = SYNTAX_ERROR,
	FeatureNotYetImplemented = FEATURE_NOT_YET_IMPLEMENTED,
	BugCheck = BUG_CHECK,
	CompileError = COMPILE_ERROR,
	RuntimeError = RUNTIME_ERROR,
	OcsError = OCS_ERROR,
	NetworkError = NETWORK_ERROR,
	ConversionError = CONVERSION_ERROR,
	TruncationError = TRUNCATION_ERROR,
	ConnectionError = CONNECTION_ERROR,
	DdlError = DDL_ERROR,
	ApplicationError = APPLICATION_ERROR,
	SecurityError = SECURITY_ERROR,
	DatabaseCorruption = DATABASE_CORRUPTION,
	VersionError = VERSION_ERROR,
	LicenseError = LICENSE_ERROR,
	InternalError = INTERNAL_ERROR,
	DebugError = DEBUG_ERROR,
	LostBlog = LOST_BLOB,
	InconsistentBlob = INCONSISTENT_BLOB,
	DeletedBlob = DELETED_BLOB,
	LogError = LOG_ERROR,
	DatabaseDamaged = DATABASE_DAMAGED,
	UpdateConflict = UPDATE_CONFLICT,
	NoSuchTable = NO_SUCH_TABLE,
	IndexOverflow = INDEX_OVERFLOW,
	UniqueDuplicate = UNIQUE_DUPLICATE,
	UncommittedUpdates = UNCOMMITTED_UPDATES,
	Deadlock = DEADLOCK,
	OutOfMemoryError = OUT_OF_MEMORY_ERROR,
	OutOfRecordMemoryError = OUT_OF_RECORD_MEMORY_ERROR,
	LockTimeout = LOCK_TIMEOUT,
	PlatformError = PLATFORM_ERROR,
	NoSchema = NO_SCHEMA,
	ConfigurationError = CONFIGURATION_ERROR,
	ReadOnlyError = READ_ONLY_ERROR,
	NoGeneratedKeys = NO_GENERATED_KEYS,
	ThrownException = THROWN_EXCEPTION,
	InvalidTransactionIsolation = INVALID_TRANSACTION_ISOLATION,
	UnsupportedTransactionIsolation = UNSUPPORTED_TRANSACTION_ISOLATION,
	InvalidUtf8 = INVALID_UTF8,
	ConstraintError = CONSTRAINT_ERROR
};

[Serializable]
public ref class NuoDbException : System::Data::Common::DbException
{
private:
	NuoSqlErrorCode _errorCode;
	System::String^ _trace;
	System::String^ _objectName;
	System::String^ _sqlState;

public:
	NuoDbException(System::String^ message) :
		System::Data::Common::DbException(message),
		_errorCode(NuoSqlErrorCode::ApplicationError)
	{
	}

	NuoDbException(NuoDB::SQLException& e) :
		System::Data::Common::DbException(gcnew System::String(e.getText())),
		_errorCode((NuoSqlErrorCode)e.getSqlcode()),
		_trace(gcnew System::String(e.getTrace())),
		_sqlState(gcnew System::String(e.getSQLState()))
	{
	}

public:
	property NuoSqlErrorCode ErrorCode { NuoSqlErrorCode get() new { return _errorCode; } }
	property System::String^ Trace { System::String^ get() { return _trace; } }
	property System::String^ SqlState { System::String^ get() { return _sqlState; } }
};

private ref class NuoDbColumnInfo
{
private:
	String^ _name;
	NuoDbType _type;

public:
	NuoDbColumnInfo(String^ n, NuoDbType t) :
		_name(n),
		_type(t)
	{
	}

	NuoDbColumnInfo(String^ n, int t) :
		_name(n),
		_type((NuoDbType)t)
	{
	}

public:
	property String^ Name { String^ get() { return _name; } }
	property NuoDbType Type { NuoDbType get() { return _type; } }
};

ref class NuoDbCommand;
ref class NuoDbConnection;
ref class NuoDbDataReader;
ref class NuoDbParameter;
ref class NuoDbParameterCollection;
ref class NuoDbTransaction;

NUODB_NAMESPACE_END
