/*
 * statement.h
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
 *	notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in
 *	the documentation and/or other materials provided with the
 *	distribution.
 * 3. Neither the name of the Author nor the names of its contributors
 *	may be used to endorse or promote products derived from this
 *	software without specific prior written permission.
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

#ifndef __sqloco_statement_h
#define __sqloco_statement_h

#include "const.h"
#include <string>
#include <map>

namespace sqloco {

// Forward Declarations
class dbi;

typedef std::map< std::string, std::string > Hash;

class statement {
public: 
	statement() {};
	virtual ~statement() {};

	virtual void finish() = 0;

	virtual void addparam(long num) = 0;
	virtual void addparam(unsigned long num) = 0;
	virtual void addparam(double num) = 0;
	virtual void addparam(const char* str, unsigned length=0) = 0;
	virtual void addparam(const std::string& str) = 0;

	virtual long execute() = 0;
	virtual bool bind(std::string& str) = 0;
	virtual bool bind(long& num) = 0;
	virtual bool bind(double& num) = 0;
	virtual bool bind(char* buf, unsigned size) = 0;
	virtual bool fetch() = 0;
	virtual bool fetchhash(Hash& hash) = 0;
	virtual bool isnull(const std::string& fieldname) = 0;
	virtual bool isnull(unsigned fieldno) = 0;
	virtual long getuid() = 0;
};

} // end namespace sqloco

#endif // __sqloco_statement_h
