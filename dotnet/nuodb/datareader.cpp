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
#include "command.h"
#include "datareader.h"

#pragma region Enumerator
private ref class NuoDbDataReaderEnumerator : System::Collections::IEnumerator
{
#pragma region Instance Fields
	NuoDb::NuoDbDataReader^ m_reader;
#pragma endregion

#pragma region Construction / Destruction
public:
	NuoDbDataReaderEnumerator(NuoDb::NuoDbDataReader^ r) :
		m_reader(r)
	{
		if (nullptr == r)
			throw gcnew ArgumentNullException("r");
	}
#pragma endregion

#pragma region IEnumerator Implementation
public:
	virtual property Object^ Current {
		Object^ get() { return m_reader; }
	}

	virtual bool MoveNext()
	{
		return m_reader->Read();
	}

	virtual void Reset()
	{
		throw gcnew NotSupportedException();
	}
#pragma endregion
};
#pragma endregion

#pragma region Construction / Destruction
NuoDb::NuoDbDataReader::NuoDbDataReader(SqlResultSetWrapper* results, NuoDbCommand^ command, System::Data::CommandBehavior behavior) :
	m_command(command),
	m_behavior(behavior),
	m_isClosed(false),
	m_results(results),
	m_disposed(false),
	m_columns(gcnew List<NuoDbColumnInfo^>())
{
	if (NULL == results)
		throw gcnew ArgumentNullException("results");

	if (nullptr == command)
		throw gcnew ArgumentNullException("command");

	if (nullptr == command->Connection)
		throw gcnew NuoDbException("The command has no connection.");

	if (!command->Connection->IsOpen)
		throw gcnew NuoDbException("The connection is not open.");

	ExtractMetadata();
}

NuoDb::NuoDbDataReader::~NuoDbDataReader()
{
	if (m_disposed)
		return;

	this->!NuoDbDataReader();

	m_disposed = true;
}

NuoDb::NuoDbDataReader::!NuoDbDataReader()
{
	Close();
}
#pragma endregion

#pragma region Implementation
void NuoDb::NuoDbDataReader::ExtractMetadata()
{
	for (int i = 0; i < (int)m_results->ref()->getColumnCount(); i++)
	{
		SqlColumnMetaDataWrapper d(m_results->ref()->getMetaData(i));

		m_columns.Add(gcnew NuoDbColumnInfo(gcnew System::String(d.ref()->getColumnName()), d.ref()->getType()));
	}
}
#pragma endregion

#pragma region DbDataReader Overrides
#pragma region Properties
int NuoDb::NuoDbDataReader::Depth::get()
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return 0;
}

int NuoDb::NuoDbDataReader::FieldCount::get()
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return m_columns.Count;
}

bool NuoDb::NuoDbDataReader::HasRows::get()
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	throw gcnew NotSupportedException();
}

int NuoDb::NuoDbDataReader::RecordsAffected::get()
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	throw gcnew NotSupportedException();
}

int NuoDb::NuoDbDataReader::VisibleFieldCount::get()
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return 0;
}

Object^ NuoDb::NuoDbDataReader::default::get(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= m_columns.Count)
		throw gcnew IndexOutOfRangeException("ordinal");

	switch (m_columns[ordinal]->Type)
	{
		case Boolean: return m_results->ref()->getBoolean(ordinal);
		case Integer: return m_results->ref()->getInteger(ordinal);
		case BigInt: return m_results->ref()->getLong(ordinal);
		case Double: return m_results->ref()->getDouble(ordinal);

		case Date:
		case Time:
		case DateTime:
			{
				SqlDateWrapper d((nuodb::sqlapi::SqlDate*)m_results->ref()->getDate(ordinal));

				System::DateTime epoch(1970, 1, 1);

				return epoch.AddMilliseconds((double)d.ref()->getTime());
			}

		default: break;
	}

	return gcnew System::String(m_results->ref()->getString(ordinal));
}

Object^ NuoDb::NuoDbDataReader::default::get(System::String^ name)
{
	if (System::String::IsNullOrEmpty(name))
		throw gcnew ArgumentNullException("name");

	return get(GetOrdinal(name));
}
#pragma endregion

#pragma region Methods
void NuoDb::NuoDbDataReader::Close()
{
	if (m_isClosed)
		return;

	delete m_results;

	m_results = NULL;

	if (CommandBehavior::CloseConnection == m_behavior)
		m_command->Connection->Close();

	m_isClosed = true;
}

bool NuoDb::NuoDbDataReader::GetBoolean(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= m_columns.Count)
		throw gcnew IndexOutOfRangeException("ordinal");

	return m_results->ref()->getBoolean(ordinal);
}

unsigned char NuoDb::NuoDbDataReader::GetByte(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	throw gcnew NotSupportedException();
}

long long NuoDb::NuoDbDataReader::GetBytes(int ordinal, long long dataOffset, array<unsigned char>^ buffer, int bufferOffset, int length)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	throw gcnew NotSupportedException();
}

wchar_t NuoDb::NuoDbDataReader::GetChar(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= m_columns.Count)
		throw gcnew IndexOutOfRangeException("ordinal");

	if (m_columns[ordinal]->Type != String)
		throw gcnew InvalidCastException();

	System::String^ s = gcnew System::String(m_results->ref()->getString(ordinal));

	return System::String::IsNullOrEmpty(s) ? '0' : s[0];
}

long long NuoDb::NuoDbDataReader::GetChars(int ordinal, long long dataOffset, array<wchar_t>^ buffer, int bufferOffset, int length)
{

	if (ordinal < 0 || ordinal >= m_columns.Count)
		throw gcnew IndexOutOfRangeException("ordinal");

	if (m_columns[ordinal]->Type != String)
		throw gcnew InvalidCastException();

	System::String^ s = gcnew System::String(m_results->ref()->getString(ordinal));

	if (System::String::IsNullOrEmpty(s))
		return 0;

	s->CopyTo((int)dataOffset, buffer, bufferOffset, (int)length);

	return length;
}

System::String^ NuoDb::NuoDbDataReader::GetDataTypeName(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= m_columns.Count)
		throw gcnew IndexOutOfRangeException("ordinal");

	switch (m_columns[ordinal]->Type)
	{
		case Boolean: return System::Boolean::typeid->FullName;
		case Integer: return System::Int32::typeid->FullName;
		case BigInt: return System::Int64::typeid->FullName;
		case Double: return System::Double::typeid->FullName;

		case Date:
		case Time:
		case DateTime:
			return System::DateTime::typeid->FullName;

		default: break;
	}

	return System::String::typeid->FullName;
}

DateTime NuoDb::NuoDbDataReader::GetDateTime(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= m_columns.Count)
		throw gcnew IndexOutOfRangeException("ordinal");

	if (m_columns[ordinal]->Type != DateTime)
		throw gcnew InvalidCastException();

	SqlDateWrapper d((nuodb::sqlapi::SqlDate*)m_results->ref()->getDate(ordinal));

	System::DateTime epoch(1970, 1, 1);

	return epoch.AddMilliseconds((double)d.ref()->getTime());
}

Decimal NuoDb::NuoDbDataReader::GetDecimal(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= m_columns.Count)
		throw gcnew IndexOutOfRangeException("ordinal");

	if (m_columns[ordinal]->Type != Double)
		throw gcnew InvalidCastException();

	return Decimal(m_results->ref()->getDouble(ordinal));
}

Double NuoDb::NuoDbDataReader::GetDouble(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= m_columns.Count)
		throw gcnew IndexOutOfRangeException("ordinal");

	if (m_columns[ordinal]->Type != Double)
		throw gcnew InvalidCastException();

	return m_results->ref()->getDouble(ordinal);
}

System::Collections::IEnumerator^ NuoDb::NuoDbDataReader::GetEnumerator()
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return gcnew NuoDbDataReaderEnumerator(this);
}

Type^ NuoDb::NuoDbDataReader::GetFieldType(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= m_columns.Count)
		throw gcnew IndexOutOfRangeException("ordinal");

	switch (m_columns[ordinal]->Type)
	{
		case Boolean: return System::Boolean::typeid;
		case Integer: return System::Int32::typeid;
		case BigInt: return System::Int64::typeid;
		case Double: return System::Double::typeid;

		case Date:
		case Time:
		case DateTime:
			return System::DateTime::typeid;

		default: break;
	}

	return System::String::typeid;
}

Single NuoDb::NuoDbDataReader::GetFloat(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= m_columns.Count)
		throw gcnew IndexOutOfRangeException("ordinal");

	if (m_columns[ordinal]->Type != Double)
		throw gcnew InvalidCastException();

	return Single(m_results->ref()->getDouble(ordinal));
}

Guid NuoDb::NuoDbDataReader::GetGuid(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	throw gcnew NotSupportedException();
}

short NuoDb::NuoDbDataReader::GetInt16(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= m_columns.Count)
		throw gcnew IndexOutOfRangeException("ordinal");

	if (m_columns[ordinal]->Type != Integer && m_columns[ordinal]->Type != BigInt)
		throw gcnew InvalidCastException();

	return (short)m_results->ref()->getInteger(ordinal);
}

int NuoDb::NuoDbDataReader::GetInt32(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= m_columns.Count)
		throw gcnew IndexOutOfRangeException("ordinal");

	if (m_columns[ordinal]->Type != Integer)
		throw gcnew InvalidCastException();

	return m_results->ref()->getInteger(ordinal);
}

long long NuoDb::NuoDbDataReader::GetInt64(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= m_columns.Count)
		throw gcnew IndexOutOfRangeException("ordinal");

	if (m_columns[ordinal]->Type != BigInt)
		throw gcnew InvalidCastException();

	return m_results->ref()->getLong(ordinal);
}

System::String^ NuoDb::NuoDbDataReader::GetName(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= m_columns.Count)
		throw gcnew IndexOutOfRangeException("ordinal");

	return m_columns[ordinal]->Name;
}

int NuoDb::NuoDbDataReader::GetOrdinal(System::String^ name)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	for (int i = 0; i < m_columns.Count; i++)
	{
		if (name == m_columns[i]->Name)
			return i;
	}

	throw gcnew IndexOutOfRangeException("name");
}

Type^ NuoDb::NuoDbDataReader::GetProviderSpecificFieldType(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return GetFieldType(ordinal);
}

Object^ NuoDb::NuoDbDataReader::GetProviderSpecificValue(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return this[ordinal];
}

int NuoDb::NuoDbDataReader::GetProviderSpecificValues(array<Object^>^ values)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return DbDataReader::GetProviderSpecificValues(values);
}

DataTable^ NuoDb::NuoDbDataReader::GetSchemaTable()
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	throw gcnew NotSupportedException();
}

System::String^ NuoDb::NuoDbDataReader::GetString(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= m_columns.Count)
		throw gcnew IndexOutOfRangeException("ordinal");

	if (m_columns[ordinal]->Type != String)
		throw gcnew InvalidCastException();

	return gcnew System::String(m_results->ref()->getString(ordinal));
}

Object^ NuoDb::NuoDbDataReader::GetValue(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= m_columns.Count)
		throw gcnew IndexOutOfRangeException("ordinal");

	return this[ordinal];
}

int NuoDb::NuoDbDataReader::GetValues(array<Object^>^ values)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (nullptr == values)
		throw gcnew ArgumentNullException("values");

	if (values->Length < m_columns.Count)
		throw gcnew ArgumentException("values does not have the required number of elements");

	for (int i = 0; i < m_columns.Count; i++)
	{
		values[i] = this[i];
	}

	return m_columns.Count;
}

bool NuoDb::NuoDbDataReader::IsDBNull(int ordinal)
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	throw gcnew NotSupportedException();
}

bool NuoDb::NuoDbDataReader::NextResult()
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return false;
}

bool NuoDb::NuoDbDataReader::Read()
{
	if (m_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return m_results->ref()->next();
}
#pragma endregion
#pragma endregion
