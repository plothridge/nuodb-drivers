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

public ref class NuoDbParameter sealed : public System::Data::Common::DbParameter
{
private:
	NuoDbType _dbType;
	System::Data::ParameterDirection _direction;
	bool _nullable;
	String^ _name;
	int _size;
	String^ _source;
	bool _sourceNullMapping;
	System::Data::DataRowVersion _version;
	Object^ _value;

public:
	NuoDbParameter();

public:
	property NuoDb::NuoDbType NuoDbType
	{
		NuoDb::NuoDbType get() { return _dbType; }
		void set(NuoDb::NuoDbType t);
	}

public:
	virtual property System::Data::DbType DbType
	{
		System::Data::DbType get() override;
		void set (System::Data::DbType value) override;
	}

	virtual property System::Data::ParameterDirection Direction
	{
		System::Data::ParameterDirection get() override { return _direction; }
		void set(System::Data::ParameterDirection value) override { _direction = value; }
	}

	virtual property bool IsNullable
	{
		bool get() override { return _nullable; }
		void set(bool value) override { _nullable = value; }
	}

	virtual property String^ ParameterName
	{
		String^ get() override { return _name; }
		void set(String^ value) override;
	}

	virtual property int Size
	{
		int get() override { return _size; }
		void set(int value) override { _size = value; }
	}

	virtual property String^ SourceColumn
	{
		String^ get() override { return _source; }
		void set(String^ value) override { _source = value; }
	}

	virtual property bool SourceColumnNullMapping
	{
		bool get() override { return _sourceNullMapping; }
		void set(bool value) override { _sourceNullMapping = value; }
	}

	virtual property System::Data::DataRowVersion SourceVersion
	{
		System::Data::DataRowVersion get() override { return _version; }
		void set(System::Data::DataRowVersion value) override { _version = value; }
	}

	virtual property Object^ Value
	{
		Object^ get() override { return _value; }
		void set(Object^ value) override;
	}

public:
	virtual void ResetDbType() override;
};

public ref class NuoDbParameterCollection sealed : public System::Data::Common::DbParameterCollection
{
private:
	System::Collections::Generic::List<NuoDbParameter^>^ _parameters;

public:
	NuoDbParameterCollection() :
		_parameters(gcnew System::Collections::Generic::List<NuoDbParameter^>())
	{
	}

public:
	NuoDbParameter^ Add(String^ parameterName, NuoDbType type);
	NuoDbParameter^ Add(String^ parameterName, NuoDbType type, int size);
	NuoDbParameter^ Add(String^ parameterName, NuoDbType type, int size, String^ sourceColumn);
	void AddRange(array<NuoDbParameter^>^ values);
	void CopyTo(array<NuoDbParameter^>^ a, int index);
	void Insert(int index, NuoDbParameter^ value);
	void Remove(NuoDbParameter^ value);

public:
	virtual property int Count { int get() override { return _parameters->Count; } }
	virtual property bool IsFixedSize { bool get() override { return ((System::Collections::IList^)_parameters)->IsFixedSize; } }
	virtual property bool IsReadOnly { bool get() override { return ((System::Collections::IList^)_parameters)->IsReadOnly; } }
	virtual property bool IsSynchronized { bool get() override { return ((System::Collections::IList^)_parameters)->IsSynchronized; } }
	virtual property Object^ SyncRoot { Object^ get() override { return this; } }

	virtual property NuoDbParameter^ default[int]
	{
		NuoDbParameter^ get(int index);
		void set(int index, NuoDbParameter^ value);
	}
	
	virtual property NuoDbParameter^ default[String^]
	{
		NuoDbParameter^ get(String^ parameterName);
		void set(String^ parameterName, NuoDbParameter^ value);
	}

public:
	virtual int Add(Object^ value) override;
	virtual void AddRange(Array^ values) override;
	virtual void Clear() override { _parameters->Clear(); }
	virtual bool Contains(Object^ value) override;
	virtual bool Contains(String^ value) override;
	virtual void CopyTo(Array^ a, int index) override;
	virtual System::Collections::IEnumerator^ GetEnumerator() override { return _parameters->GetEnumerator(); }
	virtual System::Data::Common::DbParameter^ GetParameter(int index) override { return this[index]; }
	virtual System::Data::Common::DbParameter^ GetParameter(String^ parameterName) override { return this[parameterName]; }
	virtual int IndexOf(Object^ value) override;
	virtual int IndexOf(String^ parameterName) override;
	virtual void Insert(int index, Object^ value) override;
	virtual void Remove(Object^ value) override;
	virtual void RemoveAt(int index) override;
	virtual void RemoveAt(String^ parameterName) override;

protected:
	virtual void SetParameter(int index, System::Data::Common::DbParameter^ value) override;
	virtual void SetParameter(String^ parameterName, System::Data::Common::DbParameter^ value) override;
};

NUODB_NAMESPACE_END
