/*
 * statement_mysql.cxx
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

const static char* desc="$Ver$";

#ifdef SQLOCO_ENABLE_MYSQL

#include "statement_mysql.h"
#include "dbi_impl.h"
#include "dbi_mysql.h"
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
statement_mysql::statement_mysql(dbi_impl* d, const char* stmt)
	: statement(stmt), dbhi(dynamic_cast<dbi_mysql*>(d)), cursor(0)
{
	dbhi->regsth(this);
}


/*
 * 
 */
statement_mysql::~statement_mysql()
{
	dbhi->unregsth(this);
	if (cursor)
		mysql_free_result(cursor);
}


/*
 * 
 */
void statement_mysql::finish()
{
	if (cursor)
	{
		mysql_free_result(cursor);
		cursor = 0;
	}
	bindings.clear();
	fieldnames.clear();
}


/* 
 *
 */
void statement_mysql::addparam(long num)
{   
	char buf[32];
	std::sprintf(buf, "%ld", num);
	params.push(buf);
}   
	
	
/*
 * 
 */
void statement_mysql::addparam(unsigned long num)
{
	char buf[32];
	std::sprintf(buf, "%lu", num);
	params.push(buf);
}


/*
 * 
 */
void statement_mysql::addparam(double num)
{
	char buf[32];
	std::sprintf(buf, "%lf", num);
	params.push(buf);
}


/*
 * 
 */
void statement_mysql::addparam(const char* str, unsigned length)
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
	try {
		// Convert reserved characters in string
		mysql_real_escape_string(dbhi->conn, buf, str, length);
		fmt+= buf;
	} catch(...) {
		delete [] buf;
		throw;
	}
	delete [] buf;
	fmt+= '\'';
	params.push(fmt);
}


/*
 *
 */
void statement_mysql::addparam(const std::string& str)
{
	addparam(str.c_str(), str.length());
}


/*
 *
 */
long statement_mysql::execute()
{
	if (cursor)
		mysql_free_result(cursor);
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

	// Send the query
	if (mysql_real_query(dbhi->conn, stmt.c_str(),
				stmt.length()))
	{
		return -1;
	}
	
	if ( !(cursor = mysql_store_result(dbhi->conn)) )
		return mysql_affected_rows(dbhi->conn);

	unsigned int num_fields, i;
	MYSQL_FIELD *fields;
	num_fields = mysql_num_fields(cursor);
	fields = mysql_fetch_fields(cursor);
	for(i = 0; i < num_fields; i++)
		fieldnames.push_back(fields[i].name);

	return mysql_num_rows(cursor);
}


/*
 * 
 */
bool statement_mysql::bind(std::string& str)
{
	mysql_field_s f;
	f.str = &str;
	if (f.getfieldinfo(cursor))
		return true;
	bindings.push_back(f);
	return false;
}


/*
 * 
 */
bool statement_mysql::bind(long& num)
{
	mysql_field_s f;
	f.lnum = &num;
	if (f.getfieldinfo(cursor))
		return true;
	if (f.type != FIELD_TYPE_FLOAT &&
			f.type != FIELD_TYPE_DECIMAL &&
			f.type != FIELD_TYPE_LONGLONG &&
			f.type != FIELD_TYPE_INT24 &&
			f.type != FIELD_TYPE_LONG &&
			f.type != FIELD_TYPE_SHORT &&
			f.type != FIELD_TYPE_TINY)
		return true;	// incompatible type
	bindings.push_back(f);
	return false;
}


/*
 * 
 */
bool statement_mysql::bind(double& num)
{
	mysql_field_s f;
	f.dnum = &num;
	if (f.getfieldinfo(cursor))
		return true;
	if (f.type != FIELD_TYPE_FLOAT &&
			f.type != FIELD_TYPE_DECIMAL &&
			f.type != FIELD_TYPE_LONGLONG &&
			f.type != FIELD_TYPE_INT24 &&
			f.type != FIELD_TYPE_LONG &&
			f.type != FIELD_TYPE_SHORT &&
			f.type != FIELD_TYPE_TINY)
		return true;	// incompatible type
	bindings.push_back(f);
	return false;
}


/*
 * 
 */
bool statement_mysql::bind(char* buf, unsigned size)
{
	mysql_field_s f;
	f.buf = buf;
	f.size = size;
	if (f.getfieldinfo(cursor))
		return true;
	bindings.push_back(f);
	return false;
}


/*
 * 
 */
bool statement_mysql::fetch()
{
	MYSQL_ROW row;
	if (!cursor)
		return true;

	if ( !(row = mysql_fetch_row(cursor)) )
		return true;
	long fcount = mysql_num_fields(cursor),
		bcount = bindings.size();
	bool isnull;
	for (long i=0; i < fcount && i < bcount; ++i)
	{
		mysql_field_s& f = bindings[i];
		isnull = row[i] ? false : true;
		nullfields[fieldnames[i]] = isnull;
		if (f.str)
		{
			if (!isnull)
				*f.str = row[i];
			else
				f.str->erase();
		}
		else if (f.lnum)
		{
			if (!isnull)
				*f.lnum = std::atoi(row[i]);
			else
				*f.lnum = 0;
		}
		else if (f.dnum)
		{
			if (!isnull)
				*f.dnum = std::atof(row[i]);
			else
				*f.dnum = 0;
		}
		else if (f.buf)
		{
			if (!isnull)
			{
				unsigned size = f.size < f.maxlength ? f.size : f.maxlength;
				std::memcpy(f.buf, row[i], size);
			}
			else
				std::memset(f.buf, 0, f.size);
		}
	}
	return false;
}


/*
 * 
 */
bool statement_mysql::fetchhash(Hash& hash)
{
	MYSQL_ROW row;
	if (!cursor)
		return true;

	hash.clear();
	if ( !(row = mysql_fetch_row(cursor)) )
		return true;
	long fcount = mysql_num_fields(cursor);
	for (long i=0; i < fcount; ++i)
	{
		mysql_field_s& f = bindings[i];
		if (row[i])
		{
			hash[fieldnames[i]] = row[i];
			nullfields[fieldnames[i]] = false;
		}
		else
			nullfields[fieldnames[i]] = true;
	}
	return false;
}


/*
 *
 */
bool statement_mysql::isnull(const std::string& fieldname)
{
	return nullfields[fieldname];
}

/*
 *
 */
bool statement_mysql::isnull(unsigned fieldno)
{
	if (fieldno > fieldnames.size())
		return true;
	return nullfields[fieldnames[fieldno]];
}


} // end namespace sqloco

#endif // SQLOCO_ENABLE_MYSQL
