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

#include "NuoDbConnection.h"
#include "NuoDbTransaction.h"
#include "NuoDbCommand.h"

NUODB_NAMESPACE_BEGIN

NuoDbConnection::NuoDbConnection() :
	_disposed(false),
	_connectionString(String::Empty),
	_connection(NULL),
	_metadata(NULL),
	_inTransaction(false)
{
}

NuoDbConnection::NuoDbConnection(String^ connectionString) :
	_disposed(false),
	_connectionString(connectionString),
	_connection(NULL),
	_metadata(NULL),
	_inTransaction(false)
{
	ParseConnectionString();
}

NuoDbConnection::~NuoDbConnection()
{
	if (_disposed)
		return;

	this->!NuoDbConnection();

	_disposed = true;
}

NuoDbConnection::!NuoDbConnection()
{
	Close();
}

Object^ NuoDbConnection::GetColumnValue(int ordinal, NuoDB::ResultSet* results, NuoDB::ResultSetMetaData* metadata)
{
	if (NULL == results)
		throw gcnew ArgumentNullException("results");

	if (NULL == metadata)
		throw gcnew ArgumentNullException("metadata");

	if (ordinal < 0 || ordinal >= metadata->getColumnCount())
		throw gcnew IndexOutOfRangeException("ordinal");

	switch (metadata->getColumnType(ordinal))
	{
		case NuoDB::NUOSQL_NULL: return nullptr;

		case NuoDB::NUOSQL_BIT:
		case NuoDB::NUOSQL_BOOLEAN:
			return results->getBoolean(ordinal);

		case NuoDB::NUOSQL_TINYINT: return results->getByte(ordinal);
		case NuoDB::NUOSQL_SMALLINT: return results->getShort(ordinal);
		case NuoDB::NUOSQL_INTEGER: return results->getInt(ordinal);
		case NuoDB::NUOSQL_BIGINT: return results->getLong(ordinal);
		case NuoDB::NUOSQL_FLOAT: return results->getFloat(ordinal);
		case NuoDB::NUOSQL_DOUBLE: return results->getDouble(ordinal);

		case NuoDB::NUOSQL_DATE:
		case NuoDB::NUOSQL_TIMESTAMP:
			{
				NuoDB::Date* d = results->getDate(ordinal);

				if (d != NULL)
					return DateTime(1970, 1, 1).AddMilliseconds((double)d->getMilliseconds());

				return DateTime::MinValue;
			}

		case NuoDB::NUOSQL_TIME:
			{
				NuoDB::Date* d = results->getDate(ordinal);

				if (d != NULL)
					return DateTime(1970, 1, 1).AddSeconds((double)d->getSeconds());

				return DateTime::MinValue;
			}

		default: break;
	}

	return gcnew System::String(results->getString(ordinal));
}

NuoDbCommand^ NuoDbConnection::CreateCommand()
{
	NuoDbCommand^ c = gcnew NuoDbCommand();

	c->Connection = this;

	return c;
}

bool NuoDbConnection::Ping()
{
	if (NULL == _connection)
		return false;

	try
	{
		_connection->ping();

		return true;
	}
	catch (NuoDB::SQLException&)
	{
		return false;
	}
}

void NuoDbConnection::CommitTransaction()
{
	if (NULL == _connection)
		throw gcnew InvalidOperationException("The connection is not open.");

	if (!_inTransaction)
		throw gcnew InvalidOperationException("There is no current transaction.");

	_inTransaction = false;

	try
	{
		_connection->commit();
		_connection->setAutoCommit(true);
	}
	catch (NuoDB::SQLException& e)
	{
		throw gcnew NuoDbException(e);
	}
}

void NuoDbConnection::RollbackTransaction()
{
	if (NULL == _connection)
		throw gcnew InvalidOperationException("The connection is not open.");

	if (!_inTransaction)
		throw gcnew InvalidOperationException("There is no current transaction.");

	_inTransaction = false;

	try
	{
		_connection->rollback();
		_connection->setAutoCommit(true);
	}
	catch (NuoDB::SQLException& e)
	{
		throw gcnew NuoDbException(e);
	}
}

void NuoDbConnection::OpenConnection()
{
	if (_connection != NULL)
		throw gcnew NuoDbException("The connection is already open.");

	if (String::IsNullOrEmpty(_connectionString))
		throw gcnew ArgumentNullException("ConnectionString");

	if (String::IsNullOrEmpty(_server))
		throw gcnew ArgumentNullException("No server was specified in the ConnectionString");

	if (String::IsNullOrEmpty(_database))
		throw gcnew ArgumentNullException("No database was specified in the ConnectionString");

	if (String::IsNullOrEmpty(_schema))
		throw gcnew ArgumentNullException("No schema was specified in the ConnectionString");

	if (String::IsNullOrEmpty(_username))
		throw gcnew ArgumentNullException("No username was specified in the ConnectionString");

	if (String::IsNullOrEmpty(_password))
		throw gcnew ArgumentNullException("No password was specified in the ConnectionString");

	String^ database(_database);

	database = database + "@" + _server;

#if DOTNET_35
	String^ schema(_schema);
	String^ username(_username);
	String^ password(_password);
#endif //DOTNET_35

	msclr::interop::marshal_context^ mc = gcnew msclr::interop::marshal_context();

	try
	{
		_connection = ::createConnection();

		NuoDB::Properties* props = _connection->allocProperties();

#if DOTNET_35
		props->putValue("schema", mc->marshal_as<const char*>(schema));
		props->putValue("user", mc->marshal_as<const char*>(username));
		props->putValue("password", mc->marshal_as<const char*>(password));
#else
		props->putValue("schema", mc->marshal_as<const char*>(_schema));
		props->putValue("user", mc->marshal_as<const char*>(_username));
		props->putValue("password", mc->marshal_as<const char*>(_password));
#endif //DOTNET_35

		_connection->openDatabase(mc->marshal_as<const char*>(database), props);
		_metadata = _connection->getMetaData();
	}
	catch (NuoDB::SQLException& e)
	{
		throw gcnew NuoDbException(e);
	}
	finally
	{
		delete mc;
	}
}

void NuoDbConnection::ParseConnectionString()
{
	if (String::IsNullOrEmpty(_connectionString))
		throw gcnew ArgumentNullException("ConnectionString");

	String^ chunkDelimiter = ";";
	String^ pairDelimiter = "=";

	for each (String^ chunk in _connectionString->Split(chunkDelimiter->ToCharArray(), StringSplitOptions::RemoveEmptyEntries))
	{
		array<String^>^ parts = chunk->Split(pairDelimiter->ToCharArray());

		if (parts->Length != 2)
			throw gcnew ArgumentException("The connection string is malformed");

		if (parts[0]->Equals("Database", StringComparison::InvariantCultureIgnoreCase))
			_database = parts[1];
		else if (parts[0]->Equals("Server", StringComparison::InvariantCultureIgnoreCase))
			_server = parts[1];
		else if (parts[0]->Equals("Schema", StringComparison::InvariantCultureIgnoreCase))
			_schema = parts[1];
		else if (parts[0]->Equals("Username", StringComparison::InvariantCultureIgnoreCase))
			_username = parts[1];
		else if (parts[0]->Equals("Password", StringComparison::InvariantCultureIgnoreCase))
			_password = parts[1];
		else
			throw gcnew ArgumentException("Unknown connection string property \"" + parts[0] + "\"");
	}
}

void NuoDbConnection::ConnectionString::set(String^ value)
{
	if (String::IsNullOrEmpty(value))
		throw gcnew ArgumentNullException("value");

	_connectionString = value;

	ParseConnectionString();
}

String^ NuoDbConnection::ServerVersion::get()
{
	if (NULL == _connection)
		throw gcnew NuoDbException("The connection is closed.");

	return gcnew String(_metadata->getDatabaseProductVersion());
}

System::Data::ConnectionState NuoDbConnection::State::get()
{
	return NULL == _connection ? System::Data::ConnectionState::Closed : System::Data::ConnectionState::Open;
}

NuoDbTransaction^ NuoDbConnection::BeginTransaction()
{
	return BeginTransaction(System::Data::IsolationLevel::Unspecified);
}

NuoDbTransaction^ NuoDbConnection::BeginTransaction(System::Data::IsolationLevel isolationLevel)
{
	if (NULL == _connection)
		throw gcnew NuoDbException("The connection is closed.");

	if (_inTransaction)
		throw gcnew NuoDbException("A transaction is already underway.");

	int level = NuoDB::transactionConsistentRead;

	switch (isolationLevel)
	{
		case System::Data::IsolationLevel::ReadUncommitted: level = NuoDB::transactionReadUncommitted; break;
		case System::Data::IsolationLevel::ReadCommitted: level = NuoDB::transactionReadCommitted; break;
		case System::Data::IsolationLevel::RepeatableRead: level = NuoDB::transactionRepeatableRead; break;
		case System::Data::IsolationLevel::Serializable: level = NuoDB::transactionSerializable; break;
		case System::Data::IsolationLevel::Unspecified: break;

		default:
			throw gcnew NotSupportedException("The isolation level is not supported");
	}

	try
	{
		_connection->setTransactionIsolation(level);
		_connection->setAutoCommit(false);

		_inTransaction = true;

		return gcnew NuoDbTransaction(this, isolationLevel);
	}
	catch (NuoDB::SQLException& e)
	{
		throw gcnew NuoDbException(e);
	}
}

System::Data::Common::DbTransaction^ NuoDbConnection::BeginDbTransaction(System::Data::IsolationLevel isolationLevel)
{
	return BeginTransaction(isolationLevel);
}

void NuoDbConnection::ChangeDatabase(String^ databaseName)
{
	if (String::IsNullOrEmpty(databaseName))
		throw gcnew ArgumentNullException("databaseName");

	Close();

	_schema = databaseName;

	OpenConnection();
}

void NuoDbConnection::Close()
{
	RollbackTransaction();

	if (_connection != NULL)
		_connection->close();

	_connection = NULL;
	_metadata = NULL;
}

void NuoDbConnection::EnlistTransaction(System::Transactions::Transaction^ transaction)
{
	throw gcnew NotSupportedException();
}

void NuoDbConnection::Open()
{
	OpenConnection();
}

System::Data::Common::DbCommand^ NuoDbConnection::CreateDbCommand()
{
	return CreateCommand();
}

NUODB_NAMESPACE_END
