use strict;
use Test::More tests => 6;
BEGIN { use_ok('DBD::NuoDB') };

use DBI;
my $host = defined $ENV{AGENT_PORT} ? "localhost:".$ENV{AGENT_PORT} : "localhost";
my $dbh = DBI->connect('dbi:NuoDB:test@'.$host, "cloud", "user", {PrintError => 0});

ok(defined $dbh);

my $sth = $dbh->prepare("SELECT 'one' FROM DUAL");
$sth->execute();
my ($value) = $sth->fetchrow_array();
ok($value eq 'one');

my $sth_err = $dbh->prepare("SYNTAX ERROR");
ok(not defined $sth_err);
ok($dbh->errstr() =~ m{syntax error}i);

my $dbh_raiseerror = DBI->connect('dbi:NuoDB:test@'.$host, "cloud", "user", {PrintError => 0, RaiseError => 1});
eval {
	my $sth_err2 = $dbh_raiseerror->prepare("SYNTAX ERROR");
};
ok($@ =~ m{syntax error}i);
