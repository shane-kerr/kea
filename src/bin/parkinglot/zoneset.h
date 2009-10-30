// Copyright (C) 2009  Internet Systems Consortium, Inc. ("ISC")
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

// $Id$

#ifndef __ZONESET_H
#define __ZONESET_H 1

#include <set>

class ZoneSet : std::set<std::string> {
    public:
        void serve(std::string s) {
            std::cout << "now serving: " << s << std::endl;
            this->insert(s);
        }
        void forget(std::string s) {
            std::cout << "no longer serving: " << s << std::endl;
            this->erase(s);
        }
        bool contains(std::string s) {
            return (this->find(s) != this->end());
        }
};

#endif // __ZONESET_H

// Local Variables:
// mode: c++
// End:
