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

require 'dbi'
require 'arel/visitors/nuodb'
require 'active_record'
require 'active_record/base'
require 'active_record/connection_adapters/abstract_adapter'
require 'active_record/connection_adapters/nuodb/database_statements'
require 'active_record/connection_adapters/nuodb/version'

module ActiveRecord
  
  class Base
    
    def self.nuodb_connection(config) #:nodoc:
      config = config.symbolize_keys
      # supply configuration defaults
      config.reverse_merge! :host => 'localhost'
      ConnectionAdapters::NuoDBAdapter.new nil, logger, nil, config 
    end
    
  end
  
  module ConnectionAdapters
    
    class NuoDBColumn < Column

      def initialize(name, default, sql_type = nil, null = true, options = {})
        @options = options.symbolize_keys
        super(name, default, sql_type, null)
        @primary = @options[:is_identity] || @options[:is_primary]
      end
      
      class << self
        
        def string_to_binary(value)
          "0x#{value.unpack("H*")[0]}"
        end
        
        def binary_to_string(value)
          value =~ /[^[:xdigit:]]/ ? value : [value].pack('H*')
        end
        
      end
      
      def is_identity?
        @options[:is_identity]
      end
      
      def is_primary?
        @options[:is_primary]
      end
      
      def is_utf8?
        !!(@sql_type =~ /nvarchar|ntext|nchar/i)
      end
      
      def is_integer?
        !!(@sql_type =~ /int/i)
      end
      
      def is_real?
        !!(@sql_type =~ /real/i)
      end
      
      def sql_type_for_statement
        if is_integer? || is_real?
          sql_type.sub(/\((\d+)?\)/,'')
        else
          sql_type
        end
      end
      
      def default_function
        @options[:default_function]
      end
      
      def table_name
        @options[:table_name]
      end
      
      def table_klass
        @table_klass ||= begin
                           table_name.classify.constantize
                         rescue StandardError, NameError, LoadError
                           nil
                         end
        (@table_klass && @table_klass < ActiveRecord::Base) ? @table_klass : nil
      end
      
      private
      
      def extract_limit(sql_type)
        case sql_type
        when /^smallint/i
          2
        when /^int/i
          4
        when /^bigint/i
          8
        when /\(max\)/, /decimal/, /numeric/
          nil
        else
          super
        end
      end
      
      def simplified_type(field_type)
        case field_type
        when /real/i              then :float
        when /money/i             then :decimal
        when /image/i             then :binary
        when /bit/i               then :boolean
        when /uniqueidentifier/i  then :string
        when /datetime/i          then :datetime
        when /varchar\(max\)/     then :text
        when /timestamp/          then :binary
        else super
        end
      end
      
    end #class NuoDBColumn
    
    class NuoDBAdapter < AbstractAdapter

      include Nuodb::DatabaseStatements
      
      def initialize(connection, logger, pool, config)
        super(connection, logger, pool)
        @visitor = Arel::Visitors::NuoDB.new self
        @config = config
        connect
      end
      
      def adapter_name
        'NuoDB'
      end
      
      def supports_migrations?
        true
      end
      
      def supports_primary_key?
        true
      end
      
      def supports_count_distinct?
        true
      end
      
      def supports_ddl_transactions?
        true
      end
      
      def supports_bulk_alter?
        false
      end
      
      def supports_savepoints?
        true
      end
      
      def supports_index_sort_order?
        true
      end
      
      def supports_explain?
        true
      end
      
      def active?
        raw_connection_do 'select 1 from system.tables fetch first 1 rows'
        true
      rescue
        false
      end

      def reconnect!
        disconnect!
        connect
        active?
      end

      def disconnect!
        @connection.disconnect rescue nil # TODO
      end
      
      def reset!
        # TODO -- IMPLEMENT RESET
        # Reset the state of this connection, directing the DBMS to clear
        # transactions and other connection-related server-side state. Usually a
        # database-dependent operation.
      end
      
      def pk_and_sequence_for(table_name)
        idcol = identity_column(table_name)
        idcol ? [idcol.name,nil] : nil
      end

      def primary_key(table_name)
        identity_column(table_name).try(:name)
      end
      
      def version
        self.class::VERSION
      end
      
      def auto_connect
        @@auto_connect.is_a?(FalseClass) ? false : true
      end
      
      def auto_connect_duration
        @@auto_connect_duration ||= 10
      end

      protected
      
      def connect
        database = @config[:database]
        hostname = @config[:host]
        username = @config[:username]
        password = @config[:password]
        @connection = DBI.connect("DBI:NuoDB:#{database}:#{hostname}", username, password)
      end

      #
      # schema
      #

      public

      def table_exists?(table_name)
        tables.include?(table_name.to_s)
      end

      def tables()
        @connection.tables
      end

      def columns(table_name, name = nil)
        @connection.columns(table_name).map {|ci|
          # http://ruby-dbi.rubyforge.org/rdoc/classes/DBI/ColumnInfo.html
          # TODO copy null value from ci
          Column.new(ci.name.downcase, nil, ci.dbi_type, true)
        }
      end

      def native_database_types
        @native_database_types ||= initialize_native_database_types.freeze
      end

      protected
      
      def initialize_native_database_types
        {
          :primary_key  => 'int not null generated always primary key',
          :string       => { :name => 'varchar', :limit => 255  },
          :text         => { :name => 'varchar' },
          :integer      => { :name => 'integer' },
          :float        => { :name => 'float', :limit => 8 },
          :decimal      => { :name => 'decimal' },
          :datetime     => { :name => 'datetime' },
          :timestamp    => { :name => 'datetime' },
          :time         => { :name => 'time' },
          :date         => { :name => 'date' },
          :binary       => { :name => 'binary' },
          :boolean      => { :name => 'boolean'},
          :char         => { :name => 'char' },
          :varchar_max  => { :name => 'varchar(max)' },
          :nchar        => { :name => 'nchar' },
          :nvarchar     => { :name => 'nvarchar', :limit => 255 },
          :nvarchar_max => { :name => 'nvarchar(max)' },
          :ntext        => { :name => 'ntext' },
          :ss_timestamp => { :name => 'timestamp' }
        }
      end

    end #class NuoDBAdapter < AbstractAdapter
    
  end #module ConnectionAdapters
  
end #module ActiveRecord

