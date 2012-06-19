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
#include "NuoDbCommand.h"
#include "NuoDbDataReader.h"

NUODB_NAMESPACE_BEGIN

private ref class NuoDbDataReaderEnumerator : System::Collections::IEnumerator
{
private:
	NuoDbDataReader^ _reader;

public:
	NuoDbDataReaderEnumerator(NuoDbDataReader^ r) :
		_reader(r)
	{
		if (nullptr == r)
			throw gcnew ArgumentNullException("r");
	}

public:
	virtual property Object^ Current { Object^ get() { return _reader; } }

public:
	virtual bool MoveNext() { return _reader->Read(); }
	virtual void Reset() { throw gcnew NotSupportedException(); }
};

NuoDbDataReader::NuoDbDataReader(NuoDB::ResultSet* results, NuoDB::ResultSetMetaData* metadata, NuoDbCommand^ command, System::Data::CommandBehavior behavior) :
	_command(command),
	_behavior(behavior),
	_isClosed(false),
	_results(results),
	_metadata(metadata),
	_disposed(false),
	_columns(gcnew System::Collections::Generic::List<NuoDbColumnInfo^>())
{
	if (NULL == results)
		throw gcnew ArgumentNullException("results");

	if (NULL == metadata)
		throw gcnew ArgumentNullException("metadata");

	if (nullptr == command)
		throw gcnew ArgumentNullException("command");

	if (nullptr == command->Connection)
		throw gcnew NuoDbException("The command has no connection.");

	if (!command->Connection->IsOpen)
		throw gcnew NuoDbException("The connection is not open.");
}
	
NuoDbDataReader::~NuoDbDataReader()
{
	if (_disposed)
		return;

	this->!NuoDbDataReader();

	_disposed = true;
}

NuoDbDataReader::!NuoDbDataReader()
{
	Close();
}

int NuoDbDataReader::Depth::get()
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return 0;
}

int NuoDbDataReader::FieldCount::get()
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return _metadata->getColumnCount();
}

bool NuoDbDataReader::HasRows::get()
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	throw gcnew NotSupportedException();
}

int NuoDbDataReader::RecordsAffected::get()
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	throw gcnew NotSupportedException();
}

int NuoDbDataReader::VisibleFieldCount::get()
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return 0;
}

Object^ NuoDbDataReader::default::get(int ordinal)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return NuoDbConnection::GetColumnValue(ordinal, _results, _metadata);
}

Object^ NuoDbDataReader::default::get(String^ name)
{
	if (String::IsNullOrEmpty(name))
		throw gcnew ArgumentNullException("name");

	return get(GetOrdinal(name));
}

void NuoDbDataReader::Close()
{
	if (_isClosed)
		return;

	_results->close();
	_results = NULL;
	_metadata = NULL;

	if (System::Data::CommandBehavior::CloseConnection == _behavior)
		_command->Connection->Close();

	_isClosed = true;
}

bool NuoDbDataReader::GetBoolean(int ordinal)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= _metadata->getColumnCount())
		throw gcnew IndexOutOfRangeException("ordinal");

	return _results->getBoolean(ordinal);
}

unsigned char NuoDbDataReader::GetByte(int ordinal)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= _metadata->getColumnCount())
		throw gcnew IndexOutOfRangeException("ordinal");

	return _results->getByte(ordinal);
}

long long NuoDbDataReader::GetBytes(int ordinal, long long dataOffset, array<unsigned char>^ buffer, int bufferOffset, int length)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	throw gcnew NotSupportedException();
}

wchar_t NuoDbDataReader::GetChar(int ordinal)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= _metadata->getColumnCount())
		throw gcnew IndexOutOfRangeException("ordinal");

	return _results->getByte(ordinal);
}

long long NuoDbDataReader::GetChars(int ordinal, long long dataOffset, array<wchar_t>^ buffer, int bufferOffset, int length)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (nullptr == buffer)
		throw gcnew ArgumentNullException("buffer");

	if (ordinal < 0 || ordinal >= _metadata->getColumnCount())
		throw gcnew IndexOutOfRangeException("ordinal");

	System::String^ s = gcnew System::String(_results->getString(ordinal));

	if (System::String::IsNullOrEmpty(s))
		return 0;

	s->CopyTo((int)dataOffset, buffer, bufferOffset, (int)length);

	return length;
}

DateTime NuoDbDataReader::GetDateTime(int ordinal)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= _metadata->getColumnCount())
		throw gcnew IndexOutOfRangeException("ordinal");

	NuoDB::Date* d = _results->getDate(ordinal);

	if (d != NULL)
		return DateTime(1970, 1, 1).AddMilliseconds((double)d->getMilliseconds());

	return DateTime::MinValue;
}

Decimal NuoDbDataReader::GetDecimal(int ordinal)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= _metadata->getColumnCount())
		throw gcnew IndexOutOfRangeException("ordinal");

	return Decimal(_results->getDouble(ordinal));
}

Double NuoDbDataReader::GetDouble(int ordinal)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= _metadata->getColumnCount())
		throw gcnew IndexOutOfRangeException("ordinal");

	return _results->getDouble(ordinal);
}

System::Collections::IEnumerator^ NuoDbDataReader::GetEnumerator()
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return gcnew NuoDbDataReaderEnumerator(this);
}

Type^ NuoDbDataReader::GetFieldType(int ordinal)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= _metadata->getColumnCount())
		throw gcnew IndexOutOfRangeException("ordinal");

	switch (_metadata->getColumnType(ordinal))
	{
		case NuoDB::NUOSQL_BIT:
		case NuoDB::NUOSQL_BOOLEAN:
			return Boolean::typeid;

		case NuoDB::NUOSQL_TINYINT: Byte::typeid;
		case NuoDB::NUOSQL_SMALLINT: Int16::typeid;
		case NuoDB::NUOSQL_INTEGER: Int32::typeid;
		case NuoDB::NUOSQL_BIGINT: Int64::typeid;
		case NuoDB::NUOSQL_FLOAT: Single::typeid;
		case NuoDB::NUOSQL_DOUBLE: Double::typeid;

		case NuoDB::NUOSQL_DATE:
		case NuoDB::NUOSQL_TIMESTAMP:
		case NuoDB::NUOSQL_TIME:
			return DateTime::typeid;

		default: break;
	}

	return String::typeid;
}

Single NuoDbDataReader::GetFloat(int ordinal)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= _metadata->getColumnCount())
		throw gcnew IndexOutOfRangeException("ordinal");

	return _results->getFloat(ordinal);
}

Guid NuoDbDataReader::GetGuid(int ordinal)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	throw gcnew NotSupportedException();
}

short NuoDbDataReader::GetInt16(int ordinal)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= _metadata->getColumnCount())
		throw gcnew IndexOutOfRangeException("ordinal");

	return _results->getShort(ordinal);
}

int NuoDbDataReader::GetInt32(int ordinal)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= _metadata->getColumnCount())
		throw gcnew IndexOutOfRangeException("ordinal");

	return _results->getInt(ordinal);
}

long long NuoDbDataReader::GetInt64(int ordinal)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= _metadata->getColumnCount())
		throw gcnew IndexOutOfRangeException("ordinal");

	return _results->getLong(ordinal);
}

String^ NuoDbDataReader::GetName(int ordinal)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= _metadata->getColumnCount())
		throw gcnew IndexOutOfRangeException("ordinal");

	return gcnew String(_metadata->getColumnName(ordinal));
}

int NuoDbDataReader::GetOrdinal(String^ name)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	for (int i = 0; i < _metadata->getColumnCount(); i++)
	{
		if (name == gcnew String(_metadata->getColumnName(i)))
			return i;
	}

	throw gcnew IndexOutOfRangeException("name");
}

int NuoDbDataReader::GetProviderSpecificValues(array<Object^>^ values)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return DbDataReader::GetProviderSpecificValues(values);
}

System::Data::DataTable^ NuoDbDataReader::GetSchemaTable()
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	throw gcnew NotSupportedException();
}

String^ NuoDb::NuoDbDataReader::GetString(int ordinal)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= _metadata->getColumnCount())
		throw gcnew IndexOutOfRangeException("ordinal");

	return gcnew String(_results->getString(ordinal));
}

int NuoDbDataReader::GetValues(array<Object^>^ values)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (nullptr == values)
		throw gcnew ArgumentNullException("values");

	int l = _metadata->getColumnCount();

	if (values->Length < l)
		throw gcnew ArgumentException("values does not have the required number of elements");

	for (int i = 0; i < l; i++)
		values[i] = this[i];

	return l;
}

bool NuoDbDataReader::IsDBNull(int ordinal)
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	if (ordinal < 0 || ordinal >= _metadata->getColumnCount())
		throw gcnew IndexOutOfRangeException("ordinal");

	return NuoDB::NUOSQL_NULL == _metadata->getColumnType(ordinal);
}

bool NuoDbDataReader::NextResult()
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return false;
}

bool NuoDbDataReader::Read()
{
	if (_isClosed)
		throw gcnew NuoDbException("The reader is closed.");

	return _results->next();
}

NUODB_NAMESPACE_END
