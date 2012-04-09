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
	ref class NuoDbException;
	ref class NuoDbTransaction;

	/// <summary>
	/// Represents a connection to a NuoDb database.  This class cannot be inherited.
	/// </summary>
	/// <remarks>
	/// <see cref="ConnectionString"/>s are in the format:  <c>Database=W;Schema=X;User=Y;Password=Z</c>.
	/// </remarks>
	public ref class NuoDbConnection sealed : public DbConnection, IDisposable
	{
#pragma region Instance Fields
	private:
		SqlConnectionWrapper* m_connection;
		bool m_inTransaction;
		System::String^ m_connectionString;
		System::String^ m_database;
		System::String^ m_schema;
		System::String^ m_username;
		System::String^ m_password;
		bool m_disposed;
#pragma endregion

#pragma region Construction / Destruction
	public:
		/// <summary>
		/// Initializes a new instance of <c>NuoDbConnection</c>.
		/// </summary>
		NuoDbConnection();

		/// <summary>
		/// Initializes a new instance of <c>NuoDbConnection</c>.
		/// </summary>
		/// <param name="connectionString">The connection string used to connect to the NuoDb database.</param>
		/// <exception cref="ArgumentNullException">The <paramref name="connectionString"/> was <see langword="null"/> or <see cref="String::Empty"/>.</exception>
		NuoDbConnection(System::String^ connectionString);

		/// Destroys this instance of <c>NuoDbConnection</c>.
		~NuoDbConnection();

	protected:
		/// Disposes of this instance of <c>NuoDbConnection</c>.
		!NuoDbConnection();
#pragma endregion

#pragma region Properties
	internal:
		property bool IsOpen
		{
			bool get() { return m_connection != NULL; }
		}

		property bool InTransaction
		{
			bool get() { return m_inTransaction; }
		}

		property SqlConnectionWrapper* InternalConnection
		{
			SqlConnectionWrapper* get() { return m_connection; }
		}
#pragma endregion

#pragma region Methods
	public:
		/// <summary>
		/// Enlists in the specified transaction as a distributed transaction.
		/// </summary>
		/// <param name="transaction">A reference to an existing <see cref="System::EnterpriseServices::ITransaction"/> in which to enlist.</param>
		void EnlistDistributedTransaction(System::EnterpriseServices::ITransaction^ transaction);

		/// <summary>
		/// Creates and returns a <see cref="NuoDbCommand"/> object associated with the current connection.
		/// </summary>
		/// <returns>A <see cref="NuoDbCommand"/>.</returns>
		NuoDbCommand^ CreateCommand();

	internal:
		void CommitTransaction();
		void RollbackTransaction();

	private:
		void ParseConnectionString();
		void OpenConnection();
#pragma endregion

#pragma region DbConnection Overrides
#pragma region Properties
	public:
		/// Gets or sets the string used to open the connection.
		virtual property System::String^ ConnectionString
		{
			System::String^ get() override { return m_connectionString; }
			void set(System::String^ value) override;
		}

		/// <summary>
		/// Gets the name of the current database after a connection is opened, or
		/// the database name specified in the connection string before the
		/// connection is opened.
		/// </summary>
		virtual property System::String^ Database
		{
			System::String^ get() override { return m_schema; }
		}

		/// Gets the name of the database server to which to connect.
		virtual property System::String^ DataSource
		{
			System::String^ get() override { return m_database; }
		}

		/// Gets a string that represents the version of the server to which this is connected.
		virtual property System::String^ ServerVersion
		{
			System::String^ get() override;
		}

		/// Gets the state of the connection.
		virtual property ConnectionState State
		{
			ConnectionState get() override;
		}
#pragma endregion

#pragma region Methods
	public:
		/// <summary>
		/// Starts a database transaction.
		/// </summary>
		/// <returns>A <see cref="NuoDbTransaction"/> representing the new transaction.</returns>
		virtual NuoDbTransaction^ BeginTransaction() new;

		/// <summary>
		/// Starts a database transaction with the specified isolation level.
		/// </summary>
		/// <param name="isolationLevel">Specifies the isolation level for the transaction.</param>
		/// <returns>A <see cref="NuoDbTransaction"/> representing the new transaction.</returns>
		virtual NuoDbTransaction^ BeginTransaction(IsolationLevel isolationLevel) new;

		/// <summary>
		/// Starts a database transaction with the specified isolation level.
		/// </summary>
		/// <param name="isolationLevel">Specifies the isolation level for the transaction.</param>
		/// <returns>An object representing the new transaction.</returns>
		/// <exception cref="NuoDbException">Something went wrong.</exception>
		virtual DbTransaction^ BeginDbTransaction(IsolationLevel isolationLevel) override;

		/// <summary>
		/// Changes the current database for an open connection.
		/// </summary>
		/// <param name="databaseName">Specifies the name of the database for the connection to use.</param>
		/// <exception cref="NuoDbException">Something went wrong.</exception>
		/// <exception cref="ArgumentNullException">The <paramref name="databaseName"/> was <see langword="null"/> or <see cref="String::Empty"/>.</exception>
		virtual void ChangeDatabase(System::String^ databaseName) override;
		
		/// <summary>
		/// Closes the connection to the database. This is the preferred method of closing any open connection.
		/// </summary>
		/// <exception cref="NuoDbException">Something went wrong.</exception>
		virtual void Close() override;

		/// <summary>
		/// Enlists in the specified transaction.
		/// </summary>
		/// <param name="transaction">A reference to an existing <see cref="System::Transactions::Transaction"/> in which to enlist.</param>
		virtual void EnlistTransaction(System::Transactions::Transaction^ transaction) override;

		/// <summary>
		/// Opens a database connection with the settings specified by the <see cref="ConnectionString"/>.
		/// </summary>
		/// <exception cref="NuoDbException">Something went wrong.</exception>
		virtual void Open() override;

	protected:
		/// <summary>
		/// Creates and returns a <see cref="DbCommand"/> object associated with the current connection.
		/// </summary>
		/// <returns>A <see cref="DbCommand"/>.</returns>
		virtual DbCommand^ CreateDbCommand() override;
#pragma endregion
#pragma endregion
	};
}
