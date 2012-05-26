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
    @dbh = DBI.connect("DBI:NuoDB:test:localhost", "dba", "goalie")
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
    @dbh.do("DROP TABLE IF EXISTS EMPLOYEE")

    @dbh.do("CREATE TABLE EMPLOYEE (
               FIRST_NAME CHAR(20) NOT NULL,
               LAST_NAME CHAR(20),
               AGE INT,  
               SEX CHAR(1),
               INCOME DOUBLE )" );

    @dbh.do( "INSERT INTO EMPLOYEE(FIRST_NAME, LAST_NAME, AGE, SEX, INCOME)
          VALUES ('Mac', 'Mohan', 20, 'M', 2000)" )

    row = @dbh.select_one("SELECT * FROM EMPLOYEE")

    assert_not_nil row
    assert_equal 5, row.length
    assert_equal 'Mac', row[0]
    assert_equal 'Mohan', row[1]
    assert_equal 20, row[2]
    assert_equal 'M', row[3]
    assert_equal 2000.0, row[4]
  end

  def test_insert_params()
    @dbh.do("DROP TABLE IF EXISTS EMPLOYEE")

    @dbh.do("CREATE TABLE EMPLOYEE (
               FIRST_NAME CHAR(20) NOT NULL,
               LAST_NAME CHAR(20),
               AGE INT,  
               SEX CHAR(1),
               INCOME DOUBLE )" );

    sth = @dbh.prepare( "INSERT INTO EMPLOYEE(FIRST_NAME,
                   LAST_NAME, 
                   AGE, 
 		   SEX, 
		   INCOME)
                   VALUES (?, ?, ?, ?, ?)" )
    sth.execute('John', 'Poul', 25, 'M', 2300.0)
    sth.execute('Zara', 'Ali', 17, 'F', 1000.0)
    sth.finish

    # TODO finish this test

    puts "Employees over 1000"
    sth = @dbh.prepare("SELECT * FROM EMPLOYEE WHERE INCOME > ?")
    sth.execute(1000)
    sth.fetch do |row|
      pp row
    end
    sth.finish
  end

end
