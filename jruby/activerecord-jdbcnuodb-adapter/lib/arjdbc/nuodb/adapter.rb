module ::ArJdbc
  module NuoDB

    def self.column_selector
      [ /nuodb/i, lambda {  | cfg, col | col.extend( ::ArJdbc::NuoDB::Column ) } ]
    end

    module Column
    end

    def self.arel2_visitors(config)
      {}.tap {|v| %w(nuodb).each {|a| v[a] = ::Arel::Visitors::NuoDB } }
    end

    def modify_types(tp)
      tp[:primary_key] = 'int not null generated always primary key'
      tp[:boolean] = { :name => 'boolean' }
      tp[:integer] = { :name => 'int', :limit => 4 }
      tp[:decimal] = { :name => 'decimal' }
      tp[:string] = { :name => 'string' }
      tp[:timestamp] = { :name => 'datetime' }
      tp[:datetime][:limit] = nil
      tp
    end

    def type_to_sql(type, limit = nil, precision = nil, scale = nil) #:nodoc:
      limit = nil if %w(text binary string).include? type.to_s
      return 'uniqueidentifier' if (type.to_s == 'uniqueidentifier')
      return super unless type.to_s == 'integer'

      if limit.nil? || limit == 4
        'int'
      elsif limit == 2
        'smallint'
      elsif limit == 1
        'smallint'
      else
        'bigint'
      end
    end

    def quote(value, column = nil)
      case value
      when TrueClass, FalseClass
        value.to_s
      else
        super
      end
    end

    def exec_insert(sql, name, binds)
      sql = substitute_binds(sql, binds)
      @connection.execute_insert(sql)
    end

    def primary_keys(table)
      @connection.primary_keys(qualify_table(table))
    end

    def columns(table_name, name=nil)
      @connection.columns_internal(table_name.to_s, name, nuodb_schema)
    end

    private

    def qualify_table(table)
      if (table.include? '.') || @config[:schema].blank? 
        table
      else
        nuodb_schema + "." + table
      end
    end

    def nuodb_schema
      config[:schema] || ''
    end

  end

end

module Arel
  module Visitors
    class NuoDB < Arel::Visitors::ToSql
      private
      def visit_Arel_Nodes_Lock o
        visit o.expr
      end

      def visit_Arel_Nodes_Limit o
        "FETCH FIRST #{visit o.expr} ROWS ONLY"
      end

      def visit_Arel_Nodes_Matches o
        "#{visit o.left} LIKE #{visit o.right}"
      end

      def visit_Arel_Nodes_DoesNotMatch o
        "#{visit o.left} NOT LIKE #{visit o.right}"
      end
    end
  end
end

