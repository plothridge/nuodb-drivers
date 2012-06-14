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
require 'pp'

class TC_Nuodb < Test::Unit::TestCase

  def setup()

    schema = ENV['NUODB_SCHEMA'] || 'test'
    hostname = ENV['NUODB_HOSTNAME'] || 'localhost'
    username = ENV['NUODB_USERNAME'] || 'cloud'
    password = ENV['NUODB_PASSWORD'] || 'user'
    @dbh = DBI.connect("DBI:NuoDB:#{schema}:#{hostname}", username, password)

    @dbh.execute("DROP TABLE IF EXISTS EMPLOYEE")
    @dbh.execute("CREATE TABLE EMPLOYEE (
               FIRST_NAME CHAR(20) NOT NULL,
               LAST_NAME CHAR(20),
	       HIRE DATE,
	       INTERVIEW TIME,
	       START TIMESTAMP,
               AGE INT,
               SEX CHAR(1),
               INCOME DOUBLE )" );
  end

  def teardown()
    @dbh.disconnect if @dbh
  end

  def test_dual()
    row = @dbh.select_one("SELECT 1 FROM DUAL")
    assert_not_nil row
    assert_equal 1, row.length
    assert_equal 1, row[0]
  end

  def test_insert()

    @dbh.execute( "INSERT INTO EMPLOYEE(FIRST_NAME, LAST_NAME, HIRE, INTERVIEW, START, AGE, SEX, INCOME)
          VALUES ('Mac', 'Mohan', '6/1/2012', '10:30:00', '6/6/2012 09:30:00', 20, 'M', 2000)" )

    row = @dbh.select_one("SELECT * FROM EMPLOYEE")

    assert_not_nil row
    assert_equal 8, row.length
    assert_equal 'Mac', row[0]
    assert_equal 'Mohan', row[1]
    assert_equal '6/1/2012', row[2].strftime("%-m/%-d/%Y")
    assert_equal '10:30:00', row[3].strftime("%H:%M:%S")
    assert_equal '6/1/2012', row[4].strftime("%-m/%-d/%Y")
    assert_equal 20, row[5]
    assert_equal 'M', row[6]
    assert_equal 2000.0, row[7]
  end

  def test_insert_params()
    assert(@dbh.convert_types)

    sth = @dbh.prepare( "INSERT INTO EMPLOYEE(FIRST_NAME,
                   LAST_NAME, 
		   HIRE,
		   INTERVIEW,
	           START,
                   AGE, 
 		   SEX, 
		   INCOME)
                   VALUES (?,?, ?,?, ?, ?, ?, ?)" )
    sth.execute('Fred', 'Flintstone', '6/1/2012', '10:00:00', '6/7/2012 10:00:00', 43, 'M', 2300.0)
    sth.execute('Betty', 'Rubble', '6/1/2012', '11:30:00', '6/8/2012 12:00:00', 38, 'F', 1000.0)
    sth.finish

    # this query produces one record
    sth = @dbh.prepare("SELECT * FROM EMPLOYEE WHERE INCOME > ?")
    sth.execute(1000)

    # check first record
    result = sth.fetch
    # I'm sure there is some cool Ruby syntax for this but don't know it
    
    assert_equal result[0], 'Fred'
    assert_equal result[1], 'Flintstone'
    assert_equal result[2].strftime("%-m/%-d/%Y"), '6/1/2012'
    assert_equal result[3].strftime("%H:%M:%S"), '10:00:00'
    assert_equal result[4].strftime("%-m/%-d/%Y %H:%M:%S"), '6/7/2012 10:00:00'
    assert_equal result[5], 43
    assert_equal result[6], 'M'
    assert_equal result[7], 2300.0

    # ensure no more records
    assert_nil sth.fetch

    sth.finish

    begin
      sth.fetch
    rescue DBI::InterfaceError => e
      assert_equal 'Statement was already closed!', e.message
    end

  end

  def test_tables()
    c = @dbh.tables
    assert_not_nil c
    assert_equal true, c.include?('EMPLOYEE')
  end

  def test_columns()
    c = @dbh.columns('employee')
    assert_not_nil c
    assert_equal 8, c.size
    assert_equal( { :name=>'FIRST_NAME',
                    :dbi_type=>DBI::Type::Varchar,
                    :precision=>20, :scale=>0},
                  c[0])
    assert_equal( { :name=>'LAST_NAME',
                    :dbi_type=>DBI::Type::Varchar,
                    :precision=>20, :scale=>0},
                  c[1])
    assert_equal( { :name=>'HIRE',
                    :dbi_type=>DBI::Type::Timestamp,
                    :precision=>10,:scale=>0},
                  c[2])
    assert_equal( { :name=>'INTERVIEW',
                    :dbi_type=>DBI::Type::Timestamp,
                    :precision=>16,:scale=>0},
                  c[3])
    assert_equal( { :name=>'START',
                    :dbi_type=>DBI::Type::Timestamp,
                    :precision=>16,:scale=>0},
                  c[4])
    assert_equal( { :name=>'AGE',
                    :dbi_type=>DBI::Type::Integer,
                    :precision=>9, :scale=>0},
                  c[5])
    assert_equal( { :name=>'SEX',
                    :dbi_type=>DBI::Type::Varchar,
                    :precision=>1, :scale=>0},
                  c[6])
    assert_equal( { :name=>'INCOME',
                    :dbi_type=>DBI::Type::Float,
                    :precision=>15, :scale=>0},
                  c[7])

    c = @dbh.columns 'test.employee'
    assert_not_nil c
    assert_equal 8, c.size

    begin
      @dbh.columns 'a.b.c'
      fail 'columns did not raise'
    rescue RuntimeError => e
      assert_equal 'Invalid table name: a.b.c', e.message
    end
  end

  def test_ping()
    assert_equal true, @dbh.ping
  end

  def test_generated_keys()
    @dbh.execute("DROP TABLE IF EXISTS GENKEYS")
    @dbh.execute("CREATE TABLE GENKEYS (
               A INTEGER,
               B INTEGER GENERATED ALWAYS,
               C INTEGER GENERATED ALWAYS)" );

    stmt = @dbh.execute('INSERT INTO GENKEYS (A) VALUES (?)', 11)
    keys = stmt.handle.generated_keys
    assert_not_nil keys
    assert_equal 1, keys.length
    assert keys[0] > 0
  end

end
