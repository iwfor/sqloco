/*
 * dbi_impl.cxx
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

#include <sqloco/statement.h>
#include "dbi_impl.h"

namespace sqloco {

dbi_impl::~dbi_impl()
{
	// Deallocate any statement handles that have not been freed.
	if (!sthlist.empty())
	{
		std::vector< statement* >::iterator it(sthlist.begin()),
			end(sthlist.end());
		for (; it != end; ++it)
			if (*it)
				delete *it;
	}
}

/*
 * Register a statement handle as being associated with this database instance.
 *
 */
void dbi_impl::regsth(statement* sth)
{
	sthlist.push_back(sth);
}

/*
 * Unregister a statement handle.
 *
 */
void dbi_impl::unregsth(statement* sth)
{
	std::vector< statement* >::iterator it(sthlist.begin()),
		end(sthlist.end());
	for (; it != end; ++it)
		if (*it == sth)
			*it = 0;
}

} // end namespace sqloco

