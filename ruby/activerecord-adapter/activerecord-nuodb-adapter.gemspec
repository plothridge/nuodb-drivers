# -*- encoding: utf-8 -*-

$LOAD_PATH << File.expand_path('../../nuodb/lib', __FILE__)

version = "1.0.0"

Gem::Specification.new do |s|
  s.name        = "activerecord-nuodb-adapter"
  s.version     = version
  s.platform    = Gem::Platform::RUBY
  s.authors = ["NuoDB, Inc."]
  s.description = %q{Install this gem to use NuoDB on Rails.}
  s.email = %q{support@nuodb.com}
  s.files = [
    "Rakefile",
    "README",
    "../LICENSE",
    "lib/active_record/connection_adapters/nuodb_adapter.rb"
  ]
  s.homepage = %q{http://nuodb.com/}
  s.require_paths = ["lib"]
  s.summary = %q{NuoDB adapter}
end
