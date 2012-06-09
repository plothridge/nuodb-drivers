#
# Copyright (c) 2012, NuoDB, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of NuoDB, Inc. nor the names of its contributors may
#       be used to endorse or promote products derived from this software
#       without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL NUODB, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

module DBI::DBD::NuoDB

  class Database < DBI::BaseDatabase

    def initialize(conn, attr)
      super
      @conn = conn
    end

    def disconnect
      @conn = nil # TODO do we need an explicit disconnect method?
    end

    def ping
	@conn.ping
    end

    def tables
      sql = 'select tablename from system.tables where schema = ?'
      stmt = @conn.createPreparedStatement sql
      stmt.setString 1, @conn.getSchema
      rset = stmt.executeQuery
      tables = []
      while rset.next
        tables << rset.getString(1)
      end
      tables
    end

    def columns(table)
      # http://ruby-dbi.rubyforge.org/rdoc/classes/DBI/BaseDatabase.html#M000244
      # http://ruby-dbi.rubyforge.org/rdoc/classes/DBI/ColumnInfo.html
      # Please note here that the type returned from the system.fields table is different than the JDBC types
      # returned in getMetaData

      sql = 'select field,datatype,precision,scale from system.fields where schema=? and tablename=?'

      stmt = @conn.createPreparedStatement sql
      schema_name, table_name = split_table_name table
      stmt.setString 1, schema_name
      stmt.setString 2, table_name

      rset = stmt.executeQuery
      cols = []
      while rset.next
        name = rset.getString 1
        type = rset.getInteger 2
        case type
        when 2
          dbi_type = DBI::Type::Varchar
        when 3
          dbi_type = DBI::Type::Varchar
        when 5
          dbi_type = DBI::Type::Integer
        when 8
          dbi_type = DBI::Type::Float
        when 9
          dbi_type = DBI::Type::Timestamp
        when 10
          dbi_type = DBI::Type::Timestamp
        when 11
          dbi_type = DBI::Type::Timestamp
        when 15
          dbi_type = DBI::Type::Timestamp
        when 22
          dbi_type = DBI::Type::Boolean
        else
          raise "unknown type #{type} for column #{name}"
        end

        precision = rset.getInteger 3
        scale = rset.getInteger 4
        cols << DBI::ColumnInfo.new({ :name => name,
                                      :dbi_type => dbi_type,
                                      :precision => precision,
                                      :scale => scale })
      end
      cols
    end

    def prepare(sql)
      stmt = @conn.createPreparedStatement sql
      return Statement.new stmt
    end

    def commit
      @conn.commit
    end

    def rollback
      @conn.rollback
    end

    def do(statement, *bindvars)
      stmt = @conn.createPreparedStatement statement
      stmt.execute
    end

    #
    # Set an attribute on the database handle.
    #
    def []=(attr, value)
    end

    private

    def split_table_name(table)
      name_parts = table.split '.'
      case name_parts.length
      when 1
        schema_name = @conn.getSchema
        table_name = name_parts[0]
      when 2
        schema_name = name_parts[0]
        table_name = name_parts[1]
      else
        raise "Invalid table name: #{table}"
      end
      [schema_name, table_name]
    end

  end
end
