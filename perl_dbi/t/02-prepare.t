use strict;
use Test::More tests => 10;
BEGIN { use_ok('DBD::NuoDB') };

use DBI;
my $host = defined $ENV{AGENT_PORT} ? "localhost:".$ENV{AGENT_PORT} : "localhost";
my $dbh = DBI->connect('dbi:NuoDB:test@'.$host, "cloud", "user", { PrintError => 0, schema => 'dbi' });

ok(defined $dbh);

my $sth = $dbh->prepare("SELECT 'one', 'two' FROM DUAL UNION ALL SELECT 'three' , 'four' FROM DUAL");
$sth->execute();

my $values = $sth->fetchall_arrayref();
ok($values->[0]->[0] eq 'one');
ok($values->[0]->[1] eq 'two');
ok($values->[1]->[0] eq 'three');
ok($values->[1]->[1] eq 'four');

my $rows = $sth->rows();
ok($rows == 2);

my $sth_err = $dbh->prepare("SYNTAX ERROR");
ok(not defined $sth_err);
ok($dbh->errstr() =~ m{syntax error}i);

my $dbh_raiseerror = DBI->connect('dbi:NuoDB:test@'.$host, "cloud", "user", {PrintError => 0, RaiseError => 1});
eval {
	my $sth_err2 = $dbh_raiseerror->prepare("SYNTAX ERROR");
};
ok($@ =~ m{syntax error}i);

$dbh->do("CREATE TABLE t1 (f1 INTEGER)");
$dbh->do("INSERT INTO t1 VALUES (1),(1)");

#my $sth_update = $dbh->prepare("UPDATE t1 SET f1 = 2");
#my $affected_rows = $sth_update->execute();
#ok($affected_rows == 2);
