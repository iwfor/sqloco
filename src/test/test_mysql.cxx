/*
 * test1.cxx
 *
 * $Id$
 *
 * tabstop=4
 *
 */

/*
 * Copyright (C) 2002 Isaac W. Foraker (isaac@tazthecat.net)
 * All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Author nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE AND DOCUMENTATION IS PROVIDED BY THE AUTHOR AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "clo.h"
#include <sqloco/sqloco.h>
#include <iostream>
#include <stdexcept>
#include <string>

bool test(clo::parser& parser);

int main(int argc, char* argv[])
{
	clo::options options;

	try {
		// Use clo++ to process the command line
		clo::parser parser;
		parser.parse(argc, argv);

		if (test(parser))
		{
			std::cout << "FAILED" << std::endl;
			return -1;
		}
		else
		{
			std::cout << "PASSED" << std::endl;
		}
	} catch(const clo::exception& e) {
		std::cout << e.what() << std::endl;
	} catch(const std::exception& e) {
		 std::cout << "Exception " << e.what() << std::endl;
	} catch(...) {
		std::cout << "Unknown exception" << std::endl;
	}
	return 0;
}

struct entry_s {
	const char* name;
	const char* number;
} entries[] = {
	{"John Smith", "211-631-1343"},
	{"Alex Thomas", "834-411-1241"},
	{"Horrace Jones", "312-621-7251"},
	{"Amber McKinsey", "502-419-0094"},
	{"Thomas Young", "614-603-9800"},
	{"Judy Robinson", "805-545-3978"}
};

const unsigned num_entries = sizeof(entries) / sizeof(entry_s);

bool test(clo::parser& parser)
{
	const clo::options& options = parser.get_options();
	::dbi dbh(sqloco::db_mysql);
	::statement* sth;
	unsigned i;

	if (dbh.open(options.username.c_str(), options.password.c_str()))
	{
		std::cout << "Failed to connect to database: " << dbh.errstr() << std::endl;
		return true;
	}

	if (dbh.execute("CREATE DATABASE _test") < 0)
	{
		std::cout << "Failed to create database: " << dbh.errstr() << std::endl;
		return true;
	}
	if (dbh.execute("USE _test") < 0)
	{
		std::cout << "Failed to use database: " << dbh.errstr() << std::endl;
		return true;
	}
	
	if (dbh.execute(
			"CREATE TABLE phonebook (\n"
			"phid INT PRIMARY KEY AUTO_INCREMENT,\n"
			"name VARCHAR(64) NOT NULL UNIQUE,\n"
			"phonenumber VARCHAR(64) NOT NULL)") < 0)
	{
		std::cout << "Failed to create table: " << dbh.errstr() << std::endl;
		return true;
	}

	sth = dbh.prepare("INSERT INTO phonebook (name, phonenumber)\n"
			"VALUES (?, ?)");
	for (i=0; i<num_entries; ++i)
	{
		sth->addparam(entries[i].name);
		sth->addparam(entries[i].number);
		sth->execute();
	}
	sth->finish();
	
	sth = dbh.prepare("SELECT * FROM phonebook ORDER BY name");
	if (sth->execute() < 0)
	{
		std::cout << "Failed query: " << dbh.errstr() << std::endl;
		return true;
	}
	std::string name, number;
	long phid;
	sth->bind(phid);
	sth->bind(name);
	sth->bind(number);
	while (!sth->fetch())
	{
		std::cout << "phid: " << phid << std::endl;
		std::cout << "name: " << name << std::endl;
		std::cout << "ph #: " << number << std::endl;
	}
	sth->finish();
	
	dbh.execute("DROP DATABASE _test");
	return false;
}
