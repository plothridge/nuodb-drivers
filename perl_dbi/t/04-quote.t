use Test::More tests => 1;
use DBI;
my $dbh = DBI->connect("dbi:NuoDB:test", "cloud", "user", {PrintError => 1, PrintWarn => 0, AutoCommit => 1, schema => 'dbi'});

my ($out) = $dbh->selectrow_array("SELECT ".$dbh->quote("Don't")." FROM DUAL");
ok($out eq "Don't");
