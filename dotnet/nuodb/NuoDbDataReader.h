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

public ref class NuoDbDataReader sealed : public System::Data::Common::DbDataReader
{
private:
	NuoDbCommand^ _command;
	System::Data::CommandBehavior _behavior;
	bool _isClosed;
	NuoDB::ResultSet* _results;
	NuoDB::ResultSetMetaData* _metadata;
	bool _disposed;
	System::Collections::Generic::List<NuoDbColumnInfo^> _columns;

internal:
	NuoDbDataReader(NuoDB::ResultSet* results, NuoDB::ResultSetMetaData* metadata, NuoDbCommand^ command, System::Data::CommandBehavior behavior);

public:
	~NuoDbDataReader();
	!NuoDbDataReader();

public:
	virtual property int Depth { int get() override; }
	virtual property int FieldCount { int get() override; }
	virtual property bool HasRows { bool get() override; }
	virtual property bool IsClosed { bool get() override { return _isClosed; } }
	virtual property int RecordsAffected { int get() override; }
	virtual property int VisibleFieldCount { int get() override; }
	virtual property Object^ default[int] { Object^ get(int ordinal) override; }
	virtual property Object^ default[String^] { Object^ get(String^ name) override; }

public:
	virtual void Close() override;
	virtual bool GetBoolean(int ordinal) override;
	virtual unsigned char GetByte(int ordinal) override;
	virtual long long GetBytes(int ordinal, long long dataOffset, array<unsigned char>^ buffer, int bufferOffset, int length) override;
	virtual wchar_t GetChar(int ordinal) override;
	virtual long long GetChars(int ordinal, long long dataOffset, array<wchar_t>^ buffer, int bufferOffset, int length) override;
	virtual String^ GetDataTypeName(int ordinal) override { return GetFieldType(ordinal)->ToString(); }
	virtual System::DateTime GetDateTime(int ordinal) override;
	virtual Decimal GetDecimal(int ordinal) override;
	virtual Double GetDouble(int ordinal) override;
	virtual System::Collections::IEnumerator^ GetEnumerator() override;
	virtual Type^ GetFieldType(int ordinal) override;
	virtual Single GetFloat(int ordinal) override;
	virtual Guid GetGuid(int ordinal) override;
	virtual short GetInt16(int ordinal) override;
	virtual int GetInt32(int ordinal) override;
	virtual long long GetInt64(int ordinal) override;
	virtual String^ GetName(int ordinal) override;
	virtual int GetOrdinal(String^ name) override;
	virtual Type^ GetProviderSpecificFieldType(int ordinal) override { return GetFieldType(ordinal); }
	virtual Object^ GetProviderSpecificValue(int ordinal) override { return this[ordinal]; }
	virtual int GetProviderSpecificValues(array<Object^>^ values) override;
	virtual System::Data::DataTable^ GetSchemaTable() override;
	virtual String^ GetString(int ordinal) override;
	virtual Object^ GetValue(int ordinal) override { return this[ordinal]; }
	virtual int GetValues(array<Object^>^ values) override;
	virtual bool IsDBNull(int ordinal) override;
	virtual bool NextResult() override;
	virtual bool Read() override;
};

NUODB_NAMESPACE_END
