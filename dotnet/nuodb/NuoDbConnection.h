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

NUODB_NAMESPACE_BEGIN

public ref class NuoDbConnection sealed : public System::Data::Common::DbConnection, IDisposable
{
private:
	bool _disposed;
	String^ _connectionString;
	String^ _server;
	String^ _database;
	String^ _schema;
	String^ _username;
	String^ _password;
	NuoDB::Connection* _connection;
	NuoDB::DatabaseMetaData* _metadata;
	bool _inTransaction;

public:
	NuoDbConnection();
	NuoDbConnection(String^ connectionString);
	~NuoDbConnection();
	!NuoDbConnection();

internal:
	property NuoDB::Connection* InternalConnection { NuoDB::Connection* get() { return _connection; } }
	property bool IsOpen { bool get() { return _connection != NULL; } }
	property bool IsInTransaction { bool get() { return _inTransaction; } }

public:
	NuoDbCommand^ CreateCommand();
	bool Ping();

internal:
	void CommitTransaction();
	void RollbackTransaction();

private:
	void OpenConnection();
	void ParseConnectionString();

public:
	virtual property String^ ConnectionString
	{
		String^ get() override { return _connectionString; }
		void set(String^ value) override;
	}

	virtual property String^ Database { String^ get() override { return _schema; } }
	virtual property String^ DataSource { String^ get() override { return _database; } }
	virtual property String^ ServerVersion { String^ get() override; }
	virtual property System::Data::ConnectionState State { System::Data::ConnectionState get() override; }

public:
	virtual NuoDbTransaction^ BeginTransaction() new;
	virtual NuoDbTransaction^ BeginTransaction(System::Data::IsolationLevel isolationLevel) new;
	virtual System::Data::Common::DbTransaction^ BeginDbTransaction(System::Data::IsolationLevel isolationLevel) override;
	virtual void ChangeDatabase(String^ databaseName) override;
	virtual void Close() override;
	virtual void EnlistTransaction(System::Transactions::Transaction^ transaction) override;
	virtual void Open() override;

protected:
	virtual System::Data::Common::DbCommand^ CreateDbCommand() override;
};

NUODB_NAMESPACE_END
