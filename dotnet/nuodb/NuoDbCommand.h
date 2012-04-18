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

public ref class NuoDbCommand sealed : public System::Data::Common::DbCommand
{
private:
	String^ _commandText;
	int _commandTimeout;
	System::Data::CommandType _commandType;
	NuoDbConnection^ _connection;
	NuoDbParameterCollection^ _parameters;
	NuoDbTransaction^ _transaction;
	bool _designTimeVisible;
	System::Data::UpdateRowSource _updateRowSource;
	NuoDB::ResultSet* _results;
	NuoDB::ResultSetMetaData* _metadata;
	int _updateCount;
	bool _disposed;

public:
	NuoDbCommand();
	~NuoDbCommand();
	!NuoDbCommand();

public:
	property NuoDb::NuoDbConnection^ Connection
	{
		NuoDb::NuoDbConnection^ get() { return _connection; }
		void set(NuoDb::NuoDbConnection^ value);
	}

	property NuoDbParameterCollection^ Parameters
	{
		NuoDbParameterCollection^ get() { return _parameters; }
	}

	property NuoDbTransaction^ Transaction
	{
		NuoDbTransaction^ get() { return _transaction; }
		void set(NuoDbTransaction^ value);
	}

public:
	NuoDbDataReader^ ExecuteDataReader(System::Data::CommandBehavior behavior);

private:
	void Execute();

public:
	virtual property String^ CommandText
	{
		String^ get() override { return _commandText; }
		void set(String^ value) override;
	}

	virtual property int CommandTimeout
	{
		int get() override { return _commandTimeout; }
		void set(int value) override { _commandTimeout = value; }
	}

	virtual property System::Data::CommandType CommandType
	{
		System::Data::CommandType get() override { return _commandType; }
		void set(System::Data::CommandType value) override;
	}

	virtual property System::Data::Common::DbConnection^ DbConnection
	{
		System::Data::Common::DbConnection^ get() override { return _connection; }
		void set(System::Data::Common::DbConnection^ value) override;
	}

	virtual property System::Data::Common::DbParameterCollection^ DbParameterCollection
	{
		System::Data::Common::DbParameterCollection^ get() override;
	}

	virtual property System::Data::Common::DbTransaction^ DbTransaction
	{
		System::Data::Common::DbTransaction^ get() override;
		void set(System::Data::Common::DbTransaction^ value) override;
	}

	virtual property bool DesignTimeVisible
	{
		bool get() override { return _designTimeVisible; }
		void set(bool value) override { _designTimeVisible = value; }
	}

	virtual property System::Data::UpdateRowSource UpdatedRowSource
	{
		System::Data::UpdateRowSource get() override { return _updateRowSource; }
		void set(System::Data::UpdateRowSource value) override { _updateRowSource = value; }
	}

public:
	virtual void Cancel() override;
	virtual NuoDbParameter^ CreateParameter() new;
	virtual System::Data::Common::DbParameter^ CreateDbParameter() override;
	virtual System::Data::Common::DbDataReader^ ExecuteDbDataReader(System::Data::CommandBehavior behavior) override;
	virtual int ExecuteNonQuery() override;
	virtual System::Object^ ExecuteScalar() override;
	virtual void Prepare() override;
};

NUODB_NAMESPACE_END
