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
my $cwd = cwd();
my %clo;

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

if (not defined $ENV{'CXX'}) {
	print STDERR "*** your CXX environment variable is not set. SQLoco needs this  ***\n";
	print STDERR "*** this variable to find your C++ compiler. Please set it to    ***\n";
	print STDERR "*** the path to your compiler and re-run configure.pl. Thanks.   ***\n";
	exit 1;
}

GetOptions(
	\%clo,
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
$clo{'help'} && usage();

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

$clo{'prefix'}	||= "/usr/local";
$clo{'incdir'}	||= "$clo{'prefix'}/include";
$clo{'libdir'}	||= "$clo{'prefix'}/lib";

my $libcnt = 0;

# Verify CXX
#if (not -e $ENV{'CXX'}) {
#	print "The specified compiler, $ENV{'CXX'}, does not appear to exist.\n";
#	exit;
#}
print "C++ Compiler... $ENV{'CXX'}\n";

# Check for databases
my $finc;
my $flib;
if (!$clo{'without-mysql'}) {
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
if (!$clo{'without-postgresql'}) {
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
if ($clo{'developer'}) {
	$mkmf_flags .= "--developer ";
}

if (!$libcnt) {
	print "No databases found.  Cannot proceed.\n";
	exit;
}

print "Generating SQLoco Makefiles ";
generate_toplevel_makefile();
generate_library_makefile();
generate_tests_makefile();

print "\n";
print "+-------------------------------------------------------------+\n";
print "| Okay, looks like you are ready to go.  To build, type:      |\n";
print "|                                                             |\n";
print "|       make                                                  |\n";
print "|                                                             |\n";
print "| To install, type:                                           |\n";
print "|                                                             |\n";
print "|       make install                                          |\n";
print "|                                                             |\n";
print "| While you wait, why not drop a note to isaac\@tazthecat.net? |\n";
print "+-------------------------------------------------------------+\n";


sub generate_toplevel_makefile {
	unless (open(SPEC, ">$install_spec")) {
		print STDERR "\n$0: can't open $install_spec: $!\n";
		exit 1;
	}

	print SPEC "libdir=$clo{'libdir'}\n";
	print SPEC "static-lib src/lib sqloco\n";
	print SPEC "includedir=$clo{'incdir'}\n";
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
	if ($clo{'with-mysql'}) {
		print "Using user setting for MySQL\n";
		$finc = "$clo{'with-mysql'}/include";
		$flib = "$clo{'with-mysql'}/lib";
		return ($finc, $flib);
	}
	print "Checking for MySQL... ";

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
	if (defined($finc) && defined($flib)) {
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
	if (defined($finc) && defined($flib)) {
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
	if ($clo{'with-postgresql'}) {
		print "Using user setting for PostgreSQL\n";
		return "$clo{'with-postgresql'}/include";
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
