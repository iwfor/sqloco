/*
 * statement_sqlite.h
 *
 * $Id$
 *
 */
/*
 * Copyright (C) 2002-2003 Isaac W. Foraker (isaac@tazthecat.net)
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

#ifndef include_sqloco_statement_sqlite_h
#define include_sqloco_statement_sqlite_h
#include <sqloco/statement.h>
#include <sqlite.h>
#include <vector>
#include <string>
#include <queue>
#include <map>


namespace sqloco {

// forward declaration
class dbi_sqlite;
class dbi_impl;

struct sqlite_field_s {
	std::string* str;
	long* lnum;
	double* dnum;
	char* buf;
	unsigned size;

	std::string fieldname;
	unsigned maxlength;
	int type;
	
	sqlite_field_s() : str(0),lnum(0),dnum(0),buf(0),size(0) {}
	void clear() { str=0;lnum=0;dnum=0;buf=0;size=0; }
	bool getfieldinfo()
	{
	}
};

class statement_sqlite : public statement {
public:
	statement_sqlite(dbi_impl* dbhi, const char*);
	~statement_sqlite();

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
	statement_sqlite();
	// These variables are set by dbi::prepare()
	std::string statement_;

	dbi_sqlite* dbhi_;
    bool havecursor_;
    struct sqlite_vm cursor_;

	// These variables are set by statement::[methods]()
	std::queue< std::string > params_;
	std::vector< std::string > fieldnames_;
	std::vector< sqlite_field_s > bindings_;
	std::map< std::string, bool > nullfields_;
};



} // end namespace sqloco

#endif // include_sqloco_statement_sqlite_h
