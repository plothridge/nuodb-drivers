module Jdbc
  module NuoDB
    VERSION = "1.0"
  end
end
if RUBY_PLATFORM =~ /java/
  require "nuodbjdbc.jar"
elsif $VERBOSE
  warn "jdbc-nuodb is only for use with JRuby"
end
