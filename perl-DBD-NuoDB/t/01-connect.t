use Test::More tests => 10;
BEGIN { use_ok('DBD::NuoDB') };

use DBI;
my $dbh = DBI->connect("dbi:NuoDB:test", "cloud", "user", {PrintError => 1, RaiseError => 1});
ok(defined $dbh);

my $dbh_no_such_database = DBI->connect('dbi:NuoDB:no_such_database', 'cloud', 'user', {PrintError => 0});

ok($DBI::err == -7);
ok($DBI::errstr eq 'no NuoDB nodes are available for database "no_such_database"');

my $dbh_no_such_user = DBI->connect('dbi:NuoDB:test', 'nuodbi_no_such_user', 'user', {PrintError => 0});
ok($DBI::err == -13);
ok($DBI::errstr eq '"nuodbi_no_such_user" is not a known user for database "test"');

my $dbh_wrong_password = DBI->connect('dbi:NuoDB:test', 'cloud', 'wrong_password', {PrintError => 0});
ok($DBI::err == -13);
ok($DBI::errstr eq 'Authentication failed');

eval {
	my $raise_error = DBI->connect('dbi:NuoDB:no_such_database', 'cloud', 'user', {RaiseError => 1, PrintError => 0});
};
ok($@ eq "DBI connect('no_such_database','cloud',...) failed: no NuoDB nodes are available for database \"no_such_database\" at t/01-connect.t line 22\n");

my $dbh_schema = DBI->connect('dbi:NuoDB:test', 'cloud', 'user', { PrintError => 1, RaiseError => 1 , 'schema' => 'nuodbischema2' } );
$dbh_schema->do("DROP TABLE IF EXISTS t1;");
$dbh_schema->do("DROP TABLE IF EXISTS t2;");
$dbh_schema->do("CREATE TABLE t1 (f1 INTEGER)");
$dbh_schema->do("CREATE TABLE t2 (f1 INTEGER)");
my ($table_count) = $dbh->selectrow_array("SELECT COUNT(*) FROM SYSTEM.TABLES WHERE SCHEMA = 'nuodbischema2'");
ok($table_count == 2);

$dbh_schema->do("DROP SCHEMA nuodbischema2 CASCADE");
