/*
 * statement_postgresql.h
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

#ifndef __sqloco_statement_postgresql_h
#define __sqloco_statement_postgresql_h

#include <sqloco/statement.h>
#include <libpq-fe.h>
#include <vector>
#include <string>
#include <queue>
#include <map>

namespace sqloco {

// forward declaration
class dbi_postgresql;
class dbi_impl;

struct postgresql_field_s {
	std::string* str;
	long* lnum;
	double* dnum;
	char* buf;
	unsigned size;

	std::string fieldname;
	unsigned maxlength;
	Oid type;
	
	postgresql_field_s() : str(0),lnum(0),dnum(0),buf(0),size(0) {}
	void clear() { str=0;lnum=0;dnum=0;buf=0;size=0; }
	bool getfieldinfo(PGresult* cursor, int index)
	{
		type = PQftype(cursor, index);
		fieldname = PQfname(cursor, index);
		maxlength = PQfsize(cursor, index);
		return false;
	}
};


class statement_postgresql : public statement {
public:
	statement_postgresql(dbi_impl* dbhi, const char*);
	~statement_postgresql();

	void addparam(long);
	void addparam(unsigned long);
	void addparam(double);
	void addparam(const char*, unsigned);
	void addparam(const std::string&);

	long execute();
	void finish();

	bool bind(std::string&);
	bool bind(long&);
	bool bind(double&);
	bool bind(char*, unsigned);

	bool fetch();
	bool fetchhash(Hash& hash);
	bool isnull(const std::string& fieldname);
	bool isnull(unsigned fieldno);
	
private:
	statement_postgresql();
	// These variables are set by dbi::prepare()
	std::string statement;

	dbi_postgresql* dbhi;

	// These variables are set by statement::[methods]()
	std::queue< std::string > params;
	std::vector< std::string > fieldnames;
	std::vector< postgresql_field_s > bindings;
	std::map< std::string, bool > nullfields;
	int numrows;
	int curfield;
	int currow;
	PGresult* cursor;
};


} // end namespace sqloco

#endif // __sqloco_statement_postgresql_h
