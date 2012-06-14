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

  class Statement < DBI::BaseStatement

    def initialize(stmt, sql)
      @stmt = stmt
      @sql = sql
    end

    #
    # Bind a parameter to a statement. DBD Required.
    #
    def bind_param(param, value, attribs = nil)
      case value
      when String
        @stmt.setString param, value
      when Integer
        @stmt.setInteger param, value
      when Fixnum
        @stmt.setInteger param, value
      when Float
        @stmt.setDouble param, value
      when TrueClass
        @stmt.setBoolean param, true
      when FalseClass
        @stmt.setBoolean param, false
      else
        raise "don't know how to bind #{value.class} to parameter #{param}"
      end
    end

    #
    # Execute the statement with known binds. DBD Required.
    #
    def execute
      # TODO this is inefficient, but I want to avoid poking at the statement text
      begin
        @result = @stmt.executeQuery
        @has_result = true
        @column_info = self.column_info
      rescue RuntimeError
        @stmt.execute
        @generated_keys = self.class.build_row @stmt.getGeneratedKeys
        @result = nil
        @has_result = false
        @column_info = nil
      end
      @result
    end

    def finish
      @result = nil;
      @stmt = nil;
    end

    #
    # Fetch the next row in the result set. DBD Required.
    #
    def fetch
      @has_result ? self.class.build_row(@result) : nil
    end

    #
    # Provides result-set column information as an array of hashes. DBD Required.
    #
    def column_info
      return [] if @result.nil?
      retval = []
      meta = @result.getMetaData
      count = meta.getColumnCount
      for i in 1..count
        type = meta.getType i
        case type
        when :SQL_INTEGER
          dbi_type = DBI::Type::Integer
        when :SQL_DOUBLE
          dbi_type = DBI::Type::Float
        when :SQL_STRING
          dbi_type = DBI::Type::Varchar
        when :SQL_DATE
          dbi_type = DBI::Type::Timestamp
        when :SQL_TIME
          dbi_type = DBI::Type::Timestamp
        when :SQL_TIMESTAMP
          dbi_type = DBI::Type::Timestamp
        when :SQL_CHAR
          dbi_type = DBI::Type::Varchar
        when :SQL_BOOLEAN
          dbi_type = DBI::Type::Boolean
        when :SQL_NULL
          dbi_type = DBI::Type::Null
        else
          raise "unknown type #{type} for column #{i}"
        end
                     
        retval << {
          'name' => meta.getColumnName(i),
          'sql_type' => type,
          'dbi_type' => dbi_type,
          'type_name' => meta.getColumnTypeName(i),
          'precision' => meta.getPrecision(i),
          'scale'     => meta.getScale(i),
          'nullable'  => meta.isNullable(i)
          #'indexed'   => meta.isIndexed(i),
          #'primary'   => meta.isPrimary(i),
          #'unique'    => meta.isUnique(i)
        }
      end
      retval
    end

    def generated_keys
      @generated_keys
    end

    def fetch_scroll(direction, offset)
      raise NotImplementedError
    end

    def []=(attr, value)
      raise NotImplementedError
    end

    private

    def self.build_row(result_set)
      return nil if result_set.nil?
      if result_set.next
        meta = result_set.getMetaData
        count = meta.getColumnCount
        retval = []
        for i in 1..count
          type = meta.getType(i)
          case type
          when :SQL_INTEGER
            retval << result_set.getInteger(i)
          when :SQL_DOUBLE
            retval << result_set.getDouble(i)
          when :SQL_STRING
            retval << result_set.getString(i)
          when :SQL_DATE
            retval << result_set.getDate(i)
          when :SQL_TIME
            retval << result_set.getTime(i)
          when :SQL_TIMESTAMP
            retval << result_set.getTimestamp(i)
          when :SQL_CHAR
            retval << result_set.getChar(i)
          else
            raise "unknown type #{type} for column #{i}"
          end
        end
        retval
      else
        return nil
      end
    end

  end
end
