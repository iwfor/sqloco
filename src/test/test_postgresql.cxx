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

#include "../testsupp/clo.h"
#include <sqloco/sqloco.h>
#include <iostream>
#include <stdexcept>
#include <string>

bool test(clo::parser& parser);
bool test1(sqloco::dbi& dbh);

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
	sqloco::dbi dbh(sqloco::db_postgresql);

	// Open the database connection
	if (dbh.open(options.username.c_str(), options.password.c_str()))
	{
		std::cout << "Failed to connect to database: " << dbh.errstr() << std::endl;
		return true;
	}

	// Create a test database
	if (dbh.execute("CREATE DATABASE sqloco_test") < 0)
	{
		std::cout << "Failed to create database: " << dbh.errstr() << std::endl;
		return true;
	}

	bool rval = test1(dbh);

	// Clean up the database
	dbh.execute("DROP DATABASE sqloco_test");
	return rval;
}

bool test1(sqloco::dbi& dbh)
{
	sqloco::statement* sth;
	unsigned i;

	// Switch to the test database
	if (dbh.execute("USE sqloco_test") < 0)
	{
		std::cout << "Failed to use database: " << dbh.errstr() << std::endl;
		return true;
	}

	// Create the first test table
	if (dbh.execute(
			"CREATE TABLE phonebook (\n"
			"phid INT PRIMARY KEY IDENTITY,\n"
			"name VARCHAR(64) NOT NULL UNIQUE,\n"
			"phonenumber VARCHAR(64) NOT NULL)") < 0)
	{
		std::cout << "Failed to create table: " << dbh.errstr() << std::endl;
		return true;
	}

	// Populate the first test table.
	sth = dbh.prepare("INSERT INTO phonebook (name, phonenumber)\n"
			"VALUES (?, ?)");
	for (i=0; i<num_entries; ++i)
	{
		sth->addparam(entries[i].name);
		sth->addparam(entries[i].number);
		sth->execute();
	}
	delete sth;

	// Retrieve entries for first table
	sth = dbh.prepare("SELECT * FROM phonebook ORDER BY phid");
	if (sth->execute() < 0)
	{
		std::cout << "Failed query: " << dbh.errstr() << std::endl;
		return true;
	}
	std::string name, number;
	long phid, row = 1;
	sth->bind(phid);
	sth->bind(name);
	sth->bind(number);
	while (!sth->fetch())
	{
		if (phid != row)
		{
			std::cout << row << " phid " << phid << " != " << row << std::endl;
			return true;
		}
		if (name != entries[row-1].name)
		{
			std::cout << row << " name " << name << " != " << entries[row-1].name << std::endl;
			return true;
		}
		if (number != entries[row-1].number)
		{
			std::cout << row << " number " << number << " != " << entries[row-1].number << std::endl;
			return true;
		}
		++row;
	}

	// Try the same query, but fetched into a hash
	if (sth->execute() < 0)
	{
		std::cout << "Failed query: " << dbh.errstr() << std::endl;
		return true;
	}
	sqloco::Hash hash;
	row = 1;
	while (!sth->fetchhash(hash))
	{
		if (std::atoi(hash["phid"].c_str()) != row)
		{
			std::cout << row << " phid " << hash["phid"] << " != " << row << std::endl;
			return true;
		}
		if (hash["name"] != entries[row-1].name)
		{
			std::cout << row << " name " << hash["name"] << " != " << entries[row-1].name << std::endl;
			return true;
		}
		if (hash["phonenumber"] != entries[row-1].number)
		{
			std::cout << row << " number " << hash["phonenumber"] << " != " << entries[row-1].number << std::endl;
			return true;
		}
		++row;
	}
	delete sth;

	// Create the second test table
	dbh.execute(
			"CREATE TABLE misctest (\n"
			"somefield varchar(8))"
			);
	dbh.execute("INSERT INTO misctest VALUES ('hello')");
	dbh.execute("INSERT INTO misctest VALUES (null)");
	dbh.execute("INSERT INTO misctest VALUES ('goodbye')");

	// Read the second test table, making sure NULL detection works
	sth = dbh.prepare("SELECT * FROM misctest\n");
	sth->execute();
	sth->bind(name);
	sth->fetch();
	if (sth->isnull(0))
		return true;
	sth->fetch();
	if (!sth->isnull(0))
		return true;
	sth->fetch();
	if (sth->isnull(0))
		return true;
	delete sth;

	return false;
}

