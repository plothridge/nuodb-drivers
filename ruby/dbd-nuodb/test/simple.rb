require 'dbi'
require 'pp'

begin
  dbh = DBI.connect("DBI:NuoDB:test:localhost", "dba", "goalie")

  # get server version string and display it
  version = dbh.select_one("SELECT 1 FROM DUAL")
  puts "Server version: " + version[0]

  dbh.do("DROP TABLE IF EXISTS EMPLOYEE")
  dbh.do("CREATE TABLE EMPLOYEE (
     FIRST_NAME  CHAR(20) NOT NULL,
     LAST_NAME  CHAR(20),
     AGE INT,  
     SEX CHAR(1),
     INCOME FLOAT )" );

  dbh.do( "INSERT INTO EMPLOYEE(FIRST_NAME,
                   LAST_NAME, 
                   AGE, 
		   SEX, 
		   INCOME)
          VALUES ('Mac', 'Mohan', 20, 'M', 2000)" )
  puts "Record has been created"
  dbh.commit

  sth = dbh.prepare( "INSERT INTO EMPLOYEE(FIRST_NAME,
                   LAST_NAME, 
                   AGE, 
 		   SEX, 
		   INCOME)
                   VALUES (?, ?, ?, ?, ?)" )
  sth.execute('John', 'Poul', 25, 'M', 2300)
  sth.execute('Zara', 'Ali', 17, 'F', 1000)
  sth.finish
  dbh.commit

  puts "Employees over 1000"
  sth = dbh.prepare("SELECT * FROM EMPLOYEE WHERE INCOME > ?")
  sth.execute(1000)
  sth.fetch do |row|
    pp row
  end
  sth.finish

rescue DBI::DatabaseError => e
  puts "An error occurred"
  puts "Error code:    #{e.err}"
  puts "Error message: #{e.errstr}"
ensure
  dbh.disconnect if dbh
end

