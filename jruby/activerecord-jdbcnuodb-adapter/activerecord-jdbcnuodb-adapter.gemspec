# -*- encoding: utf-8 -*-

$LOAD_PATH << File.expand_path('../../jdbc-nuodb/lib', __FILE__)
require 'jdbc/nuodb'
version = Jdbc::NuoDB::VERSION

Gem::Specification.new do |s|
  s.name        = "activerecord-jdbcnuodb-adapter"
  s.version     = version
  s.platform    = Gem::Platform::RUBY
  s.authors = ["NuoDB, Inc."]
  s.description = %q{Install this gem to use NuoDB with JRuby on Rails.}
  s.email = %q{support@nuodb.com}
  s.files = [
    "Rakefile",
    "README.txt",
    "LICENSE",
    "lib/active_record/connection_adapters/nuodb_adapter.rb",
    "lib/arjdbc/discover.rb",
    "lib/arjdbc/nuodb.rb",
    "lib/arjdbc/nuodb/adapter.rb",
    "lib/arjdbc/nuodb/connection_methods.rb"
  ]
  s.homepage = %q{http://nuodb.com/}
  s.require_paths = ["lib"]
  s.summary = %q{NuoDB JDBC adapter for JRuby on Rails}

  s.add_dependency 'activerecord-jdbc-adapter', "~> 1.2.0"
  s.add_dependency 'jdbc-nuodb', "~> #{version}"
end
