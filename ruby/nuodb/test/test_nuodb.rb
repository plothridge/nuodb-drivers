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

  def test_version()
    env = Nuodb::SqlEnvironment.createSqlEnvironment
    con = env.createSqlConnection @database, @schema, @username, @password
    dmd = con.getMetaData
    assert_match /^1\./, dmd.getDatabaseVersion
  end

  def test_select_from_dual()
    env = Nuodb::SqlEnvironment.createSqlEnvironment
    con = env.createSqlConnection @database, @schema, @username, @password
    stmt = con.createStatement
    assert_not_nil stmt
    stmt.execute "select 1 from dual"
  end

  def test_auto_commit_flag()
    env = Nuodb::SqlEnvironment.createSqlEnvironment
    con = env.createSqlConnection @database, @schema, @username, @password
    assert con.hasAutoCommit
    con.setAutoCommit false
    assert !con.hasAutoCommit
    con.setAutoCommit true
    assert con.hasAutoCommit
  end

  def test_statement()
    env = Nuodb::SqlEnvironment.createSqlEnvironment
    con = env.createSqlConnection @database, @schema, @username, @password
    stmt = con.createStatement
    assert_not_nil stmt
    stmt.execute "drop table test_nuodb if exists"
    stmt.execute <<EOS
create table test_nuodb (
  i integer,
  d double,
  s string,
  primary key (i))
EOS

    query = con.createStatement
    assert_not_nil query

    result = query.executeQuery "select * from test_nuodb"
    assert_not_nil result

    assert !result.next

    assert_equal 3, result.getColumnCount

    meta1 = result.getMetaData 1
    assert_not_nil meta1
    assert_equal "I", meta1.getColumnName
    assert_equal :SQL_INTEGER, meta1.getType

    meta2 = result.getMetaData 2
    assert_not_nil meta2
    assert_equal "D", meta2.getColumnName
    assert_equal :SQL_DOUBLE, meta2.getType

    meta3 = result.getMetaData 3
    assert_not_nil meta3
    assert_equal "S", meta3.getColumnName
    assert_equal :SQL_STRING, meta3.getType

    # TODO SqlConnection.createPreparedStatement
    # TODO SqlStatement.executeQuery

  end

  # TODO SqlConnection.commit
  # TODO SqlConnection.rollback

  # SqlResultSetWrapper
  # TODO bool next();
  # TODO size_t getColumnCount() const;
  # TODO SqlColumnMetaData & getMetaData(size_t column) const;
  # TODO int32_t getInteger(size_t column) const;
  # TODO double getDouble(size_t column) const;
  # TODO char const * getString(size_t column) const;
  # TODO SqlDate const * getDate(size_t column) const;

  # SqlColumnMetaDataWrapper
  # TODO char const * getColumnName() const;
  # TODO SqlType getType() const;

  # SqlPreparedStatementWrapper
  # TODO void setInteger(size_t index, int32_t value);
  # TODO void setDouble(size_t index, double value);
  # TODO void setString(size_t index, char const * value);
  # TODO void execute();
  # TODO SqlResultSet & executeQuery();

end

