/*
 * statement_sqlite.cxx
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

// This definition forces an object to exist, preventing problems on some
// platforms. (e.g.  MacOS X)
const static char* module_id="$Id$";

#ifdef SQLOCO_ENABLE_SQLITE

#include "statement_sqlite.h"
#include "dbi_impl.h"
#include "dbi_sqlite.h"
#include "dbshared.h"
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
statement_sqlite::statement_sqlite(dbi_impl* d, const char* stmt) :
    statement(stmt), dbhi_(dynamic_cast<dbi_sqlite*>(d)), havecursor_(false)
{
    dbhi_->regsth(this);
}


/*
 * 
 */
statement_sqlite::~statement_sqlite()
{
    dbhi_->unregsth(this);
}


/*
 * 
 */
void statement_sqlite::finish()
{
    if (havecursor_)
    {
        sqlite_finalize(&cursor_, dbhi_->errstr_);
        havecursor_ = false;
    }
    bindings_.clear();
    fieldnames_.clear();
}


/* 
 *
 */
void statement_sqlite::addparam(long num)
{   
    char buf[32];
    params_.push(itoa(buf, num));
}   
    
    
/*
 * 
 */
void statement_sqlite::addparam(unsigned long num)
{
    char buf[32];
    params_.push(utoa(buf, num));
}


/*
 * 
 */
void statement_sqlite::addparam(double num)
{
    char buf[32];
    std::sprintf(buf, "%lf", num);
    params_.push(buf);
}


/*
 * 
 */
void statement_sqlite::addparam(const char* str, unsigned length)
{   
    if (!str)
    {   
        params_.push("NULL");
        return;
    }
    if (!length)
        length = std::strlen(str);

    unsigned buflength = length*2 + 1;  // 1 for null
    std::string fmt("'");
    // TODO: Make this buffer a member of the class to reduce memory allocation.
    char* buf = new char[buflength];
    try {
        // Convert reserved characters in string
//      TODO
//      sqlite_real_escape_string(dbhi_->conn, buf, str, length);
        fmt+= buf;
    } catch(...) {
        delete [] buf;
        throw;
    }
    delete [] buf;
    fmt+= '\'';
    params_.push(fmt);
}


/*
 *
 */
void statement_sqlite::addparam(const std::string& str)
{
    addparam(str.c_str(), str.length());
}


/*
 *
 */
long statement_sqlite::execute()
{
    // Clear storage
    fieldnames_.clear();
    nullfields_.clear();
    bindings_.clear();

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
                if (params_.empty())
                    stmt+= "''";
                else
                {
                    stmt+= params_.front();
                    params_.pop();
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
    const char* ignore;
    if (sqlite_compile(dbhi_->conn, stmt.c_str(), &ignore, &cursor_,
                dbhi_->errstr_) != SQLITE_OK)
    {
        return -1;
    }
    havecursor_ = true;

    // TODO
/*
    if ( !(cursor = sqlite_store_result(dbhi_->conn)) )
        return sqlite_affected_rows(dbhi_->conn);

    unsigned int num_fields, i;
    SQLITE_FIELD *fields;
    num_fields = sqlite_num_fields(cursor);
    fields = sqlite_fetch_fields(cursor);
    for(i = 0; i < num_fields; i++)
        fieldnames_.push_back(fields[i].name);
*/
    return sqlite_num_rows(cursor);
}


/*
 * 
 */
bool statement_sqlite::bind(std::string& str)
{
    sqlite_field_s f;
    f.str = &str;
    if (f.getfieldinfo(cursor))
        return true;
    bindings_.push_back(f);
    return false;
}


/*
 * 
 */
bool statement_sqlite::bind(long& num)
{
    sqlite_field_s f;
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
        return true;    // incompatible type
    bindings_.push_back(f);
    return false;
}


/*
 * 
 */
bool statement_sqlite::bind(double& num)
{
    sqlite_field_s f;
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
        return true;    // incompatible type
    bindings_.push_back(f);
    return false;
}


/*
 * 
 */
bool statement_sqlite::bind(char* buf, unsigned size)
{
    sqlite_field_s f;
    f.buf = buf;
    f.size = size;
    if (f.getfieldinfo(cursor))
        return true;
    bindings_.push_back(f);
    return false;
}


/*
 * 
 */
bool statement_sqlite::fetch()
{
    SQLITE_ROW row;
    if (!cursor)
        return true;

    if ( !(row = sqlite_fetch_row(cursor)) )
        return true;
    long fcount = sqlite_num_fields(cursor),
        bcount = bindings_.size();
    bool isnull;
    for (long i=0; i < fcount && i < bcount; ++i)
    {
        sqlite_field_s& f = bindings_[i];
        isnull = row[i] ? false : true;
        nullfields_[fieldnames_[i]] = isnull;
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
bool statement_sqlite::fetchhash(Hash& hash)
{
    SQLITE_ROW row;
    if (!cursor)
        return true;

    hash.clear();
    if ( !(row = sqlite_fetch_row(cursor)) )
        return true;
    long fcount = sqlite_num_fields(cursor);
    for (long i=0; i < fcount; ++i)
    {
        sqlite_field_s& f = bindings_[i];
        if (row[i])
        {
            hash[fieldnames_[i]] = row[i];
            nullfields_[fieldnames_[i]] = false;
        }
        else
            nullfields_[fieldnames_[i]] = true;
    }
    return false;
}


/*
 *
 */
bool statement_sqlite::isnull(const std::string& fieldname)
{
    return nullfields_[fieldname];
}

/*
 *
 */
bool statement_sqlite::isnull(unsigned fieldno)
{
    if (fieldno > fieldnames_.size())
        return true;
    return nullfields_[fieldnames_[fieldno]];
}


} // end namespace sqloco

#endif // SQLOCO_ENABLE_SQLITE

