/*
 * dbi_mysql.h
 *
 */
/*
 * Copyright (C) 2002-2003 Isaac W. Foraker (isaac at noscience dot net)
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

#ifndef __sqloco_dbi_mysql_h
#define __sqloco_dbi_mysql_h

#include <sqloco/const.h>
#include "dbi_impl.h"
#include <mysql.h>

namespace sqloco {

class dbi_mysql : public dbi_impl {
public:
	dbi_mysql();
	~dbi_mysql();

	bool open(const char* username, const char* password, const char* db = 0,
			 const char* hostname = "localhost", unsigned int port=0);
	void close();
	bool error() const;
	const char* errstr();
	statement* prepare(const char* query);
	bool isconnected() const;
	databases gettype() const { return db_mysql; }

	friend class statement_mysql;
	 
private:
	MYSQL* conn;
	MYSQL conn_struct;
	std::vector< std::string > params;
};


} // end namespace sqloco

#endif // __sqloco_dbi_mysql_h
