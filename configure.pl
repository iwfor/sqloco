#! /usr/bin/perl
#
# configure.pl (bootstrap SQLoco)
# Use on *nix platforms.
#
# SQLoco - A C++ wrapper for SQL libraries.
# Copyright (C) 2002-2003 Isaac W. Foraker (isaac@tazthecat.net)
# All Rights Reserved
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 
# 3. Neither the name of the Author nor the names of its contributors
#    may be used to endorse or promote products derived from this
#    software without specific prior written permission.
# 
# THIS SOFTWARE AND DOCUMENTATION IS PROVIDED BY THE AUTHOR AND
# CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
# BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
# FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
# AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

#####
# Includes
use strict;
use Getopt::Long;
use Cwd qw(cwd chdir);

#####
# Constants
use constant DATE		=> 'Thu May  10 4:44:54 2002';
use constant ID			=> '$Id$';

#####
# Global Variables
use vars qw{$opt_help $opt_bundle $opt_developer $opt_prefix $opt_incdir
	$opt_incdir $opt_libdir $opt_cxx $opt_disable_shared $opt_with_mysql
	$opt_with_postgresql $opt_without_postgresql $opt_without_mysql};

# The current directory
my $cwd = cwd();

# Possible names for the compiler
my @cxx_guess = qw(g++ c++ CC cl bcc32);

my $mkmf	= "${cwd}/tools/mkmf";
my $cxxflags	= "${cwd}/tools/cxxflags";

my $libname	= "sqloco";
my $install_spec= "doc/install.spec";

my $includes	= "--include '${cwd}/src/inc' ";
my $libraries	= "--slinkwith '${cwd}/src/lib,$libname' --linkwith z ";

my @extra_compile = (
	"${cwd}/src/lib",
	"${cwd}/src/testsupp",
	"${cwd}/src/test"
);

my %dbs;

#####
# Code Start
$|++;

GetOptions(
	'help',
	'developer',
	'prefix=s',
	'incdir=s',
	'libdir=s',
	'with-mysql=s',
	'with-postgresql=s',
	'without-mysql',
	'without-postgresql'
) or usage();
$opt_help && usage();

sub usage {
	print "Usage: $0 [options]\n", <<EOT;
  --developer              Turn on developer mode

  --prefix path            Set the install prefix to path  [/usr/local]
  --incdir path            Set the install inc dir to path [PREFIX/inc]
  --libdir path            Set the install lib dir to path [PREFIX/lib]

By default, configure.pl will search for installed databases.  If search is not
successfull, you should specify the locations of database libraries you use.

  --with-mysql path        Set path to MySQL files
  --with-postgresql path   Set path to Postgresql files

  --without-mysql	       Do not include MySQL support
  --without-postgresql     Do not include Postgresql support
EOT
	exit;
}

$opt_prefix	||= "/usr/local";
$opt_incdir	||= "$opt_prefix/include";
$opt_libdir	||= "$opt_prefix/lib";

my $libcnt = 0;

print "Configuring SQLoco...\n";

#####
# Determine C++ compiler settings
$opt_cxx ||= $ENV{'CXX'};
if (not $opt_cxx) {
	print "Checking C++ compiler... ";
	my $path;
	# search for a compiler
	foreach (@cxx_guess) {
		if ($path = search_path($_)) {
			$opt_cxx = "$path/$_";
			last;
		}
	}
	if ($opt_cxx) {
		print "$opt_cxx\n";
	} else {
		print <<EOT;
Not found.

You must specify your C++ compiler with the --cxx parameter or by setting the
CXX environment variable.
EOT
		exit;
	}
} else {
	if (not -e $opt_cxx) {
		print "ERROR The C++ compiler does not appear to be valid: $opt_cxx\n";
		exit;
	}
	print "Using C++ compiler... $opt_cxx\n";
}
$ENV{'CXX'} = $opt_cxx;		# This will be passed into mkmf

# Check for databases
my $finc;
my $flib;
if (!$opt_without_mysql) {
	($finc, $flib) = find_mysql();
	if ($finc) {
		$libcnt++;
		$ENV{'CXXFLAGS'}.= " -DSQLOCO_ENABLE_MYSQL";
		$dbs{'mysql'} = 1;
		if (-e "$flib/libmysqlclient_r.a") {
			# Use thread safe library if available
			$libraries.= "--linkwith '$flib,mysqlclient_r' ";
		} else {
			$libraries.= "--linkwith '$flib,mysqlclient' ";
		}
		if (($finc ne "system") && ($finc ne "/usr/local/include")) {
			$includes.= "--include '$finc' ";
		}
	}
}
if (!$opt_without_postgresql) {
	($finc, $flib) = find_postgresql();
	if ($finc) {
		$libcnt++;
		$ENV{'CXXFLAGS'}.= " -DSQLOCO_ENABLE_POSTGRESQL";
		$dbs{'postgresql'} = 1;
		$libraries.= "--linkwith '$flib,pq' ";
		if (($finc ne "system") && ($finc ne "/usr/local/include")) {
			$includes.= "--include '$finc' ";
		}
	}
}

my $mkmf_flags  = "--cxxflags '$cxxflags' --mt --quiet ";
if ($opt_developer) {
	print "Developer extensions... enabled\n";
	$mkmf_flags.= "--developer ";
}

if (!$libcnt) {
	print "No databases found.  Cannot proceed.\n";
	exit;
}

print "Generating SQLoco Makefiles ";
generate_toplevel_makefile();
generate_library_makefile();
generate_tests_makefile();

if (!$opt_bundle) {
	print "\n";
	print "Install Prefix:          $opt_prefix\n";
	print "Includes Install Path:   $opt_incdir\n";
	print "Libraries Install Path:  $opt_libdir\n";
	print "\n";

	print <<EOT;
===============================================================================

Configuration complete.  To built, type:

    make

To install, type:

    make install

===============================================================================
EOT
}


sub generate_toplevel_makefile {
	unless (open(SPEC, ">$install_spec")) {
		print STDERR "\n$0: can't open $install_spec: $!\n";
		exit 1;
	}

	print SPEC "libdir=$opt_libdir\n";
	print SPEC "static-lib src/lib sqloco\n";
	print SPEC "includedir=$opt_incdir\n";
	print SPEC "include-dir src/inc/sqloco sqloco\n";
	close SPEC;

	system("$^X $mkmf $mkmf_flags --install $install_spec --wrapper @extra_compile");
	print ".";
}


sub generate_library_makefile {
	if (not chdir("$cwd/src/lib")) {
		print STDERR "\n$0: can't chdir to src: $!\n";
		exit 1;
	}

	system("$^X $mkmf $mkmf_flags $includes --static-lib $libname *.cxx");
	print ".";
	chdir $cwd;
}


sub generate_tests_makefile {
	if (not chdir("$cwd/src/testsupp")) {
		print STDERR "\n$0: can't chdir to src/testsupp: $!\n";
		exit 1;
	}
	print ".";
	system("$^X $mkmf $mkmf_flags --static-lib sqloco_testsupp *.cxx");

	if (not chdir("$cwd/src/test")) {
		print STDERR "\n$0: can't chdir to src/test: $!\n";
		exit 1;
	}

	print ".";
	my $files;
	$files.= " test_mysql.cxx" if ($dbs{'mysql'});
	$files.= " test_postgresql.cxx" if ($dbs{'postgresql'});
	system("$^X $mkmf $mkmf_flags $includes $libraries --linkwith '../testsupp,sqloco_testsupp' --quiet --many-exec $files");
	chdir $cwd;
}

#
# Search for a MySQL install
#
sub find_mysql {
	my $flib;
	my $finc;
	if ($opt_with_mysql) {
		print "Using user setting for MySQL\n";
		$finc = "$opt_with_mysql/include";
		$flib = "$opt_with_mysql/lib";
		return ($finc, $flib);
	}
	print "Checking for MySQL... ";

	chomp ($flib = `mysql_config --libs`);
	chomp ($finc = `mysql_config --cflags`);
	$flib =~ s/.*-L'(.*?)'.*/$1/;
	$finc =~ s/.*-I'(.*?)'.*/$1/;
	if (($flib ne '') && ($finc ne '')) {
		print "found.\n";
		return ($finc, $flib);
	}

	my $path;
	my @paths;
	@paths = qw(
		/usr
	);
	foreach $path (@paths) {
		if (-e "$path/include/mysql") {
			$finc = 'system';
			last;
		}
	}
	foreach $path (@paths) {
		if (-e "$path/lib/mysql/libmysqlclient.a") {
			$flib = "$path/lib/mysql";
			last;
		}
		if (-e "$path/lib/libmysqlclient.a") {
			$flib = "$path/lib";
			last;
		}
	}
	if (($flib ne '') && ($finc ne '')) {
		print "found.\n";
		return ($finc, $flib);
	}

	@paths = qw(
		/usr/local
		/usr/local/mysql
		/usr/mysql
		/opt/mysql
		/export/mysql
		/home/mysql
	);

	foreach $path (@paths) {
		if (-e "$path/include/mysql.h") {
			$finc = "$path/include";
			last;
		}
		if (-e "$path/include/mysql") {
			$finc = "$path/include/mysql";
			last;
		}
	}
	foreach $path (@paths) {
		if (-e "$path/lib/libmysqlclient.a") {
			$flib = "$path/lib";
			last;
		}
	}
	if (($flib ne '') && ($finc ne '')) {
		print "found.\n";
	} else {
		print "not found.\n";
		return (undef, undef);
	}
	return ($finc, $flib);
}


#
# Search for a PostgreSQL install
# Note: May want to use pg_config in future if available.
#
sub find_postgresql {
	my $flib;
	my $finc;
	if ($opt_with_postgresql) {
		print "Using user setting for PostgreSQL\n";
		return "$opt_with_postgresql/include";
	}
	print "Checking for PostgreSQL... ";
	chomp ($flib = `pg_config --libdir`);
	chomp ($finc = `pg_config --includedir`);
	if (($finc ne '') && ($flib ne '')) {
		print "found.\n";
		return ($finc, $flib);
	}

	my $path;
	my @paths;
	@paths = qw(
		/usr
	);
	foreach $path (@paths) {
		if (-e "$path/include/libpq-fe.h") {
			$finc = "system";
			last;
		}
	}
	foreach $path (@paths) {
		if (-e "$path/lib/libpq.a") {
			$flib = "$path/lib";
			last;
		}
	}
	if (($finc ne '') && ($flib ne '')) {
		print "found.\n";
		return ($finc, $flib);
	}

	@paths = qw(
		/usr/local
		/usr/local/pgsql
		/usr/pgsql
		/opt/pgsql
		/export/pgsql
		/home/pgsql
	);

	foreach $path (@paths) {
		if (-e "$path/include/libpq-fe.h") {
			$finc = "$path/include";
			last;
		}
	}
	foreach $path (@paths) {
		if (-e "$path/lib/libpq.a") {
			$flib = "$path/lib";
			last;
		}
	}
	if (($finc ne '') && ($flib ne '')) {
		print "found.\n";
	} else {
		print "not found.\n";
		return (undef, undef);
	}
	return ($finc, $flib);
}


sub search_path {
	my $prog = shift;
	# Determine search paths
	my $path = $ENV{'PATH'} || $ENV{'Path'} || $ENV{'path'};
	my @paths = split /[;| |:]/, $path;

	my $ext = $^O =~ /win/i ? '.exe' : '';

	foreach (@paths) {
		if (-e "$_/$prog$ext") {
			return $_;
		}
	}
	return undef;
}

sub run_command {
	my $cmd = shift;
	my $output;

	# Note, INSTREAM is ignored.
	my $pid = open3(\*INSTREAM, \*OUTSTREAM, \*OUTSTREAM, $cmd);

	if (not $pid) {
		return undef;
	}
	while (<OUTSTREAM>) {
		$output.= $_;
	}
	waitpid($pid, 0);
	return $output;
}
