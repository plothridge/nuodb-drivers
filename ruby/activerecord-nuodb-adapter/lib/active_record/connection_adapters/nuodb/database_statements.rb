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

module ActiveRecord
  module ConnectionAdapters
    module Nuodb
      module DatabaseStatements

        def execute(sql, name = nil)
          do_execute sql, name
        end

        def do_execute(sql, name = 'SQL')
          log(sql, name) do
            stmt = @connection.createStatement
            stmt.execute(sql)
          end
        end

        def exec_query(sql, name = 'SQL', binds = [])

          stmt = @connection.createPreparedStatement sql

          param = 1
          binds.each {|bind|
            value = bind[1]
            case value
            when String
              stmt.setString param, value
            when Integer
              stmt.setInteger param, value
            when Fixnum
              stmt.setInteger param, value
            when Float
              stmt.setDouble param, value
            when TrueClass
              stmt.setBoolean param, true
            when FalseClass
              stmt.setBoolean param, false
            when Time
              stmt.setTime param, value
            else
              raise "don't know how to bind #{value.class} to parameter #{param}"
            end
            param += 1
          }

          stmt.execute

          genkeys = stmt.getGeneratedKeys
	  row = genkeys ? next_row(genkeys) : nil
	  @last_inserted_id = row ? row[0] : nil

          result = stmt.getResultSet

          if result
            names = column_names result
            rows = all_rows result
            ActiveRecord::Result.new(names, rows)
          else 
            nil
          end

        end

        def last_inserted_id(result)
          @last_inserted_id
        end

        protected

        def select(sql, name = nil, binds = [])
          exec_query(sql, name, binds).to_a
        end

        protected

        def select_rows(sql, name = nil)
          rows = exec_query(sql, name).rows
        end

        private

        def column_names result
          return [] if result.nil?
          names = []
          meta = result.getMetaData
          count = meta.getColumnCount
          for i in 1..count
            names << meta.getColumnName(i).downcase
          end
          names
        end

        def all_rows(result)
          rows = []
          while (row = next_row(result)) != nil
            rows << row
          end
          rows
        end

        def next_row(result)
          return nil if result.nil?
          if result.next
            meta = result.getMetaData
            count = meta.getColumnCount
            row = []
            for i in 1..count
              type = meta.getType(i)
              case type
              when :SQL_INTEGER
                row << result.getInteger(i)
              when :SQL_DOUBLE
                row << result.getDouble(i)
              when :SQL_STRING
                row << result.getString(i)
              when :SQL_DATE
                row << result.getDate(i)
              when :SQL_TIME
                row << result.getTime(i)
              when :SQL_TIMESTAMP
                row << result.getTimestamp(i)
              when :SQL_CHAR
                row << result.getChar(i)
              when :SQL_BOOLEAN
                row << result.getBoolean(i)
              else
                raise "unknown type #{type} for column #{i}"
              end
            end
            row
          else
            nil
          end
        end

      end
    end
  end
end
