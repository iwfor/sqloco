/*
 * dbi_postgresql.cxx
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

// This definition forces an object to exist, preventing problems on some
// platforms. (e.g.  MacOS X)
const static char* desc="$Id$";

#ifdef SQLOCO_ENABLE_POSTGRESQL

#include <sqloco/dbi.h>
#include <sqloco/statement.h>
#include <sqloco/except.h>
#include "dbi_impl.h"
#include "dbi_postgresql.h"
#include "statement_postgresql.h"

#include <libpq-fe.h>
#include <vector>
#include <cstdio>
#include <cstring>
#include <iostream>

namespace sqloco {

/*
 * Construct a DBI instance for the specified database. 
 *
 */
dbi_postgresql::dbi_postgresql() :
	conn(0)
{
}


/*
 * Release resources used by this DBI instance
 */
dbi_postgresql::~dbi_postgresql()
{
	close();
}


/*
 * Open a connection to the specified database.
 */
bool dbi_postgresql::open(const char* username, const char* password, const char* db,
		const char* hostname, unsigned int port)
{
	close();
	if (!hostname)
		return true;
	if (!db || !*db)
		db = "template1";
	if (port)
	{
		char portbuf[16];
		std::sprintf(portbuf, "%u", port);
		conn = PQsetdbLogin(hostname, portbuf, 0, "120", db, username, password);
	}
	else
		conn = PQsetdbLogin(hostname, 0, 0, "120", db, username, password);
	if (!conn || (PQstatus(conn) != CONNECTION_OK))
		return true;
	return false;
}


/*
 * 
 */
void dbi_postgresql::close()
{
	if (conn)
		PQfinish(conn);
	conn = 0;
}


/*
 * Check if a connection is open to the server.
 */
bool dbi_postgresql::isconnected() const
{
	return conn != 0;
}


/*
 * Check for an error condition.
 */
bool dbi_postgresql::error() const
{
	return error();
}


/*
 * Get the current error string.
 */
const char* dbi_postgresql::errstr()
{
	if (!conn)
		return "";
	return PQerrorMessage(conn);
}


/*
 * Prepare a statement handle for execution.
 */
statement* dbi_postgresql::prepare(const char* statement)
{
	return new statement_postgresql(this, statement);
}

} // end namespace sqloco

#endif // SQLOCO_ENABLE_MYSQL
