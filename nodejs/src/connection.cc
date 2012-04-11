/****************************************************************************
 * Copyright (c) 2012, NuoDB, Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of NuoDB, Inc. nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NUODB, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************/

#include <sstream>

#include "./connection.h"
#include "./result.h"

#include "nuodb/stdint.h"
#include "nuodb/sqlapi/SqlExceptions.h"
#include "nuodb/sqlapi/SqlEnvironment.h"
#include "nuodb/sqlapi/SqlStatement.h"
#include "nuodb/sqlapi/SqlDatabaseMetaData.h"

node_db_nuodb::Connection::Connection()
    : handle(0) {

    this->port = 48004;
    this->quoteName = '"';
}

class ConnectionHandle {
public:
    ConnectionHandle(nuodb::sqlapi::SqlEnvironment * environment, nuodb::sqlapi::SqlConnection * connection) : environment(environment), connection(connection) {}
    nuodb::sqlapi::SqlEnvironment * environment;
    nuodb::sqlapi::SqlConnection * connection;
};

node_db_nuodb::Connection::~Connection() {
    this->close();
}

bool node_db_nuodb::Connection::isAlive(bool ping) throw() {
    if (ping && this->alive) {
    }
    return this->alive;
}

void node_db_nuodb::Connection::open() throw(node_db::Exception&) {
    this->close();

    std::ostringstream conn_str;
    conn_str << this->database << "@" << this->hostname << ":" << this->port;

    try {
        nuodb::sqlapi::SqlOption options[3];
        options[0].option = "database"; options[0].extra = (void*) conn_str.str().c_str();
        options[1].option = "user"; options[1].extra = (void*) this->user.c_str();
        options[2].option = "password"; options[2].extra = (void*) this->password.c_str();

        nuodb::sqlapi::SqlOptionArray optionsArray;
        optionsArray.count = 3;
        optionsArray.array = options;

        using namespace nuodb::sqlapi;
        SqlEnvironment * environment = SqlEnvironment::createSqlEnvironment(&optionsArray);
        SqlConnection * connection = environment->createSqlConnection(&optionsArray);
        handle = reinterpret_cast<uintptr_t>(new ConnectionHandle(environment, connection));

        this->alive = true;
    } catch(nuodb::sqlapi::ErrorCodeException & exception) {
        throw node_db::Exception(exception.what());
    }
}

void node_db_nuodb::Connection::close() {
    if (this->alive) {
        ConnectionHandle * instance = reinterpret_cast<ConnectionHandle*>(handle);
        delete instance->connection;
        delete instance->environment;
        delete instance;
        handle = 0;
    }
    this->alive = false;
}

std::string node_db_nuodb::Connection::escape(const std::string& string) const throw(node_db::Exception&) {
    throw node_db::Exception("This binding does not implement escape()");
}

std::string node_db_nuodb::Connection::version() const {
    using namespace nuodb::sqlapi;
    if (this->alive) {
        ConnectionHandle * instance = reinterpret_cast<ConnectionHandle*>(handle);
        SqlDatabaseMetaData * dmd = instance->connection->getMetaData();
        char const * version = dmd->getDatabaseVersion();
        delete dmd;
        return version;
    }
}

node_db::Result* node_db_nuodb::Connection::query(const std::string& query) const throw(node_db::Exception&) {
    using namespace nuodb::sqlapi;
    ConnectionHandle * instance = reinterpret_cast<ConnectionHandle*>(handle);
    try {
        SqlStatement * statement = instance->connection->createStatement();
        node_db_nuodb::Result * result = new node_db_nuodb::Result(statement->executeQuery(query.c_str()));
        delete statement;
        return result;
    } catch(ErrorCodeException & ex) {
        throw node_db::Exception(ex.what());
    }
}
