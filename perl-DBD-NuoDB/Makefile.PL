use ExtUtils::MakeMaker;
use DBI::DBD;

my $nuodb_includes = '/home/philips/git/nimbusdb/Remote';

WriteMakefile(
	dbd_edit_mm_attribs({
		'NAME'		=> 'DBD::NuoDB',
		'ABSTRACT'	=> 'Perl DBD DBI driver for NuoDB',
		'AUTHOR'	=> 'Philip Stoev <pstoev@nuodb.com>',
		'VERSION_FROM'	=> 'lib/DBD/NuoDB.pm',
		'C'		=> ['dbdimp.cpp', 'NuoDB.c'],
		'XS'		=> {'NuoDB.xs' => 'NuoDB.c'},
		'OBJECT'	=> 'dbdimp.o NuoDB.o',
		'dist'		=> {
				'SUFFIX'   => '.gz',
				'COMPRESS' => 'gzip -9f'
		},
		'realclean'	=> { FILES => '*.xsi' },
		'CONFIGURE'	=> sub {
			my $dbi_arch_dir = dbd_dbi_arch_dir();
			if (exists($opts{INC})) {
				return {INC => "-I$nuodb_includes -I$dbi_arch_dir $opts{INC}"};
			} else {
				return {INC => "-I$nuodb_includes -I$dbi_arch_dir"};
			}
		},
		LD		=> 'g++',
		CC		=> 'g++'
	}
	)
);

package MY;

sub postamble { return main::dbd_postamble(@_); }

#sub libscan {
#	my ($self, $path) = @_;
#	($path =~ m/\~$/) ? undef : $path;
#}
