use Test::More tests => 5;
BEGIN { use_ok('DBD::NuoDB') };

use DBI;
my $dbh = DBI->connect("dbi:NuoDB:test", "cloud", "user", {PrintError => 0});
ok(defined $dbh);

my $sth = $dbh->prepare("SELECT 'one' FROM DUAL");
$sth->execute();
my ($value) = $sth->fetchrow_array();
ok($value eq 'one');

my $sth_err = $dbh->prepare("SYNTAX ERROR");
ok(not defined $sth_err);

my $dbh_raiseerror = DBI->connect("dbi:NuoDB:test", "cloud", "user", {PrintError => 0, RaiseError => 1});
eval {
	my $sth_err2 = $dbh_raiseerror->prepare("SYNTAX ERROR");
};
ok($@ ne '');
