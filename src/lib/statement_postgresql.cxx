/*
 * statement_postgresql.cxx
 *
 * $Id$
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

#include "statement_postgresql.h"
#include "dbi_impl.h"
#include "dbi_postgresql.h"
#include <sqloco/statement.h>
#include <sqloco/dbi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

namespace sqloco {


/*
 * 
 */
statement_postgresql::statement_postgresql(dbi_impl* d, const char* stmt)
	: statement(stmt), dbhi(dynamic_cast<dbi_postgresql*>(d)), cursor(0),
	numrows(0), curfield(0), currow(0)
{
	dbhi->regsth(this);
}


/*
 * 
 */
statement_postgresql::~statement_postgresql()
{
	dbhi->unregsth(this);
	if (cursor)
		PQclear(cursor);
}


/*
 * 
 */
void statement_postgresql::finish()
{
	if (cursor)
	{
		PQclear(cursor);
		cursor = 0;
	}
	numrows = 0;
	curfield = 0;
	currow = 0;
	bindings.clear();
	fieldnames.clear();
}


/* 
 *
 */
void statement_postgresql::addparam(long num)
{   
	char buf[32];
	std::sprintf(buf, "%ld", num);
	params.push(buf);
}   
	
	
/*
 * 
 */
void statement_postgresql::addparam(unsigned long num)
{
	char buf[32];
	std::sprintf(buf, "%lu", num);
	params.push(buf);
}


/*
 * 
 */
void statement_postgresql::addparam(double num)
{
	char buf[32];
	std::sprintf(buf, "%lf", num);
	params.push(buf);
}


/*
 * 
 */
void statement_postgresql::addparam(const char* str, unsigned length)
{   
	if (!str)
	{   
		params.push("NULL");
		return;
	}
	if (!length)
		length = std::strlen(str);

	unsigned buflength = length*2 + 1;  // 1 for null
	std::string fmt("'");
	char* buf = new char[buflength];
	try {	// Make sure memory is not leaked if there is an exception.
		PQescapeString(buf, str, length);
		fmt+= buf;
		fmt+= '\'';
	} catch(...) {
		delete [] buf;
		throw;
	}
	delete [] buf;
	params.push(fmt);
}


/*
 *
 */
void statement_postgresql::addparam(const std::string& str)
{
	addparam(str.c_str(), str.length());
}


/*
 *
 */
long statement_postgresql::execute()
{
	if (cursor)
		PQclear(cursor);
	cursor = 0;

	// Clear storage
	fieldnames.clear();
	nullfields.clear();
	bindings.clear();

	// read through statement, replacing ? with values added with
	// addparam() functions.
	std::string stmt;
	bool in1str=false;
	bool in2str=false;
	std::string::const_iterator it(statement.begin()),
		end(statement.end());
	for (; it != end; ++it)
	{
		if (in1str || in2str)
		{
			stmt+= *it;
			if (in1str && *it == '\'')
				in1str = false;
			else if (in2str && *it == '"')
				in2str = false;
		}
		else
		{
			if (*it == '?')
			{
				if (params.empty())
					stmt+= "''";
				else
				{
					stmt+= params.front();
					params.pop();
				}
			}
			else
			{
				stmt+= *it;
				if (*it == '\'')
					in1str = true;
				else if (*it == '\"')
					in2str = true;
			}
		}
	}

	if ((cursor = PQexec(dbhi->conn, stmt.c_str())) == 0)
		return -1;

	char *tuples = PQcmdTuples(cursor);
	if ( *tuples != '\0' )
		return std::atoi(tuples);

	unsigned int num_fields, i;
	num_fields = PQnfields(cursor);
	for(i = 0; i < num_fields; i++)
		fieldnames.push_back(PQfname(cursor, i));

	numrows = PQntuples(cursor);
	return numrows;
}


/*
 * 
 */
bool statement_postgresql::bind(std::string& str)
{
	if (curfield >= fieldnames.size())
		return true;
	postgresql_field_s f;
	f.str = &str;
	if (f.getfieldinfo(cursor, curfield))
		return true;
	++curfield;
	bindings.push_back(f);
	return false;
}


/*
 * 
 */
bool statement_postgresql::bind(long& num)
{
	if (curfield >= fieldnames.size())
		return true;
	postgresql_field_s f;
	f.lnum = &num;
	if (f.getfieldinfo(cursor, curfield))
		return true;
//	TODO
//	if (f.type != FIELD_TYPE_FLOAT &&
//			f.type != FIELD_TYPE_DECIMAL &&
//			f.type != FIELD_TYPE_LONGLONG &&
//			f.type != FIELD_TYPE_INT24 &&
//			f.type != FIELD_TYPE_LONG &&
//			f.type != FIELD_TYPE_SHORT &&
//			f.type != FIELD_TYPE_TINY)
//		return true;	// incompatible type
	++curfield;
	bindings.push_back(f);
	return false;
}


/*
 * 
 */
bool statement_postgresql::bind(double& num)
{
	if (curfield >= fieldnames.size())
		return true;
	postgresql_field_s f;
	f.dnum = &num;
	if (f.getfieldinfo(cursor, curfield))
		return true;
//	TODO
//	if (f.type != FIELD_TYPE_FLOAT &&
//			f.type != FIELD_TYPE_DECIMAL &&
//			f.type != FIELD_TYPE_LONGLONG &&
//			f.type != FIELD_TYPE_INT24 &&
//			f.type != FIELD_TYPE_LONG &&
//			f.type != FIELD_TYPE_SHORT &&
//			f.type != FIELD_TYPE_TINY)
//		return true;	// incompatible type
	++curfield;
	bindings.push_back(f);
	return false;
}


/*
 * 
 */
bool statement_postgresql::bind(char* buf, unsigned size)
{
	if (curfield >= fieldnames.size())
		return true;
	postgresql_field_s f;
	f.buf = buf;
	f.size = size;
	if (f.getfieldinfo(cursor, curfield))
		return true;
	++curfield;
	bindings.push_back(f);
	return false;
}


/*
 * 
 */
bool statement_postgresql::fetch()
{
	if (!cursor)
		return true;
	if (currow >= numrows)
		return true;

	long bcount = bindings.size();
	const char* result;
	bool isnull;
	for (long i=0; i < bcount; ++i)
	{
		postgresql_field_s& f = bindings[i];
		result = PQgetvalue(cursor, currow, i);
		isnull = PQgetisnull(cursor, currow, i);
		nullfields[fieldnames[i]] = isnull;
		if (f.str)
			*f.str = result;
		else if (f.lnum)
			*f.lnum = std::atoi(result);
		else if (f.dnum)
			*f.dnum = std::atof(result);
		else if (f.buf)
		{
			if (!isnull)
			{
				unsigned size = PQgetlength(cursor, currow, i);
				// Choose the smaller buffer size for copy
				size = (f.size < size) ? f.size : size;
				std::memcpy(f.buf, result, size);
			}
			else
			{
				std::memset(f.buf, 0, f.size);
			}
		}
	}
	++currow;
	return false;
}


/*
 * 
 */
bool statement_postgresql::fetchhash(Hash& hash)
{
	if (!cursor)
		return true;
	if (currow >= numrows)
		return true;

	hash.clear();
	long fcount = fieldnames.size();
	for (long i=0; i < fcount; ++i)
	{
		postgresql_field_s& f = bindings[i];
		hash[fieldnames[i]] = PQgetvalue(cursor, currow, i);
		nullfields[fieldnames[i]] = PQgetisnull(cursor, currow, i);;
	}
	++currow;
	return false;
}


/*
 *
 */
bool statement_postgresql::isnull(const std::string& fieldname)
{
	return nullfields[fieldname];
}

/*
 *
 */
bool statement_postgresql::isnull(unsigned fieldno)
{
	return nullfields[fieldnames[fieldno]];
}


} // end namespace sqloco

#endif // SQLOCO_ENABLE_POSTGRESQL
