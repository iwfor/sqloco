/*
 * dbi.cxx
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

#include <sqloco/dbi.h>
#include <sqloco/statement.h>
#include <sqloco/except.h>
#include "dbi_impl.h"
#include "vars.h"
#include <iostream>

// Include database specific headers as needed.
#ifdef SQLOCO_ENABLE_MYSQL
#include "dbi_mysql.h"
#endif
#ifdef SQLOCO_ENABLE_POSTGRESQL
#include "dbi_postgresql.h"
#endif
#ifdef SQLOCO_ENABLE_MSSQL
#include "dbi_mssql.h"
#endif
#ifdef SQLOCO_ENABLE_ORACLE
#include "dbi_oracle.h"
#endif
#ifdef SQLOCO_ENABLE_SYBASE
#include "dbi_sybase.h"
#endif

namespace sqloco {

static const char* libname = sqloco_libname;
static const char* author = sqloco_author;
static const char* license = sqloco_license;
	
/**
 * Construct a DBI instance for the specified database. 
 *
 */
dbi::dbi(databases db)
{
	switch (db) {
	case db_mysql:
#ifdef SQLOCO_ENABLE_MYSQL
		imp = new dbi_mysql;
#else
		throw dbiexception("MySQL support is not available");
#endif
		break;
	case db_postgresql:
#ifdef SQLOCO_ENABLE_POSTGRESQL
		imp = new dbi_postgresql;
#else
		throw dbiexception("PostgreSQL support is not available");
#endif
		break;
	case db_mssql:
#ifdef SQLOCO_ENABLE_MSSQL
		imp = new dbi_mssql;
#else
		throw dbiexception("MS SQL support is not available");
#endif
		break;
	case db_oracle:
#ifdef SQLOCO_ENABLE_ORACLE
		imp = new dbi_oracle;
#else
		throw dbiexception("Oracle support is not available");
#endif
		break;
	case db_sybase:
#ifdef SQLOCO_ENABLE_SYBASE
		imp = new dbi_sybase;
#else
		throw dbiexception("Sybase support is not available");
#endif
		break;
	default:
		throw dbiexception("Unsupported database type");
	}
}


/**
 * Release resources used by this DBI instance
 */
dbi::~dbi()
{
	delete imp;
}


/**
 * Open a connection to the specified database.
 */
bool dbi::open(const char* username, const char* password, const char* db,
		const char* hostname, unsigned int port)
{
	return imp->open(username, password, db, hostname, port);
}


/**
 * 
 */
void dbi::close()
{
	imp->close();
}


/**
 * Check if a connection is open to the server.
 *
 * @param	none
 * @return	true if connected;
 * @return	false if not connected.
 */
dbi::operator bool() const
{
	return imp->isconnected();
}


/**
 * Check that a connection is not open to the server.
 *
 * @param	none
 * @return	true if not connected;
 * @return	false if connected.
 */
bool dbi::operator !() const
{
	return !imp->isconnected();
}


/**
 * Check for an error condition.
 * 
 * @param	none
 * @return	true if error;
 * @return	false if no error.
 */
bool dbi::error()
{
	return imp->error();
}


/**
 * Get the current error string.
 *
 * @param	none
 * @return	pointer to string containing error message;
 */
const char* dbi::errstr()
{
	return imp->errstr();
}


/**
 * Prepare a statement handle for execution.
 *
 * @param	SQL Statement string.
 * @return	Pointer to newly created statement handle.
 */
statement* dbi::prepare(const char* statement)
{
	return imp->prepare(statement);
}


/**
 * Prepare a statement handle for execution.
 *
 * @param	SQL Statement string.
 * @return	Pointer to newly created statement handle.
 */
statement* dbi::prepare(const std::string& statement)
{
	return imp->prepare(statement.c_str());
}


/**
 * Immediately execute the given statement without generating a new
 * statement handle.
 *
 * @param	statement	Statement to execute.
 * @return	Number of rows affected;
 * @return	-1 on error.
 */
long dbi::execute(const char* st)
{
	statement* sth = prepare(st);
	long rval = sth->execute();
	sth->finish();
	delete sth;
	return rval;
}

/**
 * Immediately execute the given statement, retrieving a single value
 *
 * @param	statement	Statement to execute.
 * @param	value		Destination variable for fetched value.
 * @return	Number of rows affected;
 * @return	-1 on error.
 */
long dbi::executequery(const char* st, std::string& value)
{
	statement* sth = prepare(st);
	long rval = sth->execute();
	if (rval >= 0)
	{
		sth->bind(value);
		sth->fetch();
	}
	sth->finish();
	delete sth;
	return rval;
}

/**
 * Immediately execute the given statement, given a single parameter,
 * retrieving a single value
 *
 * @param	statement	Statement to execute.
 * @param	parameter	Parameter for SQL statement.
 * @param	value		Destination variable for fetched value.
 * @return	Number of rows affected;
 * @return	-1 on error.
 */
long dbi::executequery(const char* st, const std::string& parameter, std::string& value)
{
	statement* sth = prepare(st);
	sth->addparam(parameter);
	long rval = sth->execute();
	if (rval >= 0)
	{
		sth->bind(value);
		sth->fetch();
	}
	sth->finish();
	delete sth;
	return rval;
}


/**
 * Get the current type of database instance.
 *
 * @param	none
 * @return	Database type constant.
 */
databases dbi::gettype() const
{
	return imp->gettype();
}


/**
 * Get the CQWELL version number string.
 *
 * @param	none
 * @return	Pointer to version number string.
 *
 */
const char* dbi::version()
{
	return sqloco_version;
}


} // end namespace sqloco
