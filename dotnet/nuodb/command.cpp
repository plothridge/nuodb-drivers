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

#include "stdafx.h"

#include "connection.h"
#include "transaction.h"
#include "exception.h"
#include "parameter.h"
#include "command.h"
#include "datareader.h"

#pragma region Construction / Destruction
NuoDb::NuoDbCommand::NuoDbCommand() :
	m_commandText(System::String::Empty),
	m_commandTimeout(NUODB_DEFAULT_COMMAND_TIMEOUT),
	m_commandType(System::Data::CommandType::Text),
	m_connection(nullptr),
	m_parameters(gcnew NuoDbParameterCollection()),
	m_transaction(nullptr),
	m_designTimeVisible(false),
	m_updateRowSource(System::Data::UpdateRowSource::None),
	m_currentResults(NULL),
	m_disposed(false)
{
}

NuoDb::NuoDbCommand::~NuoDbCommand()
{
	if (m_disposed)
		return;

	this->!NuoDbCommand();

	m_disposed = true;
}

NuoDb::NuoDbCommand::!NuoDbCommand()
{
	if (m_currentResults != NULL)
		delete m_currentResults;

	m_currentResults = NULL;
}
#pragma endregion 

#pragma region Properties
void NuoDb::NuoDbCommand::Connection::set(NuoDb::NuoDbConnection^ value)
{
	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	m_connection = value;
}

void NuoDb::NuoDbCommand::Transaction::set(NuoDbTransaction^ value)
{
	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	m_transaction = value;
	m_connection = value->Connection;
}
#pragma endregion 

#pragma region Methods
NuoDb::NuoDbDataReader^ NuoDb::NuoDbCommand::ExecuteDataReader(CommandBehavior behavior)
{
	if (nullptr == m_connection)
		throw gcnew NuoDbException("No connection has been associated with the command.");

	if (!m_connection->IsOpen)
		throw gcnew NuoDbException("The connection is not open.");

	if (System::String::IsNullOrEmpty(m_commandText))
		throw gcnew NuoDbException("There is no CommandText.");

	Execute();

	NuoDbDataReader^ reader = gcnew NuoDbDataReader(m_currentResults, this, behavior);

	m_currentResults = NULL; // we don't own it anymore!

	return reader;
}

void NuoDb::NuoDbCommand::Execute()
{
	if (nullptr == m_connection)
		throw gcnew NuoDbException("No connection has been associated with the command.");

	if (!m_connection->IsOpen)
		throw gcnew NuoDbException("The connection is not open.");

	if (System::String::IsNullOrEmpty(m_commandText))
		throw gcnew NuoDbException("There is no CommandText.");

	if (m_currentResults != NULL)
		delete m_currentResults;

	m_currentResults = NULL;

	msclr::interop::marshal_context^ mc = gcnew msclr::interop::marshal_context();

	try
	{
		SqlConnectionWrapper* c = m_connection->InternalConnection;

#if DOTNET_35
		System::String^ local(m_commandText);

		const char* s = mc->marshal_as<const char*>(local);
#else
		const char* s = mc->marshal_as<const char*>(m_commandText);
#endif //DOTNET_35

		if (0 == m_parameters->Count)
		{
			SqlStatementWrapper w(c->ref()->createStatement());

			m_currentResults = new SqlResultSetWrapper(w.ref()->executeQuery(s));
		}

		SqlPreparedStatementWrapper pw(c->ref()->createPreparedStatement(s));

		for (int i = 0; i < m_parameters->Count; i++)
		{
			switch (m_parameters[i]->NuoDbType)
			{
				case Boolean: pw.ref()->setBoolean(i, Convert::ToBoolean(m_parameters[i]->Value)); break;
				case Integer: pw.ref()->setInteger(i, Convert::ToInt32(m_parameters[i]->Value)); break;
				case BigInt: pw.ref()->setLong(i, Convert::ToInt64(m_parameters[i]->Value)); break;
				case Double: pw.ref()->setDouble(i, Convert::ToDouble(m_parameters[i]->Value)); break;

				case Date:
				case Time:
				case DateTime: 
					{
						System::DateTime d = Convert::ToDateTime(m_parameters[i]->Value);
						System::TimeSpan t = d.Subtract(System::DateTime(1970, 1, 1));

						nuodb::sqlapi::SqlDate sd((long long)t.TotalMilliseconds);

						pw.ref()->setDate(i, &sd); 

						break;
					}

				default: pw.ref()->setString(i, mc->marshal_as<const char*>(m_parameters[i]->Value->ToString())); break;
			}
		}

		m_currentResults = new SqlResultSetWrapper(pw.ref()->executeQuery());
	}
	catch (nuodb::sqlapi::ErrorCodeException& e)
	{
		throw gcnew NuoDbException(e);
	}
	finally
	{
		delete mc;
	}
}
#pragma endregion

#pragma region DbCommand Overrides
#pragma region Properties
void NuoDb::NuoDbCommand::CommandText::set(System::String^ value)
{
	if (System::String::IsNullOrEmpty(value))
		throw gcnew ArgumentNullException("value");

	m_commandText = value;
}

void NuoDb::NuoDbCommand::CommandType::set(System::Data::CommandType value)
{
	if (value != System::Data::CommandType::Text)
		throw gcnew NotSupportedException("Only CommandType.Text is supported");

	m_commandType = value;
}

System::Data::Common::DbConnection^ NuoDb::NuoDbCommand::DbConnection::get()
{
	return m_connection;
}

void NuoDb::NuoDbCommand::DbConnection::set(System::Data::Common::DbConnection^ value)
{
	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	NuoDbConnection^ p = dynamic_cast<NuoDbConnection^>(value);

	if (nullptr == p)
		throw gcnew ArgumentException("value is not a NuoDbConnection.");

	m_connection = p;
}

System::Data::Common::DbParameterCollection^ NuoDb::NuoDbCommand::DbParameterCollection::get()
{
	return m_parameters;
}

System::Data::Common::DbTransaction^ NuoDb::NuoDbCommand::DbTransaction::get()
{
	return m_transaction;
}

void NuoDb::NuoDbCommand::DbTransaction::set(System::Data::Common::DbTransaction^ value)
{
	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	NuoDbTransaction^ p = dynamic_cast<NuoDbTransaction^>(value);

	if (nullptr == p)
		throw gcnew ArgumentException("value is not a NuoDbTransaction.");

	m_transaction = p;
	m_connection = p->Connection;
}
#pragma endregion

#pragma region Methods
void NuoDb::NuoDbCommand::Cancel()
{
	throw gcnew NotSupportedException();
}

NuoDb::NuoDbParameter^ NuoDb::NuoDbCommand::CreateParameter()
{
	return gcnew NuoDbParameter();
}

DbParameter^ NuoDb::NuoDbCommand::CreateDbParameter()
{
	return gcnew NuoDbParameter();
}

DbDataReader^ NuoDb::NuoDbCommand::ExecuteDbDataReader(CommandBehavior behavior)
{
	return ExecuteReader(behavior);
}

int NuoDb::NuoDbCommand::ExecuteNonQuery()
{
	if (nullptr == m_connection)
		throw gcnew NuoDbException("No connection has been associated with the command.");

	if (!m_connection->IsOpen)
		throw gcnew NuoDbException("The connection is not open.");

	if (System::String::IsNullOrEmpty(m_commandText))
		throw gcnew NuoDbException("There is no CommandText.");

	Execute();

	if (m_currentResults)
		delete m_currentResults;

	m_currentResults = NULL;

	return 0; // TODO: rows affected count N/A for NuoDB?
}

Object^ NuoDb::NuoDbCommand::ExecuteScalar()
{
	if (nullptr == m_connection)
		throw gcnew NuoDbException("No connection has been associated with the command.");

	if (!m_connection->IsOpen)
		throw gcnew NuoDbException("The connection is not open.");

	if (System::String::IsNullOrEmpty(m_commandText))
		throw gcnew NuoDbException("There is no CommandText.");

	Execute();

	if (0 == m_currentResults->ref()->getColumnCount())
		return nullptr;

	SqlColumnMetaDataWrapper md(m_currentResults->ref()->getMetaData(0));

	switch (md.ref()->getType())
	{
		case nuodb::sqlapi::SQL_BOOLEAN: return m_currentResults->ref()->getBoolean(0);
		case nuodb::sqlapi::SQL_INTEGER: return m_currentResults->ref()->getInteger(0);
		case nuodb::sqlapi::SQL_BIGINT: return m_currentResults->ref()->getLong(0);
		case nuodb::sqlapi::SQL_DOUBLE: return m_currentResults->ref()->getDouble(0);
		case nuodb::sqlapi::SQL_STRING: return gcnew System::String(m_currentResults->ref()->getString(0));

		case nuodb::sqlapi::SQL_DATE: 
		case nuodb::sqlapi::SQL_TIME: 
		case nuodb::sqlapi::SQL_DATETIME:
			{
				std::auto_ptr<nuodb::sqlapi::SqlDate> sd((nuodb::sqlapi::SqlDate*)m_currentResults->ref()->getDate(0));

				long long epochTime = sd->getTime();

				return System::DateTime(1970, 1, 1).Add(System::TimeSpan::FromMilliseconds(Convert::ToDouble(epochTime)));
			}

		default: return nullptr; // TODO: Throw an exception?
	}
}

void NuoDb::NuoDbCommand::Prepare()
{
	if (nullptr == m_connection)
		throw gcnew NuoDbException("No connection has been associated with the command.");

	if (!m_connection->IsOpen)
		throw gcnew NuoDbException("The connection is not open.");

	if (System::String::IsNullOrEmpty(m_commandText))
		throw gcnew NuoDbException("There is no CommandText.");
}
#pragma endregion
#pragma endregion
