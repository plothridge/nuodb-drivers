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
	ref class NuoDbCommand;

	/// <summary>
	/// Provides a way of reading a forward-only stream of rows from a NuoDb database.
	/// This class cannot be inherited.
	/// </summary>
	public ref class NuoDbDataReader sealed : DbDataReader
	{
#pragma region Instance Fields
	private:
		NuoDbCommand^ m_command;
		System::Data::CommandBehavior m_behavior;
		bool m_isClosed;
		SqlResultSetWrapper* m_results;
		bool m_disposed;
		List<NuoDbColumnInfo^> m_columns;
		//List<NuoDbType> m_columnTypes;
#pragma endregion

#pragma region Construction / Destruction
	internal:
		NuoDbDataReader(SqlResultSetWrapper* results, NuoDbCommand^ command, System::Data::CommandBehavior behavior);
		~NuoDbDataReader();

	protected:
		/// Disposes of this instance of <c>NuoDbDataReader</c>.
		!NuoDbDataReader();
#pragma endregion

#pragma region Implementation
	private:
		void ExtractMetadata();
#pragma endregion

#pragma region DbDataReader Overrides
#pragma region Properties
	public:
		/// <summary>
		/// Gets a value indicating the depth of nesting for the current row.
		/// </summary>
		virtual property int Depth
		{
			int get() override;
		}

		/// <summary>
		/// Gets the number of columns in the current row.
		/// </summary>
		virtual property int FieldCount
		{
			int get() override;
		}

		/// <summary>
		/// Gets a value that indicates whether this <see cref="NuoDbDataReader"/> contains one or more rows.
		/// </summary>
		virtual property bool HasRows
		{
			bool get() override;
		}

		/// <summary>
		/// Gets a value indicating whether the <see cref="NuoDbDataReader"/> is closed.
		/// </summary>
		virtual property bool IsClosed
		{
			bool get() override { return m_isClosed; }
		}

		/// <summary>
		/// Gets the number of rows changed, inserted, or deleted by execution of the SQL statement.
		/// </summary>
		/// <remarks>
		/// The number of rows changed, inserted, or deleted. -1 for SELECT statements;
		/// 0 if no rows were affected or the statement failed.
		/// </remarks>
		virtual property int RecordsAffected
		{
			int get() override;
		}

		/// <summary>
		/// Gets the number of fields in the <see cref="NuoDbDataReader"/> that are not hidden.
		/// </summary>
		virtual property int VisibleFieldCount
		{
			int get() override;
		}

		/// <summary>
		/// Gets the value of the specified column as an instance of <see cref="Object"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The value of the specified column.</returns>
		/// <exception cref="IndexOutOfRangeException">The index passed was outside the range of 0 through <see cref="FieldCount"/>.</exception>
		virtual property Object^ default[int]
		{
			Object^ get(int ordinal) override;
		}

		/// <summary>
		/// Gets the value of the specified column as an instance of <see cref="Object"/>.
		/// </summary>
		/// <param name="name">The name of the column.</param>
		/// <returns>The value of the specified column.</returns>
		/// <exception cref="IndexOutOfRangeException">No column with the specified name was found.</exception>
		virtual property Object^ default[System::String^]
		{
			Object^ get(System::String^ name) override;
		}
#pragma endregion

#pragma region Methods
	public:
		/// <summary>
		/// Closes the <see cref="NuoDbDataReader"/> object.
		/// </summary>
		virtual void Close() override;

		/// <summary>
		/// Gets the value of the specified column as a <see cref="Boolean"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The value of the specified column.</returns>
		/// <exception cref="InvalidCastException">The specified cast is not valid.</exception>
		virtual bool GetBoolean(int ordinal) override;

		/// <summary>
		/// Gets the value of the specified column as a <see cref="Byte"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The value of the specified column.</returns>
		/// <exception cref="InvalidCastException">The specified cast is not valid.</exception>
		virtual unsigned char GetByte(int ordinal) override;

		/// <summary>
		/// Reads a stream of bytes from the specified column, starting at location indicated
		/// by <paramref name="dataOffset"/> into the into the <paramref name="buffer"/>, starting at the location indicated by <paramref name="bufferOffset"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <param name="dataOffset">The index within the row from which to begin the read operation.</param>
		/// <param name="buffer">The buffer into which to copy the data.</param>
		/// <param name="bufferOffset">The index with the buffer to which the data will be copied.</param>
		/// <param name="length">he maximum number of characters to read.</param>
		/// <returns>The actual number of bytes read.</returns>
		/// <exception cref="InvalidCastException">The specified cast is not valid.</exception>
		virtual long long GetBytes(int ordinal, long long dataOffset, array<unsigned char>^ buffer, int bufferOffset, int length) override;

		/// <summary>
		/// Gets the value of the specified column as a <see cref="Char"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The value of the specified column.</returns>
		/// <exception cref="InvalidCastException">The specified cast is not valid.</exception>
		virtual wchar_t GetChar(int ordinal) override;

		/// <summary>
		/// Reads a stream of chars from the specified column, starting at location indicated
		/// by <paramref name="dataOffset"/> into the into the <paramref name="buffer"/>, starting at the location indicated by <paramref name="bufferOffset"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <param name="dataOffset">The index within the row from which to begin the read operation.</param>
		/// <param name="buffer">The buffer into which to copy the data.</param>
		/// <param name="bufferOffset">The index with the buffer to which the data will be copied.</param>
		/// <param name="length">he maximum number of characters to read.</param>
		/// <returns>The actual number of bytes read.</returns>
		virtual long long GetChars(int ordinal, long long dataOffset, array<wchar_t>^ buffer, int bufferOffset, int length) override;

		/// <summary>
		/// Gets name of the data type of the specified column.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>A string representing the name of the data type.</returns>
		virtual System::String^ GetDataTypeName(int ordinal) override;

		/// <summary>
		/// Gets the value of the specified column as a <see cref="DateTime"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The value of the specified column.</returns>
		/// <exception cref="InvalidCastException">The specified cast is not valid.</exception>
		virtual System::DateTime GetDateTime(int ordinal) override;

		/// <summary>
		/// Gets the value of the specified column as a <see cref="Decimal"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The value of the specified column.</returns>
		/// <exception cref="InvalidCastException">The specified cast is not valid.</exception>
		virtual Decimal GetDecimal(int ordinal) override;

		/// <summary>
		/// Gets the value of the specified column as a <see cref="Double"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The value of the specified column.</returns>
		/// <exception cref="InvalidCastException">The specified cast is not valid.</exception>
		virtual System::Double GetDouble(int ordinal) override;

		/// <summary>
		/// Returns an <see cref="System::Collections::IEnumerator"/> that can be used to iterate through the rows in the data reader.
		/// </summary>
		/// <returns>An <see cref="System::Collections::IEnumerator"/> that can be used to iterate through the rows in the data reader.</returns>
		virtual System::Collections::IEnumerator^ GetEnumerator() override;

		/// <summary>
		/// Gets the data type of the specified column.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The data type of the specified column.</returns>
		virtual Type^ GetFieldType(int ordinal) override;

		/// <summary>
		/// Gets the value of the specified column as a <see cref="Single"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The value of the specified column.</returns>
		/// <exception cref="InvalidCastException">The specified cast is not valid.</exception>
		virtual Single GetFloat(int ordinal) override;

		/// <summary>
		/// Gets the value of the specified column as a <see cref="Guid"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The value of the specified column.</returns>
		/// <exception cref="InvalidCastException">The specified cast is not valid.</exception>
		virtual Guid GetGuid(int ordinal) override;

		/// <summary>
		/// Gets the value of the specified column as a <see cref="Int16"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The value of the specified column.</returns>
		/// <exception cref="InvalidCastException">The specified cast is not valid.</exception>
		virtual short GetInt16(int ordinal) override;

		/// <summary>
		/// Gets the value of the specified column as an <see cref="Int32"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The value of the specified column.</returns>
		/// <exception cref="InvalidCastException">The specified cast is not valid.</exception>
		virtual int GetInt32(int ordinal) override;

		/// <summary>
		/// Gets the value of the specified column as a <see cref="Int64"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The value of the specified column.</returns>
		/// <exception cref="InvalidCastException">The specified cast is not valid.</exception>
		virtual long long GetInt64(int ordinal) override;

		/// <summary>
		/// Gets the name of the column, given the zero-based column ordinal.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The name of the column</returns>
		virtual System::String^ GetName(int ordinal) override;

		/// <summary>
		/// Gets the column ordinal given the name of the column.
		/// </summary>
		/// <param name="name">The name of the column.</param>
		/// <returns>The zero-based column ordinal.</returns>
		/// <exception cref="IndexOutOfRangeException">The name specified is not a valid column name.</exception>
		virtual int GetOrdinal(System::String^ name) override;

		/// <summary>
		/// Returns the provider-specific field <see cref="Type"/> of the specified column.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The <see cref="Type"/> that describes the data type of the specified column.</returns>
		virtual Type^ GetProviderSpecificFieldType(int ordinal) override;

		/// <summary>
		/// Gets the value of the specified column as an instance of <see cref="Object"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The value of the specified column.</returns>
		virtual Object^ GetProviderSpecificValue(int ordinal) override;

		/// <summary>
		/// Gets all provider-specific attribute columns in the collection for the current row.
		/// </summary>
		/// <param name="values">An array of <see cref="Object"/> into which to copy the attribute columns.</param>
		/// <returns>The number of instances of <see cref="Object"/> in the array.</returns>
		virtual int GetProviderSpecificValues(array<Object^>^ values) override;

		/// <summary>
		/// Returns a <see cref="DataTable"/> that describes the column metadata of the <see cref="NuoDbDataReader"/>.
		/// </summary>
		/// <returns>A <see cref="DataTable"/> that describes the column metadata of the <see cref="NuoDbDataReader"/>.</returns>
		virtual DataTable^ GetSchemaTable() override;

		/// <summary>
		/// Gets the value of the specified column as an <see cref="System::String"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The value of the specified column.</returns>
		/// <exception cref="InvalidCastException">The specified cast is not valid.</exception>
		virtual System::String^ GetString(int ordinal) override;

		/// <summary>
		/// Gets the value of the specified column as an <see cref="Object"/>.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns>The value of the specified column.</returns>
		virtual Object^ GetValue(int ordinal) override;

		/// <summary>
		/// Populates an array of objects with the column values of the current row.
		/// </summary>
		/// <param name="values">An array of <see cref="Object"/> into which to copy the attribute columns.</param>
		/// <returns>The number of instances of <see cref="Object"/> in the array.</returns>
		virtual int GetValues(array<Object^>^ values) override;

		/// <summary>
		/// Gets a value that indicates whether the column contains nonexistent or missing values.
		/// </summary>
		/// <param name="ordinal">The zero-based column ordinal.</param>
		/// <returns><see langword="true"/> if the specified column is equivalent to <see cref="DBNull"/>; otherwise <see langword="false"/>.</returns>
		virtual bool IsDBNull(int ordinal) override;

		/// <summary>
		/// Advances the reader to the next result when reading the results of a batch of statements.
		/// </summary>
		/// <returns><see langword="true"/> if there are more result sets; otherwise <see langword="false"/>.</returns>
		virtual bool NextResult() override;

		/// <summary>
		/// Advances the reader to the next record in a result set.
		/// </summary>
		/// <returns><see langword="true"/> if there are more rows; otherwise <see langword="false"/>.</returns>
		virtual bool Read() override;
#pragma endregion
#pragma endregion
	};
}
