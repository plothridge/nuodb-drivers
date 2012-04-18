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

#include "NuoDbParameter.h"

NUODB_NAMESPACE_BEGIN

NuoDbParameter::NuoDbParameter() : 
	_dbType(NuoDb::NuoDbType::VarChar),
	_direction(System::Data::ParameterDirection::Input),
	_nullable(true),
	_name(String::Empty),
	_size(0),
	_source(String::Empty),
	_sourceNullMapping(false),
	_version(System::Data::DataRowVersion::Original)
{
}

void NuoDbParameter::NuoDbType::set(NuoDb::NuoDbType t)
{
	_dbType = t;
	_version = System::Data::DataRowVersion::Current;
}

System::Data::DbType NuoDbParameter::DbType::get()
{
	switch (_dbType)
	{
		case NuoDb::NuoDbType::Bit: 
		case NuoDb::NuoDbType::Boolean: 
			return System::Data::DbType::Boolean;

		case NuoDb::NuoDbType::TinyInt: return System::Data::DbType::Byte;
		case NuoDb::NuoDbType::SmallInt: return System::Data::DbType::Int16;
		case NuoDb::NuoDbType::Integer: return System::Data::DbType::Int32;
		case NuoDb::NuoDbType::BigInt: return System::Data::DbType::Int64;
		case NuoDb::NuoDbType::Float: return System::Data::DbType::Single;
		case NuoDb::NuoDbType::Double: return System::Data::DbType::Double;
		case NuoDb::NuoDbType::Char: return System::Data::DbType::StringFixedLength;

		case NuoDb::NuoDbType::VarChar:
		case NuoDb::NuoDbType::LongVarChar:
		case NuoDb::NuoDbType::Clob:
			return System::Data::DbType::String;

		case NuoDb::NuoDbType::Date: return System::Data::DbType::DateTime;

		case NuoDb::NuoDbType::Time: return System::Data::DbType::Time;
		case NuoDb::NuoDbType::Timestamp: return System::Data::DbType::DateTime2;
		
		case NuoDb::NuoDbType::Blob:
		case NuoDb::NuoDbType::Binary:
		case NuoDb::NuoDbType::LongVarBinary:
			return System::Data::DbType::Object;

		case NuoDb::NuoDbType::Numeric: return System::Data::DbType::VarNumeric;
		case NuoDb::NuoDbType::Decimal: return System::Data::DbType::Decimal;
			
		default: return System::Data::DbType::String;
	}
}

void NuoDbParameter::DbType::set(System::Data::DbType value)
{
	switch (value)
	{
		case System::Data::DbType::Byte: _dbType = NuoDb::NuoDbType::TinyInt; break;
		case System::Data::DbType::Boolean: _dbType = NuoDb::NuoDbType::Boolean; break;

		case System::Data::DbType::Currency:
		case System::Data::DbType::Double:
			_dbType = NuoDb::NuoDbType::Boolean;
			
			break;

		case System::Data::DbType::Date:
		case System::Data::DbType::DateTime:
			_dbType = NuoDb::NuoDbType::Date;
			
			break;

		case System::Data::DbType::Decimal: _dbType = NuoDb::NuoDbType::Decimal; break;

		case System::Data::DbType::Int16: _dbType = NuoDb::NuoDbType::SmallInt; break;
		case System::Data::DbType::Int32: _dbType = NuoDb::NuoDbType::Integer; break;
		case System::Data::DbType::Int64: _dbType = NuoDb::NuoDbType::BigInt; break;
		case System::Data::DbType::Single: _dbType = NuoDb::NuoDbType::Float; break;
		
		case System::Data::DbType::String: _dbType = NuoDb::NuoDbType::VarChar; break;
		case System::Data::DbType::Time: _dbType = NuoDb::NuoDbType::Time; break;
		case System::Data::DbType::StringFixedLength: _dbType = NuoDb::NuoDbType::Char; break;
		case System::Data::DbType::DateTime2: _dbType = NuoDb::NuoDbType::Timestamp; break;

		default: throw gcnew ArgumentException("value");
	}

	_version = System::Data::DataRowVersion::Current;
}


void NuoDbParameter::ParameterName::set(String^ value)
{
	if (String::IsNullOrEmpty(value))
		throw gcnew ArgumentNullException("ParameterName");

	_name = value;
}

void NuoDbParameter::Value::set(Object^ value)
{
	if (!_nullable && nullptr == value)
		throw gcnew ArgumentException("The parameter does not accept null values");

	_version = System::Data::DataRowVersion::Current;
	_value = value;
}

void NuoDbParameter::ResetDbType()
{
	_version = System::Data::DataRowVersion::Current;
	_dbType = NuoDb::NuoDbType::VarChar;
}

NuoDbParameter^ NuoDbParameterCollection::Add(String^ parameterName, NuoDb::NuoDbType type)
{
	if (String::IsNullOrEmpty(parameterName))
		throw gcnew ArgumentNullException("parameterName");

	if (IndexOf(parameterName) >= 0)
		throw gcnew ArgumentException("There is already a parameter named \"" + parameterName+ "\" in the collection", "value");

	NuoDbParameter^ p = gcnew NuoDbParameter();

	p->ParameterName = parameterName;
	p->NuoDbType = type;

	_parameters->Add(p);

	return p;
}

NuoDbParameter^ NuoDbParameterCollection::Add(String^ parameterName, NuoDbType type, int size)
{
	if (String::IsNullOrEmpty(parameterName))
		throw gcnew ArgumentNullException("parameterName");

	if (IndexOf(parameterName) >= 0)
		throw gcnew ArgumentException("There is already a parameter named \"" + parameterName + "\" in the collection", "value");

	NuoDbParameter^ p = gcnew NuoDbParameter();

	p->ParameterName = parameterName;
	p->NuoDbType = type;
	p->Size = size;

	_parameters->Add(p);

	return p;
}

NuoDbParameter^ NuoDbParameterCollection::Add(String^ parameterName, NuoDbType type, int size, String^ sourceColumn)
{
	if (String::IsNullOrEmpty(parameterName))
		throw gcnew ArgumentNullException("parameterName");

	if (IndexOf(parameterName) >= 0)
		throw gcnew ArgumentException("There is already a parameter named \"" + parameterName + "\" in the collection", "value");

	if (String::IsNullOrEmpty(sourceColumn))
		throw gcnew ArgumentNullException("sourceColumn");

	NuoDbParameter^ p = gcnew NuoDbParameter();

	p->ParameterName = parameterName;
	p->NuoDbType = type;
	p->Size = size;
	p->SourceColumn = sourceColumn;

	_parameters->Add(p);

	return p;
}

void NuoDbParameterCollection::AddRange(array<NuoDbParameter^>^ values)
{
	if (nullptr == values)
		throw gcnew ArgumentNullException("values");

	for each (NuoDbParameter^ p in values)
	{
		if (String::IsNullOrEmpty(p->ParameterName))
			throw gcnew ArgumentException("At least one parameter does not have a name.");

		if (IndexOf(p->ParameterName) >= 0)
			throw gcnew ArgumentException("There is already a parameter named \"" + p->ParameterName + "\" in the collection", "value");

		_parameters->Add(p);
	}
}

void NuoDbParameterCollection::CopyTo(array<NuoDbParameter^>^ a, int index)
{
	if (nullptr == a)
		throw gcnew ArgumentNullException("a");

	_parameters->CopyTo(a, index);
}

void NuoDbParameterCollection::Insert(int index, NuoDbParameter^ value)
{
	if (index < 0 || index >= _parameters->Count)
		throw gcnew IndexOutOfRangeException("index");

	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	if (IndexOf(value->ParameterName) >= 0)
		throw gcnew ArgumentException("There is already a parameter named \"" + value->ParameterName + "\" in the collection", "value");

	_parameters->Insert(index, value);
}

void NuoDbParameterCollection::Remove(NuoDbParameter^ value)
{
	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	_parameters->Remove(value);
}

NuoDbParameter^ NuoDbParameterCollection::default::get(int index)
{
	if (index < 0 || index >= _parameters->Count)
		throw gcnew IndexOutOfRangeException("index");

	return _parameters[index];
}

void NuoDbParameterCollection::default::set(int index, NuoDbParameter^ value)
{
	if (index < 0 || index >= _parameters->Count)
		throw gcnew IndexOutOfRangeException("index");

	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	if (String::IsNullOrEmpty(value->ParameterName))
		throw gcnew ArgumentException("The parameter does not have a name.");

	if (IndexOf(value->ParameterName) >= 0)
		throw gcnew ArgumentException("There is already a parameter named \"" + value->ParameterName + "\" in the collection", "value");

	_parameters[index] = value;
}

NuoDbParameter^ NuoDbParameterCollection::default::get(String^ parameterName)
{
	if (String::IsNullOrEmpty(parameterName))
		throw gcnew ArgumentNullException("parameterName");

	int index = IndexOf(parameterName);

	if (index < 0)
		throw gcnew IndexOutOfRangeException("parameterName");

	return _parameters[index];
}

void NuoDbParameterCollection::default::set(String^ parameterName, NuoDbParameter^ value)
{
	if (String::IsNullOrEmpty(parameterName))
		throw gcnew ArgumentNullException("parameterName");

	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	int index = IndexOf(parameterName);

	if (index < 0)
		throw gcnew IndexOutOfRangeException("parameterName");

	if (String::IsNullOrEmpty(value->ParameterName))
		throw gcnew ArgumentException("The parameter does not have a name.");

	if (value->ParameterName != parameterName && IndexOf(value->ParameterName) >= 0)
		throw gcnew ArgumentException("There is already a parameter named \"" + value->ParameterName + "\" in the collection", "value");

	_parameters[index] = value;
}

int NuoDbParameterCollection::Add(Object^ value)
{
	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	NuoDbParameter^ p = dynamic_cast<NuoDbParameter^>(value);

	if (nullptr == p)
		throw gcnew ArgumentException("value is not a NuoDbParameter", "value");

	if (String::IsNullOrEmpty(p->ParameterName))
		throw gcnew ArgumentException("The parameter does not have a name.");

	if (IndexOf(p->ParameterName) >= 0)
		throw gcnew ArgumentException("There is already a parameter named \"" + p->ParameterName + "\" in the collection", "value");

	_parameters->Add(p);

	return _parameters->Count - 1;
}

void NuoDbParameterCollection::AddRange(Array^ values)
{
	if (nullptr == values)
		throw gcnew ArgumentNullException("values");

	for each (Object^ o in values)
	{
		NuoDbParameter^ p = dynamic_cast<NuoDbParameter^>(o);

		if (nullptr == p)
			throw gcnew ArgumentException("At least one object in the array is not a NuoDbParameter", "values");

		if (System::String::IsNullOrEmpty(p->ParameterName))
			throw gcnew ArgumentException("At least one parameter does not have a name.");

		if (IndexOf(p->ParameterName) >= 0)
			throw gcnew ArgumentException("There is already a parameter named \"" + p->ParameterName + "\" in the collection", "value");

		_parameters->Add(p);
	}
}

bool NuoDbParameterCollection::Contains(Object^ value)
{
	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	if (0 == _parameters->Count)
		return false;

	for each (NuoDbParameter^ p in _parameters)
		if (value == p->Value)
			return true;

	return false;
}

bool NuoDbParameterCollection::Contains(String^ value)
{
	if (String::IsNullOrEmpty(value))
		throw gcnew ArgumentNullException("value");

	return IndexOf(value) >= 0;
}

void NuoDbParameterCollection::CopyTo(Array^ a, int index)
{
	if (nullptr == a)
		throw gcnew ArgumentNullException("a");

	Array::Copy((Array^)_parameters, a, index);
}

int NuoDbParameterCollection::IndexOf(Object^ value)
{
	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	for (int i = 0; i < _parameters->Count; i++)
		if (_parameters[i] == value)
			return i;

	return -1;
}

int NuoDbParameterCollection::IndexOf(String^ parameterName)
{
	if (String::IsNullOrEmpty(parameterName))
		throw gcnew ArgumentNullException("parameterName");

	for (int i = 0; i < _parameters->Count; i++)
		if (_parameters[i]->ParameterName == parameterName)
			return i;

	return -1;
}

void NuoDbParameterCollection::Insert(int index, Object^ value)
{
	if (index < 0 || index >= _parameters->Count)
		throw gcnew IndexOutOfRangeException("index");

	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	NuoDbParameter^ p = dynamic_cast<NuoDbParameter^>(value);

	if (nullptr == p)
		throw gcnew ArgumentException("value is not a NuoDbParameter", "value");

	if (System::String::IsNullOrEmpty(p->ParameterName))
		throw gcnew ArgumentException("The parameter does not have a name.");

	if (IndexOf(p->ParameterName) >= 0)
		throw gcnew ArgumentException("There is already a parameter named \"" + p->ParameterName + "\" in the collection", "value");

	_parameters->Insert(index, p);
}

void NuoDbParameterCollection::Remove(Object^ value)
{
	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	int index = IndexOf(value);

	if (index >= 0)
		_parameters->RemoveAt(index);
}

void NuoDbParameterCollection::RemoveAt(int index)
{
	if (index < 0 || index >= _parameters->Count)
		throw gcnew IndexOutOfRangeException("index");

	_parameters->RemoveAt(index);
}

void NuoDbParameterCollection::RemoveAt(String^ parameterName)
{
	if (String::IsNullOrEmpty(parameterName))
		throw gcnew ArgumentNullException("parameterName");

	int index = IndexOf(parameterName);

	if (index >= 0)
		_parameters->RemoveAt(index);
	else
		throw gcnew IndexOutOfRangeException("parameterName");
}

void NuoDbParameterCollection::SetParameter(int index, System::Data::Common::DbParameter^ value)
{
	if (index < 0 || index >= _parameters->Count)
		throw gcnew IndexOutOfRangeException("index");

	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	NuoDbParameter^ p = dynamic_cast<NuoDbParameter^>(value);

	if (nullptr == p)
		throw gcnew ArgumentException("value is not a NuoDbParameter", "value");

	if (IndexOf(p->ParameterName) >= 0)
		throw gcnew ArgumentException("There is already a parameter named \"" + p->ParameterName + "\" in the collection", "value");

	_parameters[index] = p;
}

void NuoDbParameterCollection::SetParameter(String^ parameterName, System::Data::Common::DbParameter^ value)
{
	if (String::IsNullOrEmpty(parameterName))
		throw gcnew ArgumentNullException("parameterName");

	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	NuoDbParameter^ p = dynamic_cast<NuoDbParameter^>(value);

	if (nullptr == p)
		throw gcnew ArgumentException("value is not a NuoDbParameter", "value");

	int index = IndexOf(parameterName);

	if (index < 0)
		throw gcnew IndexOutOfRangeException("parameterName");

	if (String::IsNullOrEmpty(p->ParameterName))
		throw gcnew ArgumentException("The parameter does not have a name.");

	if (p->ParameterName != parameterName && IndexOf(p->ParameterName) >= 0)
		throw gcnew ArgumentException("There is already a parameter named \"" + p->ParameterName + "\" in the collection", "value");

	_parameters[index] = p;
}

NUODB_NAMESPACE_END
