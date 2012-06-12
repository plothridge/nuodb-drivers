#! /usr/local/bin/ruby

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

require "test/unit"
require 'ostruct'
require 'nuodb'

CONFIG = OpenStruct.new
CONFIG.database = ENV['NUODB_DATABASE'] || 'test@localhost'
CONFIG.schema = ENV['NUODB_SCHEMA'] || 'test'
CONFIG.username = ENV['NUODB_USERNAME'] || 'cloud'
CONFIG.password = ENV['NUODB_PASSWORD'] || 'user'

class TC_Nuodb < Test::Unit::TestCase

  def setup()
    @database = CONFIG.database
    @schema = CONFIG.schema
    @username = CONFIG.username
    @password = CONFIG.password
  end

  def teardown()
  end

  #def test_version()
  #  con = Nuodb::Connection.createSqlConnection @database, @schema, @username, @password
  #  dmd = con.getMetaData
  #  assert_match /^1\./, dmd.getDatabaseVersion
  #end

  def test_select_from_dual()
    con = Nuodb::Connection.createSqlConnection @database, @schema, @username, @password
    stmt = con.createStatement
    assert_not_nil stmt
    stmt.execute "select 1 from dual"
  end

  def test_get_schema()
    con = Nuodb::Connection.createSqlConnection @database, @schema, @username, @password
    s = con.getSchema
    assert_equal @schema.upcase, s
  end

  def test_ping()
    con = Nuodb::Connection.createSqlConnection @database, @schema, @username, @password
    assert_equal true, con.ping
  end

  def test_auto_commit_flag()
    con = Nuodb::Connection.createSqlConnection @database, @schema, @username, @password
    assert con.hasAutoCommit
    con.setAutoCommit false
    assert !con.hasAutoCommit
    con.setAutoCommit true
    assert con.hasAutoCommit
  end

  def test_statement()
    con = Nuodb::Connection.createSqlConnection @database, @schema, @username, @password
    stmt = con.createStatement
    assert_not_nil stmt
    stmt.execute "drop table test_nuodb if exists"
    stmt.execute <<EOS
create table test_nuodb (
  i integer,
  d double,
  s string,
  b boolean,
  x integer generated always,
  primary key (i))
EOS

    stmt = con.createStatement
    assert_not_nil stmt

    result = stmt.executeQuery "select * from test_nuodb"
    assert_equal(-1, stmt.getUpdateCount)
    assert_nil stmt.getGeneratedKeys
    assert_not_nil result
    assert_equal false, result.next

    meta = result.getMetaData
    assert_not_nil meta
    assert_equal 5, meta.getColumnCount

    assert_equal "I", meta.getColumnName(1)
    assert_equal :SQL_INTEGER, meta.getType(1)

    assert_equal "D", meta.getColumnName(2)
    assert_equal :SQL_DOUBLE, meta.getType(2)

    assert_equal "S", meta.getColumnName(3)
    assert_equal :SQL_STRING, meta.getType(3)

    assert_equal "B", meta.getColumnName(4)
    assert_equal :SQL_BOOLEAN, meta.getType(4)

    assert_equal "X", meta.getColumnName(5)
    assert_equal :SQL_INTEGER, meta.getType(5)

    stmt.execute "insert into test_nuodb(i,d,s,b) values(10,1.1,'one',true)"
    assert_equal 1, stmt.getUpdateCount
    k = stmt.getGeneratedKeys
    assert_not_nil k
    assert_equal true, k.next
    assert k.getInteger(1) > 0
    assert_equal false, k.next

    stmt.execute "update test_nuodb set s='update' where i=999"
    assert_equal(-1, stmt.getUpdateCount)
    assert_nil stmt.getGeneratedKeys

    stmt.execute "update test_nuodb set s='update' where i=10"
    assert_equal 1, stmt.getUpdateCount
    assert_nil stmt.getGeneratedKeys

    r = stmt.executeQuery "select * from test_nuodb"
    assert_not_nil r
    assert_equal true, r.next
    assert_equal 10, r.getInteger(1)
    assert_equal 1.1, r.getDouble(2)
    assert_equal 'update', r.getString(3)
    # TODO: ResultSet::getBoolean SEGV
    #assert_equal true, r.getBoolean(4)

    assert_equal false, r.next

  end

  def test_prepared_statement()
    con = Nuodb::Connection.createSqlConnection @database, @schema, @username, @password
    stmt = con.createStatement
    assert_not_nil stmt
    stmt.execute "drop table test_nuodb if exists"
    stmt.execute <<EOS
create table test_nuodb (
  i integer,
  d double,
  s string,
  b boolean,
  primary key (i))
EOS

    stmt.execute "insert into test_nuodb(i,d,s,b) values(10,1.1,'one',true)"
    stmt.execute "insert into test_nuodb(i,d,s,b) values(20,2.2,'two',false)"

    query = con.createPreparedStatement "select * from test_nuodb where i=?"
    assert_not_nil query

    assert_equal(-1, query.getUpdateCount)

    query.setInteger 1, 10
    r = query.executeQuery
    assert_equal(-1, query.getUpdateCount)
    assert_not_nil r
    assert_equal true, r.next
    assert_equal 10, r.getInteger(1)
    assert_equal 1.1, r.getDouble(2)
    assert_equal 'one', r.getString(3)
    # TODO: ResultSet::getBoolean SEGV
    # assert_equal true, r.getBoolean(4)

    assert_equal false, r.next

    query.setInteger 1, 20
    r = query.executeQuery
    assert_not_nil r
    assert_equal true, r.next
    assert_equal 20, r.getInteger(1)
    assert_equal 2.2, r.getDouble(2)
    assert_equal 'two', r.getString(3)
    # TODO: ResultSet::getBoolean SEGV
    # assert_equal false, r.getBoolean(4)

    assert_equal false, r.next

    update = con.createPreparedStatement 'update test_nuodb set s=? where i=?'
    assert_not_nil update
    update.setString 1, 'change'

    update.setInteger 2, 999
    update.execute
    assert_equal(-1, update.getUpdateCount)

    update.setInteger 2, 10
    update.execute
    assert_equal 1, update.getUpdateCount

    query.setInteger 1, 10
    r = query.executeQuery
    assert_not_nil r
    assert_equal true, r.next
    assert_equal 10, r.getInteger(1)
    assert_equal 'change', r.getString(3)
    assert_equal 1, update.getUpdateCount

  end

end

