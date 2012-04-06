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

#include "exception.h"
#include "connection.h"
#include "transaction.h"
#include "command.h"

#pragma region Construction / Destruction
NuoDb::NuoDbConnection::NuoDbConnection() :
	m_connection(NULL),
	m_inTransaction(false),
	m_connectionString(System::String::Empty),
	m_database(System::String::Empty),
	m_schema(System::String::Empty),
	m_username(System::String::Empty),
	m_password(System::String::Empty),
	m_disposed(false)
{
}

NuoDb::NuoDbConnection::NuoDbConnection(System::String^ connectionString) :
	m_connection(NULL),
	m_inTransaction(false),
	m_database(System::String::Empty),
	m_schema(System::String::Empty),
	m_username(System::String::Empty),
	m_password(System::String::Empty),
	m_disposed(false)
{
	if (System::String::IsNullOrEmpty(connectionString))
		throw gcnew ArgumentNullException("connectionString");

	m_connectionString = connectionString;

	ParseConnectionString();
}

NuoDb::NuoDbConnection::~NuoDbConnection()
{
	if (m_disposed)
		return;

	RollbackTransaction();

	this->!NuoDbConnection();

	m_disposed = true;
}

NuoDb::NuoDbConnection::!NuoDbConnection()
{
	if (m_connection != NULL)
		delete m_connection;

	m_connection = NULL;
}
#pragma endregion

#pragma region Methods
void NuoDb::NuoDbConnection::EnlistDistributedTransaction(System::EnterpriseServices::ITransaction^ transaction)
{
	throw gcnew NotSupportedException();
}

NuoDb::NuoDbCommand^ NuoDb::NuoDbConnection::CreateCommand()
{
	if (NULL == m_connection)
		throw gcnew NuoDbException("The connection is not open.");
		
	NuoDbCommand^ c = gcnew NuoDbCommand();

	c->Connection = this;

	return c;
}

void NuoDb::NuoDbConnection::CommitTransaction()
{
	if (NULL == m_connection)
		throw gcnew NuoDbException("The connection is not open.");
		
	if (!m_inTransaction)
		return;

	m_connection->ref()->commit();
	m_inTransaction = false;
	m_connection->ref()->setAutoCommit(true);
}

void NuoDb::NuoDbConnection::RollbackTransaction()
{
	if (NULL == m_connection)
		throw gcnew NuoDbException("The connection is not open.");
		
	if (!m_inTransaction)
		return;

	m_connection->ref()->rollback();
	m_inTransaction = false;
	m_connection->ref()->setAutoCommit(true);
}

void NuoDb::NuoDbConnection::ParseConnectionString()
{
	if (System::String::IsNullOrEmpty(m_connectionString))
		throw gcnew ArgumentNullException("ConnectionString");

	m_database = System::String::Empty;
	m_schema = System::String::Empty;
	m_username = System::String::Empty;
	m_password = System::String::Empty;

	for each (System::String^ item in m_connectionString->Split(';'))
	{
		array<System::String^>^ parts = item->Split('=');

		if (parts->Length != 2)
			throw gcnew ArgumentException("The connection string was malformed.");

		parts[0] = parts[0]->Trim();
		parts[1] = parts[1]->Trim();

		if (0 == parts[0]->Length || 0 == parts[1]->Length)
			throw gcnew ArgumentException("The connection string was malformed.");

		if (parts[0]->Equals("Database", System::StringComparison::InvariantCulture))
			m_database = parts[1];
		else if (parts[0]->Equals("Schema", System::StringComparison::InvariantCulture))
			m_schema = parts[1];
		else if (parts[0]->Equals("User", System::StringComparison::InvariantCulture))
			m_username = parts[1];
		else if (parts[0]->Equals("Password", System::StringComparison::InvariantCulture))
			m_password = parts[1];
		else
			throw gcnew ArgumentException("The connection string contained an unknown key \"" + parts[0] + "\".");
	}
}

void NuoDb::NuoDbConnection::OpenConnection()
{
	if (m_connection != NULL)
		throw gcnew NuoDbException("The connection is already open.");

	if (System::String::IsNullOrEmpty(m_database))
		throw gcnew ArgumentNullException("No database was specified in the ConnectionString");

	if (System::String::IsNullOrEmpty(m_schema))
		throw gcnew ArgumentNullException("No schema was specified in the ConnectionString");

	if (System::String::IsNullOrEmpty(m_username))
		throw gcnew ArgumentNullException("No username was specified in the ConnectionString");

	if (System::String::IsNullOrEmpty(m_password))
		throw gcnew ArgumentNullException("No password was specified in the ConnectionString");

	msclr::interop::marshal_context^ mc = gcnew msclr::interop::marshal_context();

	try
	{
		nuodb::sqlapi::SqlOptionArray env_opts;

		env_opts.count = 0;

		SqlEnvironmentWrapper env(nuodb::sqlapi::SqlEnvironment::createSqlEnvironment(&env_opts));

		nuodb::sqlapi::SqlOption opts[4];
		nuodb::sqlapi::SqlOptionArray conn_opts;

#if DOTNET_35
		System::String^ localDatabase(m_database);

		opts[0].option = "database";
		opts[0].extra = (void*)mc->marshal_as<const char*>(localDatabase);

		System::String^ localSchema(m_schema);

		opts[1].option = "schema";
		opts[1].extra = (void*)mc->marshal_as<const char*>(localSchema);

		System::String^ localUsername(m_username);

		opts[2].option = "username";
		opts[2].extra = (void*)mc->marshal_as<const char*>(localUsername);

		System::String^ localPassword(m_password);

		opts[3].option = "password";
		opts[3].extra = (void*)mc->marshal_as<const char*>(localPassword);
#else
		opts[0].option = "database";
		opts[0].extra = (void*)mc->marshal_as<const char*>(m_database);
		opts[1].option = "schema";
		opts[1].extra = (void*)mc->marshal_as<const char*>(m_schema);
		opts[2].option = "username";
		opts[2].extra = (void*)mc->marshal_as<const char*>(m_username);
		opts[3].option = "password";
		opts[3].extra = (void*)mc->marshal_as<const char*>(m_password);
#endif //DOTNET_35

		conn_opts.count = 4;
		conn_opts.array = opts;

		m_connection = new SqlConnectionWrapper(env.ref()->createSqlConnection(&conn_opts));
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

#pragma region DbConnection Overrides
#pragma region Properties
void NuoDb::NuoDbConnection::ConnectionString::set(System::String^ value)
{
	if (System::String::IsNullOrEmpty(value))
		throw gcnew ArgumentNullException("value");

	m_connectionString = value;

	ParseConnectionString();
}

String^ NuoDb::NuoDbConnection::ServerVersion::get()
{
	if (NULL == m_connection)
		throw gcnew NuoDbException("The connection is closed.");

	try
	{
		return gcnew System::String(m_connection->ref()->getMetaData()->getDatabaseVersion());
	}
	catch (nuodb::sqlapi::ErrorCodeException& e)
	{
		throw gcnew NuoDbException(e);
	}
}

ConnectionState NuoDb::NuoDbConnection::State::get()
{
	return NULL == m_connection ? ConnectionState::Closed : ConnectionState::Open;
}
#pragma endregion

#pragma region Methods
NuoDb::NuoDbTransaction^ NuoDb::NuoDbConnection::BeginTransaction()
{
	if (NULL == m_connection)
		throw gcnew NuoDbException("The connection is closed.");

	if (m_inTransaction)
		throw gcnew NuoDbException("A transaction is already underway.");

	m_inTransaction = true;
	m_connection->ref()->setAutoCommit(false);

	return gcnew NuoDbTransaction(this, System::Data::IsolationLevel::ReadCommitted);
}

NuoDb::NuoDbTransaction^ NuoDb::NuoDbConnection::BeginTransaction(IsolationLevel isolationLevel)
{
	if (NULL == m_connection)
		throw gcnew NuoDbException("The connection is closed.");

	if (m_inTransaction)
		throw gcnew NuoDbException("A transaction is already underway.");

	if (isolationLevel != System::Data::IsolationLevel::ReadCommitted)
		throw gcnew NuoDbException("The isolation level is unsupported.");

	m_inTransaction = true;
	m_connection->ref()->setAutoCommit(false);
	
	return gcnew NuoDbTransaction(this, isolationLevel);
}

DbTransaction^ NuoDb::NuoDbConnection::BeginDbTransaction(IsolationLevel isolationLevel)
{
	return BeginTransaction(isolationLevel);
}

void NuoDb::NuoDbConnection::ChangeDatabase(System::String^ databaseName)
{
	if (System::String::IsNullOrEmpty(databaseName))
		throw gcnew ArgumentNullException("databaseName");

	Close();

	m_schema = databaseName;

	OpenConnection();
}

void NuoDb::NuoDbConnection::Close()
{
	RollbackTransaction();

	if (m_connection != NULL)
		delete m_connection;

	m_connection = NULL;
}

void NuoDb::NuoDbConnection::EnlistTransaction(System::Transactions::Transaction^ transaction)
{
	throw gcnew NotSupportedException();
}

void NuoDb::NuoDbConnection::Open()
{
	OpenConnection();
}

DbCommand^ NuoDb::NuoDbConnection::CreateDbCommand()
{
	return CreateCommand();
}
#pragma endregion
#pragma endregion
