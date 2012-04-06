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

#include "nuodb/sqlapi/SqlColumnMetaData.h"
#include "nuodb/sqlapi/SqlConnection.h"
#include "nuodb/sqlapi/SqlDatabaseMetaData.h"
#include "nuodb/sqlapi/SqlDate.h"
#include "nuodb/sqlapi/SqlEnvironment.h"
#include "nuodb/sqlapi/SqlExceptions.h"
#include "nuodb/sqlapi/SqlPreparedStatement.h"
#include "nuodb/sqlapi/SqlResultSet.h"
#include "nuodb/sqlapi/SqlStatement.h"

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::ComponentModel;
using namespace System::Data;
using namespace System::Data::Common;
using namespace System::Runtime::Serialization;
using namespace System::Security::Permissions;

#define NUODB_WRAPPER_BASE(T) \
	private: \
		std::auto_ptr<nuodb::sqlapi::T> m_ref; \
	public: \
		T##Wrapper(nuodb::sqlapi::T* v) : m_ref(v) { } \
	public: \
		nuodb::sqlapi::T* ref() { return m_ref.get(); }

namespace NuoDb
{
	/// NuoDb data types.
	public enum NuoDbType
	{
		/// Boolean
		Boolean,
		/// 32-bit integer
		Integer,
		/// 64-bit integer
		BigInt,
		/// 64-bit double precision number
		Double,
		/// String (char, varchar, ...)
		String,
		/// Date
		Date,
		/// Time
		Time,
		/// DateTime
		DateTime
	};

	private ref class NuoDbColumnInfo
	{
#pragma region Instance Fields
	private:
		System::String^ m_name;
		NuoDbType m_type;
#pragma endregion

#pragma region Construction / Destruction
	public:
		NuoDbColumnInfo(System::String^ n, NuoDbType t) :
			m_name(n),
			m_type(t)
		{
		}

		NuoDbColumnInfo(System::String^ n, nuodb::sqlapi::SqlType t) :
			m_name(n),
			m_type(String)
		{
			switch (t)
			{
				case nuodb::sqlapi::SQL_BOOLEAN: m_type = Boolean; break;
				case nuodb::sqlapi::SQL_INTEGER: m_type = Integer; break;
				case nuodb::sqlapi::SQL_BIGINT: m_type = BigInt; break;
				case nuodb::sqlapi::SQL_DOUBLE: m_type = Double; break;
				case nuodb::sqlapi::SQL_DATE: m_type = Date; break;
				case nuodb::sqlapi::SQL_TIME: m_type = Time; break;
				case nuodb::sqlapi::SQL_DATETIME: m_type = DateTime; break;

				default: break;
			}
		}
#pragma endregion

#pragma region Properties
	public:
		property System::String^ Name
		{
			System::String^ get() { return m_name; }
		}

		property NuoDbType Type
		{
			NuoDbType get() { return m_type; }
		}
#pragma endregion
	};

	class SqlColumnMetaDataWrapper
	{
		NUODB_WRAPPER_BASE(SqlColumnMetaData)
	};
	
	class SqlConnectionWrapper
	{
		NUODB_WRAPPER_BASE(SqlConnection)
	};
	
	class SqlDateWrapper
	{
		NUODB_WRAPPER_BASE(SqlDate)
	};

	class SqlEnvironmentWrapper
	{
		NUODB_WRAPPER_BASE(SqlEnvironment)
	};

	class SqlPreparedStatementWrapper
	{
		NUODB_WRAPPER_BASE(SqlPreparedStatement)
	};

	class SqlResultSetWrapper
	{
		NUODB_WRAPPER_BASE(SqlResultSet)
	};

	class SqlStatementWrapper
	{
		NUODB_WRAPPER_BASE(SqlStatement)
	};
}
