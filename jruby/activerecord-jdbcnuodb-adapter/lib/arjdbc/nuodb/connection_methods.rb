class ActiveRecord::Base
  class << self
    def nuodb_connection( config )
      config[:schema] ||= config[:database]
      config[:url] ||= "jdbc:com.nuodb://#{config[:host]}/#{config[:database]}?schema=#{config[:schema]}"
      config[:driver] ||= "com.nuodb.jdbc.Driver"
      config[:adapter_spec] ||= ::ArJdbc::NuoDB
      config[:connection_alive_sql] ||= "select 1 from system.tables fetch first 1 rows"
      jdbc_connection(config)
    end
  end
end
