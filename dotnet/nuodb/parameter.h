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

namespace NuoDb
{
	ref class NuoDbParameter;
	ref class NuoDbCommand;

	/// <summary>
	/// Represents a parameter to a <see cref="NuoDbCommand"/> and optionally
	/// its mapping to <see cref="DataSet"/> columns.
	/// This class cannot be inherited.
	/// </summary>
	public ref class NuoDbParameter sealed : public DbParameter
	{
#pragma region Instance Fields
	private:
		NuoDbType m_dbType;
		ParameterDirection m_direction;
		bool m_nullable;
		System::String^ m_name;
		int m_size;
		System::String^ m_source;
		bool m_sourceNullMapping;
		DataRowVersion m_version;
		Object^ m_value;
#pragma endregion

#pragma region Construction / Destruction
	public:
		NuoDbParameter();
#pragma endregion

#pragma region Properties
	public:
		property NuoDb::NuoDbType NuoDbType
		{
			NuoDb::NuoDbType get() { return m_dbType; }
			void set(NuoDb::NuoDbType t);
		}
#pragma endregion

#pragma region DbParameter Overrides
#pragma region Properties
	public:
		/// <summary>
		/// Gets or sets the <see cref="DbType"/> of the parameter.
		/// </summary>
		virtual property System::Data::DbType DbType
		{
			System::Data::DbType get() override;
			void set (System::Data::DbType value) override;
		}

		/// <summary>
		/// Gets or sets a value that indicates whether the parameter is input-only,
		/// output-only, bidirectional, or a stored procedure return value parameter.
		/// </summary>
		virtual property ParameterDirection Direction
		{
			ParameterDirection get() override { return m_direction; }
			void set(ParameterDirection value) override { m_direction = value; }
		}

		/// <summary>
		/// Gets or sets a value that indicates whether the parameter accepts null values.
		/// </summary>
		virtual property bool IsNullable
		{
			bool get() override { return m_nullable; }
			void set(bool value) override { m_nullable = value; }
		}

		/// <summary>
		/// Gets or sets the name of the <see cref="NuoDbParameter"/>.
		/// </summary>
		virtual property System::String^ ParameterName
		{
			System::String^ get() override { return m_name; }
			void set(System::String^ value) override;
		}

		/// <summary>
		/// Gets or sets the maximum size, in bytes, of the data within the column.
		/// </summary>
		virtual property int Size
		{
			int get() override { return m_size; }
			void set(int value) override { m_size = value; }
		}

		/// <summary>
		/// Gets or sets the name of the source column mapped to the <see cref="DataSet"/>
		/// and used for loading or returning the <see cref="Value"/>.
		/// </summary>
		virtual property System::String^ SourceColumn
		{
			System::String^ get() override { return m_source; }
			void set(System::String^ value) override { m_source = value; }
		}

		/// <summary>
		/// Sets or gets a value which indicates whether the source column is nullable.
		/// This allows <see cref="DbCommandBuilder"/> to correctly generate Update
		/// statements for nullable columns.
		/// </summary>
		virtual property bool SourceColumnNullMapping
		{
			bool get() override { return m_sourceNullMapping; }
			void set(bool value) override { m_sourceNullMapping = value; }
		}

		/// <summary>
		/// Gets or sets the <see cref="DataRowVersion"/> to use when you load
		/// <see cref="Value"/>.
		/// </summary>
		virtual property DataRowVersion SourceVersion
		{
			DataRowVersion get() override { return m_version; }
			void set(DataRowVersion value) override { m_version = value; }
		}

		/// <summary>
		/// Gets or sets the value of the parameter.
		/// </summary>
		virtual property Object^ Value
		{
			Object^ get() override { return m_value; }
			void set(Object^ value) override;
		}
#pragma endregion

#pragma region Methods
	public:
		/// <summary>
		/// Resets the <see cref="DbType"/> property to its original settings.
		/// </summary>
		virtual void ResetDbType() override;
#pragma endregion
#pragma endregion
	};

	/// <summary>
	/// Represents a collection of <see cref="NuoDbParameter"/>s.
	/// This class cannot be inherited.
	/// </summary>
	public ref class NuoDbParameterCollection sealed : public DbParameterCollection
	{
#pragma region Instance Fields
	private:
		List<NuoDbParameter^>^ m_parameters;
#pragma endregion

#pragma region Construction / Destruction
	public:
		NuoDbParameterCollection();
#pragma endregion

#pragma region Methods
	public:
		/// <summary>
		/// Adds a <see cref="NuoDbParameter"/> to the <see cref="NuoDbParameterCollection"/> given the parameter name and the data type.
		/// </summary>
		/// <param name="parameterName">The parameter name.</param>
		/// <param name="type">The <see cref="NuoDbType"/>.</param>
		/// <returns>The new <see cref="NuoDbParameter"/>.</returns>
		NuoDbParameter^ Add(System::String^ parameterName, NuoDbType type);

		/// <summary>
		/// Adds a <see cref="NuoDbParameter"/> to the <see cref="NuoDbParameterCollection"/> given the parameter name and the data type.
		/// </summary>
		/// <param name="parameterName">The parameter name.</param>
		/// <param name="type">The <see cref="NuoDbType"/>.</param>
		/// <param name="size">The size.</param>
		/// <returns>The new <see cref="NuoDbParameter"/>.</returns>
		NuoDbParameter^ Add(System::String^ parameterName, NuoDbType type, int size);

		/// <summary>
		/// Adds a <see cref="NuoDbParameter"/> to the <see cref="NuoDbParameterCollection"/> given the parameter name and the data type.
		/// </summary>
		/// <param name="parameterName">The parameter name.</param>
		/// <param name="type">The <see cref="NuoDbType"/>.</param>
		/// <param name="size">The size.</param>
		/// <param name="sourceColumn">The name of the source column.</param>
		/// <returns>The new <see cref="NuoDbParameter"/>.</returns>
		NuoDbParameter^ Add(System::String^ parameterName, NuoDbType type, int size, System::String^ sourceColumn);

		/// <summary>
		/// Adds an array of items with the specified values to the <see cref="NuoDbParameterCollection"/>.
		/// </summary>
		/// <param name="values">The array of <see cref="NuoDbParameter"/>s to add.</param>
		void AddRange(array<NuoDbParameter^>^ values);

		/// <summary>
		/// Copies an array of items to the collection starting at the specified index.
		/// </summary>
		/// <param name="a">The array of items to copy to the collection.</param>
		/// <param name="index">The index in the collection to copy the items.</param>
		/// <exception cref="ArgumentNullException">The <paramref name="a"/> was <see langword="null"/>.</exception>
		/// <exception cref="IndexOutOfRangeException">The <paramref name="index"/> does not exist.</exception>
		void CopyTo(array<NuoDbParameter^>^ a, int index);

		/// <summary>
		/// Inserts a <see cref="NuoDbParameter"/> into the collection at the specified index.
		/// </summary>
		/// <param name="index">The index.</param>
		/// <param name="value">The <see cref="NuoDbParameter"/></param>
		/// <exception cref="IndexOutOfRangeException">The <paramref name="index"/> does not exist.</exception>
		void Insert(int index, NuoDbParameter^ value);

		/// <summary>
		/// Removes the specified <see cref="NuoDbParameter"/> object from the collection.
		/// </summary>
		/// <param name="value">The <see cref="NuoDbParameter"/></param>
		void Remove(NuoDbParameter^ value);
#pragma endregion

#pragma region DbParameterCollection Overrides
#pragma region Properties
	public:
		/// <summary>
		/// Specifies the number of items in the collection.
		/// </summary>
		virtual property int Count
		{
			int get() override { return m_parameters->Count; }
		}

		/// <summary>
		/// Specifies whether the collection is a fixed size.
		/// </summary>
		virtual property bool IsFixedSize
		{
			bool get() override { return ((System::Collections::IList^)m_parameters)->IsFixedSize; }
		}

		/// <summary>
		/// Specifies whether the collection is read-only.
		/// </summary>
		virtual property bool IsReadOnly
		{
			bool get() override { return ((System::Collections::IList^)m_parameters)->IsReadOnly; }
		}

		/// <summary>
		/// Specifies whether the collection is synchronized.
		/// </summary>
		virtual property bool IsSynchronized
		{
			bool get() override { return ((System::Collections::IList^)m_parameters)->IsSynchronized; }
		}

		/// <summary>
		/// Specifies the <see cref="System::Object"/> to be used to synchronize access to the collection.
		/// </summary>
		virtual property Object^ SyncRoot
		{
			Object^ get() override { return this; }
		}

		/// <summary>
		/// Gets and sets the <see cref="NuoDbParameter"/> at the specified index.
		/// </summary>
		/// <param name="index">The index.</param>
		/// <returns>When getting, a <see cref="NuoDbParameter"/>.</returns>
		/// <exception cref="IndexOutOfRangeException">The <paramref name="index"/> does not exist.</exception>
		virtual property NuoDbParameter^ default[int]
		{
			NuoDbParameter^ get(int index);
			void set(int index, NuoDbParameter^ value);
		}

		/// <summary>
		/// Gets or sets the <see cref="NuoDbParameter"/> parameter with the specified name.
		/// </summary>
		/// <param name="parameterName">The name of the parameter.</param>
		/// <returns>When getting, a <see cref="NuoDbParameter"/>.</returns>
		/// <exception cref="IndexOutOfRangeException">The <paramref name="parameterName"/> does not exist.</exception>
		virtual property NuoDbParameter^ default[System::String^]
		{
			NuoDbParameter^ get(System::String^ parameterName);
			void set(System::String^ parameterName, NuoDbParameter^ value);
		}
#pragma endregion

#pragma region Methods
	public:
		/// <summary>
		/// Adds a <see cref="NuoDbParameter"/> item with the specified value to the <see cref="NuoDbParameterCollection"/>.
		/// </summary>
		/// <param name="value">The value.</param>
		/// <returns>The index of the <see cref="NuoDbParameter"/> added to the collection.</returns>
		virtual int Add(Object^ value) override;
	
		/// <summary>
		/// Adds an array of items with the specified values to the <see cref="NuoDbParameterCollection"/>.
		/// </summary>
		/// <param name="values">The array of <see cref="NuoDbParameter"/>s to add.</param>
		virtual void AddRange(Array^ values) override;

		/// <summary>
		/// Removes all <see cref="NuoDbParameter"/> from the <see cref="NuoDbParameterCollection"/>.
		/// </summary>
		virtual void Clear() override { m_parameters->Clear(); }

		/// <summary>
		/// Indicates whether a <see cref="NuoDbParameter"/> with the specified <see cref="NuoDbParameter::Value"/> is contained in the collection.
		/// </summary>
		/// <param name="value">The value to look for in the collection.</param>
		/// <returns><see langword="true"/> if found; otherwise <see langword="false"/>.</returns>
		virtual bool Contains(Object^ value) override;

		/// <summary>
		/// Indicates whether a <see cref="NuoDbParameter"/> with the specified name exists in the collection.
		/// </summary>
		/// <param name="value">The parameter name.</param>
		/// <returns><see langword="true"/> if found; otherwise <see langword="false"/>.</returns>
		virtual bool Contains(System::String^ value) override;

		/// <summary>
		/// Copies an array of items to the collection starting at the specified index.
		/// </summary>
		/// <param name="a">The array of items to copy to the collection.</param>
		/// <param name="index">The index in the collection to copy the items.</param>
		/// <exception cref="ArgumentNullException">The <paramref name="a"/> was <see langword="null"/>.</exception>
		/// <exception cref="IndexOutOfRangeException">The <paramref name="index"/> does not exist.</exception>
		virtual void CopyTo(Array^ a, int index) override;

		/// <summary>
		/// Gets an enumerator that can be used to iterate through the collection.
		/// </summary>
		/// <returns>An enumerator that can be used to iterate through the collection.</returns>
		virtual System::Collections::IEnumerator^ GetEnumerator() override { return m_parameters->GetEnumerator(); }

		/// <summary>
		/// Returns the <see cref="DbParameter"/> object at the specified index.
		/// </summary>
		/// <param name="index">The index of the <see cref="DbParameter"/> in the collection.</param>
		/// <returns>The <see cref="DbParameter"/> object at the specified index in the collection.</returns>
		/// <exception cref="IndexOutOfRangeException">The <paramref name="index"/> does not exist.</exception>
		virtual DbParameter^ GetParameter(int index) override { return this[index]; }

		/// <summary>
		/// Returns the <see cref="DbParameter"/> object with the specified name.
		/// </summary>
		/// <param name="parameterName">The name of the <see cref="DbParameter"/> in the collection.</param>
		/// <returns>The <see cref="DbParameter"/> object with the specified name in the collection.</returns>
		/// <exception cref="IndexOutOfRangeException">The <paramref name="parameterName"/> does not exist.</exception>
		virtual DbParameter^ GetParameter(System::String^ parameterName) override { return this[parameterName]; }

		/// <summary>
		/// Returns the index of the specified <see cref="NuoDbParameter"/> object.
		/// </summary>
		/// <param name="value">The <see cref="NuoDbParameter"/>.</param>
		/// <returns>The index of the specified <see cref="NuoDbParameter"/>.</returns>
		virtual int IndexOf(Object^ value) override;

		/// <summary>
		/// Returns the index of the <see cref="NuoDbParameter"/> object with the specified name.
		/// </summary>
		/// <param name="parameterName">The name.</param>
		/// <returns>The index of the specified <see cref="NuoDbParameter"/>.</returns>
		virtual int IndexOf(System::String^ parameterName) override;

		/// <summary>
		/// Inserts a <see cref="NuoDbParameter"/> into the collection at the specified index.
		/// </summary>
		/// <param name="index">The index.</param>
		/// <param name="value">The <see cref="NuoDbParameter"/></param>
		/// <exception cref="IndexOutOfRangeException">The <paramref name="index"/> does not exist.</exception>
		virtual void Insert(int index, Object^ value) override;

		/// <summary>
		/// Removes the specified <see cref="NuoDbParameter"/> object from the collection.
		/// </summary>
		/// <param name="value">The <see cref="NuoDbParameter"/></param>
		virtual void Remove(Object^ value) override;

		/// <summary>
		/// Removes the <see cref="NuoDbParameter"/> object at the specified from the collection.
		/// </summary>
		/// <param name="index">The index.</param>
		/// <exception cref="IndexOutOfRangeException">The <paramref name="index"/> does not exist.</exception>
		virtual void RemoveAt(int index) override;

		/// <summary>
		/// Removes the <see cref="NuoDbParameter"/> object with the specifeid name from the collection.
		/// </summary>
		/// <param name="parameterName">The names.</param>
		/// <exception cref="IndexOutOfRangeException">The <paramref name="parameterName"/> does not exist.</exception>
		virtual void RemoveAt(System::String^ parameterName) override;

	protected:
		/// <summary>
		/// Sets the <see cref="NuoDbParameter"/> object at the specified index to a new value.
		/// </summary>
		/// <param name="index">The index to set.</param>
		/// <param name="value">The new value.</param>
		/// <exception cref="IndexOutOfRangeException">The <paramref name="index"/> does not exist.</exception>
		virtual void SetParameter(int index, DbParameter^ value) override;

		/// <summary>
		/// Sets the <see cref="NuoDbParameter"/> object with the specified name to a new value.
		/// </summary>
		/// <param name="parameterName">The parameter name.</param>
		/// <param name="value">The new value.</param>
		/// <exception cref="IndexOutOfRangeException">The <paramref name="parameterName"/> does not exist.</exception>
		virtual void SetParameter(System::String^ parameterName, DbParameter^ value) override;
#pragma endregion
#pragma endregion
	};
}
