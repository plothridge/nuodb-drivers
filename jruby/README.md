Features
========

* NuoDB's JDBC gem - jdbc-nuodb-1.0.gem
* NuoDB's Active Record gem - activerecord-jdbcnuodb-adapter-1.0.gem
* Sample application - sample/sample.rb

Running the Sample
==================

To run the sample do the following:

1. Ensure that your jruby environment is configured correctly.  

> jruby -v
jruby 1.6.5 (ruby-1.8.7-p330) (2011-10-25 9dcd388) (Java HotSpot(TM) 64-Bit Server VM 1.6.0_26) [darwin-x86_64-java]

2. Install the NuoDB gems

> gem install jdbc-nuodb-1.0.gem
> gem install activerecord-jdbcnuodb-adapter-1.0.gem

3. Verify that the gems are installed

> gem list

4. Run the sample:

> jruby sample.rb


Using Rails
===========

To use NuoDB in a JRuby Rails application:

1. Include NuoDB information in the database.yml file as:

development:
  adapter: nuodb
  database: test
  username: cloud
  password: user

2. In the Gemfile, call the nuodb gem with:

gem 'activerecord-jdbcnuodb-adapter'

