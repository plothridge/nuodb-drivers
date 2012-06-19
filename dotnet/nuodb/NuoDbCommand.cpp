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
#include "NuoDbParameter.h"
#include "NuoDbDataReader.h"

#include <time.h>

NUODB_NAMESPACE_BEGIN

public class NuoDbDateTime : public NuoDB::Date
{
private:
	long long _ms;

public:
	NuoDbDateTime(long long ms) :
		_ms(ms)
	{
	}

	~NuoDbDateTime() { }

	void addRef() { }
	int release() { return 0; }
	long long getSeconds() { return _ms / 1000; }
	void setSeconds(long long s) { _ms = s * 1000; }
	long long getMilliseconds() { return _ms; }
	void setMilliseconds(long long ms) { _ms = ms; }

	void getLocalTime(tm* timeStruct)
	{
		if (NULL == timeStruct)
			return;

		tm* t = ::gmtime((time_t*)&_ms);

		timeStruct->tm_sec = t->tm_sec;
		timeStruct->tm_min = t->tm_min;
		timeStruct->tm_hour = t->tm_hour;
		timeStruct->tm_mday = t->tm_mday;
		timeStruct->tm_mon = t->tm_mon;
		timeStruct->tm_year = t->tm_year;
		timeStruct->tm_wday = t->tm_wday;
		timeStruct->tm_yday = t->tm_yday;
		timeStruct->tm_isdst = t->tm_isdst;
	}
};

NuoDbCommand::NuoDbCommand() :
	_commandText(String::Empty),
	_commandTimeout(NUODB_DEFAULT_COMMAND_TIMEOUT),
	_commandType(System::Data::CommandType::Text),
	_parameters(gcnew NuoDbParameterCollection()),
	_designTimeVisible(false),
	_updateRowSource(System::Data::UpdateRowSource::None),
	_results(NULL),
	_metadata(NULL),
	_updateCount(0),
	_disposed(false)
{
}

NuoDbCommand::~NuoDbCommand()
{
	if (_disposed)
		return;

	this->!NuoDbCommand();

	_disposed = true;
}

NuoDbCommand::!NuoDbCommand()
{
	if (_results != NULL)
		_results->close();

	_results = NULL;
}

void NuoDbCommand::Connection::set(NuoDb::NuoDbConnection^ value)
{
	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	_connection = value;
}

void NuoDbCommand::Transaction::set(NuoDbTransaction^ value)
{
	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	_transaction = value;
	_connection = value->Connection;
}

NuoDbDataReader^ NuoDbCommand::ExecuteDataReader(System::Data::CommandBehavior behavior)
{
	if (nullptr == _connection)
		throw gcnew NuoDbException("No connection has been associated with the command.");

	if (!_connection->IsOpen)
		throw gcnew NuoDbException("The connection is not open.");

	if (String::IsNullOrEmpty(_commandText))
		throw gcnew NuoDbException("There is no CommandText.");

	Execute();

	NuoDbDataReader^ reader = gcnew NuoDbDataReader(_results, _metadata, this, behavior);

	_results = NULL; // we don't own it anymore!
	_metadata = NULL;

	return reader;
}

void NuoDbCommand::Execute()
{
	if (nullptr == _connection)
		throw gcnew NuoDbException("No connection has been associated with the command.");

	if (!_connection->IsOpen)
		throw gcnew NuoDbException("The connection is not open.");

	if (String::IsNullOrEmpty(_commandText))
		throw gcnew NuoDbException("There is no CommandText.");

	if (_results != NULL)
		_results->close();

	_results = NULL;
	_metadata = NULL;
	_updateCount = 0;

	msclr::interop::marshal_context^ mc = gcnew msclr::interop::marshal_context();

	try
	{
		NuoDB::Connection* c = _connection->InternalConnection;

#if DOTNET_35
		String^ local(_commandText);

		const char* s = mc->marshal_as<const char*>(local);
#else
		const char* s = mc->marshal_as<const char*>(_commandText);
#endif //DOTNET_35

		if (0 == _parameters->Count)
		{
			NuoDB::Statement* stmt = c->createStatement();

			_results = stmt->executeQuery(s);
			_updateCount = stmt->getUpdateCount();
		}
		else
		{
			NuoDB::PreparedStatement* stmt = c->prepareStatement(s);

			for (int i = 0; i < _parameters->Count; i++)
			{
				if (nullptr == _parameters[i]->Value)
				{
					stmt->setNull(i, (int)_parameters[i]->NuoDbType);

					continue;
				}

				switch (_parameters[i]->NuoDbType)
				{
					case NuoDbType::Bit:
					case NuoDbType::Boolean:
						stmt->setBoolean(i, Convert::ToBoolean(_parameters[i]->Value));

						break;

					case NuoDbType::TinyInt: stmt->setByte(i, Convert::ToByte(_parameters[i]->Value)); break;
					case NuoDbType::SmallInt: stmt->setShort(i, Convert::ToInt16(_parameters[i]->Value)); break;
					case NuoDbType::Integer: stmt->setInt(i, Convert::ToInt32(_parameters[i]->Value)); break;
					case NuoDbType::BigInt: stmt->setLong(i, Convert::ToInt64(_parameters[i]->Value)); break;
					case NuoDbType::Float: stmt->setFloat(i, Convert::ToSingle(_parameters[i]->Value)); break;
					case NuoDbType::Double: stmt->setDouble(i, Convert::ToDouble(_parameters[i]->Value)); break;

					case NuoDbType::Date:
					case NuoDbType::Time:
					case NuoDbType::Timestamp: 
						{
							DateTime d(Convert::ToDateTime(_parameters[i]->Value));
							TimeSpan t = d.Subtract(DateTime(1970, 1, 1));

							NuoDbDateTime sd((long long)t.TotalMilliseconds);

							stmt->setDate(i, &sd); 

							break;
						}

					case NuoDbType::Blob:
					case NuoDbType::Clob:
					case NuoDbType::Numeric:
					case NuoDbType::Decimal:
					case NuoDbType::Binary:
					case NuoDbType::LongVarBinary:
						throw gcnew NotImplementedException("The parameter data type for \"" + _parameters[i]->ParameterName + "\"is not supported");

					default: stmt->setString(i, mc->marshal_as<const char*>(_parameters[i]->Value->ToString())); break;
				}
			}

			_results = stmt->executeQuery();
			_updateCount = stmt->getUpdateCount();
		}

		_metadata = _results->getMetaData();
	}
	catch (NuoDB::SQLException& e)
	{
		_results = NULL;
		_metadata = NULL;
		_updateCount = 0;

		throw gcnew NuoDbException(e);
	}
	finally
	{
		delete mc;
	}
}

void NuoDbCommand::CommandText::set(String^ value)
{
	if (String::IsNullOrEmpty(value))
		throw gcnew ArgumentNullException("value");

	_commandText = value;
}

void NuoDbCommand::CommandType::set(System::Data::CommandType value)
{
	if (value != System::Data::CommandType::Text)
		throw gcnew NotSupportedException("Only CommandType.Text is supported");

	_commandType = value;
}

void NuoDbCommand::DbConnection::set(System::Data::Common::DbConnection^ value)
{
	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	NuoDbConnection^ p = dynamic_cast<NuoDbConnection^>(value);

	if (nullptr == p)
		throw gcnew ArgumentException("value is not a NuoDbConnection.");

	_connection = p;
}

System::Data::Common::DbParameterCollection^ NuoDbCommand::DbParameterCollection::get()
{
	return _parameters;
}

System::Data::Common::DbTransaction^ NuoDbCommand::DbTransaction::get()
{
	return _transaction;
}

void NuoDbCommand::DbTransaction::set(System::Data::Common::DbTransaction^ value)
{
	if (nullptr == value)
		throw gcnew ArgumentNullException("value");

	NuoDbTransaction^ p = dynamic_cast<NuoDbTransaction^>(value);

	if (nullptr == p)
		throw gcnew ArgumentException("value is not a NuoDbTransaction.");

	_transaction = p;
	_connection = p->Connection;
}

void NuoDbCommand::Cancel()
{
	throw gcnew NotSupportedException();
}

NuoDbParameter^ NuoDbCommand::CreateParameter()
{
	return gcnew NuoDbParameter();
}

System::Data::Common::DbParameter^ NuoDbCommand::CreateDbParameter()
{
	return CreateParameter();
}

System::Data::Common::DbDataReader^ NuoDbCommand::ExecuteDbDataReader(System::Data::CommandBehavior behavior)
{
	return ExecuteDataReader(behavior);
}

int NuoDbCommand::ExecuteNonQuery()
{
	if (nullptr == _connection)
		throw gcnew NuoDbException("No connection has been associated with the command.");

	if (!_connection->IsOpen)
		throw gcnew NuoDbException("The connection is not open.");

	if (String::IsNullOrEmpty(_commandText))
		throw gcnew NuoDbException("There is no CommandText.");

	Execute();

	_results->close();
	_results = NULL;
	_metadata = NULL;

	return _updateCount;
}

Object^ NuoDb::NuoDbCommand::ExecuteScalar()
{
	if (nullptr == _connection)
		throw gcnew NuoDbException("No connection has been associated with the command.");

	if (!_connection->IsOpen)
		throw gcnew NuoDbException("The connection is not open.");

	if (String::IsNullOrEmpty(_commandText))
		throw gcnew NuoDbException("There is no CommandText.");

	Execute();

	if (0 == _metadata->getColumnCount())
	{
		_results->close();
		_results = NULL;
		_metadata = NULL;

		return nullptr;
	}

	return NuoDbConnection::GetColumnValue(0, _results, _metadata);
}

void NuoDbCommand::Prepare()
{
	if (nullptr == _connection)
		throw gcnew NuoDbException("No connection has been associated with the command.");

	if (!_connection->IsOpen)
		throw gcnew NuoDbException("The connection is not open.");

	if (String::IsNullOrEmpty(_commandText))
		throw gcnew NuoDbException("There is no CommandText.");
}

NUODB_NAMESPACE_END
