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

    def initialize(stmt)
      @stmt = stmt
    end

    #
    # Bind a parameter to a statement. DBD Required.
    #
    def bind_param(param, value, attribs)
      # TODO @stmt.setInteger
      # TODO @stmt.setDouble
      # TODO @stmt.setString
    end

    #
    # Execute the statement with known binds. DBD Required.
    #
    def execute
      @result = @stmt.executeQuery
      @column_info = self.column_info
      @result
    end

    #
    # Close the statement and any result cursors. DBD Required.
    #
    def finish
      @stmt = nil;
    end

    #
    # Fetch the next row in the result set. DBD Required.
    #
    def fetch
      if @result.next
        count = @result.getColumnCount

        retval = []
        for i in 1..count
          meta = @result.getMetaData(i)
          type = meta.getType
          case
          when type == :SQL_INTEGER
            retval << @result.getInteger(i)
          else
            raise "unknown type #{type} for column #{i}"
          end
        end
        retval
      else
        return nil
      end
    end

    #
    # Provides result-set column information as an array of hashes. DBD Required.
    #
    def column_info
      retval = []
      count = @result.getColumnCount
      for i in 1..count
        meta = @result.getMetaData i
        retval << {
          'name' => 'DUMMY', # meta.getColumnName,
          'sql_type'  => meta.getType
          # TODO 'type_name' => '???',
          # TODO 'precision' => '???',
          # TODO 'scale'     => '???',
          # TODO 'nullable'  => '???',
          # TODO 'indexed'   => '???',
          # TODO 'primary'   => '???',
          # TODO 'unique'    => '???'
        }
      end
      retval
    end

    #
    # Optional
    #
    def fetch_scroll(direction, offset)
      raise NotImplementedError
    end

    #
    # Optional
    #
    def []=(attr, value)
      raise NotImplementedError
    end

  end
end
