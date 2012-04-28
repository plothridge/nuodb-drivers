# -*- encoding: utf-8 -*-

$LOAD_PATH << File.expand_path('../lib', __FILE__)
require 'jdbc/nuodb'
version = Jdbc::NuoDB::VERSION
Gem::Specification.new do |s|
  s.name = %q{jdbc-nuodb}
  s.version = version

  s.authors = ["NuoDB, Inc."]
  s.date = %q{2011-09-24}
  s.description = %q{Install and load this gem to gain access to the NuoDB JDBC driver.}
  s.email = %q{support@nuodb.com}

  s.files = [
    "Rakefile", 
    "README.txt", 
    "LICENSE",
    *Dir["lib/**/*"].to_a
  ]
  s.homepage = %q{http://nuodb.com/}
  s.require_paths = ["lib"]
  s.summary = %q{NuoDB JDBC driver}
end
