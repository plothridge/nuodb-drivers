#ifndef SRC_RESULT_H_
#define SRC_RESULT_H_

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

#include <cstring>
#include <string>
#include <stdexcept>

#include "./node-db/exception.h"
#include "./node-db/result.h"

#include "nuodb/sqlapi/SqlResultSet.h"
#include "nuodb/sqlapi/SqlColumnMetaData.h"

namespace node_db_nuodb {
class Result : public node_db::Result {
    public:
        class Column : public node_db::Result::Column {
            public:
                explicit Column(nuodb::sqlapi::SqlColumnMetaData * metaData);
                ~Column();
                bool isBinary() const;
                std::string getName() const;
                node_db::Result::Column::type_t getType() const;

            protected:
                std::string name;
                type_t type;
                bool binary;
        };

        explicit Result(nuodb::sqlapi::SqlResultSet * results) throw(node_db::Exception&);
        ~Result();
        void release() throw();
        bool hasNext() const throw();
        char** next() throw(node_db::Exception&);
        unsigned long* columnLengths() throw(node_db::Exception&);
        uint64_t index() const throw(std::out_of_range&);
        Column* column(uint16_t i) const throw(std::out_of_range&);
        uint16_t columnCount() const throw();
        uint64_t affectedCount() const throw();
        uint64_t insertId() const throw(node_db::Exception&);
        uint16_t warningCount() const throw(node_db::Exception&);
        bool isBuffered() const throw();
        bool isEmpty() const throw();

    protected:
        Column** columns;
        uint16_t totalColumns;
        uint64_t rowNumber;
        bool empty;

        char** row(unsigned long* rowColumnLengths) throw(node_db::Exception&);
        void free() throw();

    private:
        nuodb::sqlapi::SqlResultSet * resultSet;
        unsigned long* previousColumnLengths;
        char** previousRow;
        unsigned long* nextColumnLengths;
        char** nextRow;

        void freeRow(char** row) throw();
};
}

#endif  // SRC_RESULT_H_
