use Test::More tests => 5;
use DBI;
use utf8;
binmode STDERR, ":utf8";

my $dbh = DBI->connect("dbi:NuoDB:test", "cloud", "user", {PrintError => 1, PrintWarn => 0, AutoCommit => 1, schema => 'dbi'});

my $utf8_string = 'Това е текст';
my ($utf8_out) = $dbh->selectrow_array("SELECT '$utf8_string' FROM DUAL");
ok($utf8_out eq $utf8_string);
ok(length($utf8_string) == length($utf8_out));

my $umlauts = 'ÄËÏÖÜ';
my $sth = $dbh->prepare("SELECT ? , CHARACTER_LENGTH(? USING CHARACTERS) ,  CHARACTER_LENGTH(? USING OCTETS) FROM DUAL");
$sth->execute($umlauts, $umlauts, $umlauts);
my ($out2, $out2_characters, $out2_octets) = $sth->fetchrow_array();
ok($out2_characters == length($umlauts));
ok($out2_octets == length($umlauts) * 2);
ok($umlauts eq $out2);
