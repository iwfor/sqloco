/*
 * dbi.h
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


#ifndef __sqloco_dbi_h
#define __sqloco_dbi_h

#include "const.h"
#include <string>

namespace sqloco {

// Forward Declarations
struct dbi_impl;
class statement;

/**
 * The main database interface class.
 *
 * @except dbi::exception
 */
class dbi {
public: 
	dbi(databases);
	~dbi();

	/// Open a database connection.
	bool open(const char* username, const char* password, const char* db = 0,
			  const char* hostname = "localhost", unsigned int port=0);

	/// Close the database connection.
	void close();

	/// Check for an error.
	bool error();

	/// Get the last error string.
	const char* errstr();

	/// Prepare a query for execution.
	statement* prepare(const char* query);

	/// Prepare a query for execution.
	statement* prepare(const std::string& query);

	/// Execute a statement without reading the respone.
	long execute(const char* statement);

	/// Execute a query and get its response.
	long executequery(const char* st, std::string& value);

	/// Execute a query and get its response.
	long executequery(const char* st, const std::string& parameter, std::string& value);

	/// Check if connected to the server.
	operator bool() const;

	/// Check if not connected to the server.
	bool operator !() const;

	/// Get the type of this database instance.
	databases gettype() const;

	/// Get library version number string.
	const char* version();

private:
	dbi();
	friend class statement;
	dbi_impl* imp;
};

} // end namespace sqloco

#endif // __sqloco_dbi_h
