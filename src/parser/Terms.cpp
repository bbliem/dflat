/*{{{
Copyright 2012-2013, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dflat/>.

This file is part of D-FLAT.

D-FLAT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

D-FLAT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with D-FLAT.  If not, see <http://www.gnu.org/licenses/>.
*/
//}}}
#include <ostream>

#include "Terms.h"

namespace parser {

Terms::Terms(std::string* term)
{
	terms.push_back(term);
}

Terms::~Terms()
{
	for(auto* t : terms)
		delete t;
}

void Terms::push_back(std::string* term)
{
	terms.push_back(term);
}

const Terms::List& Terms::getTerms() const
{
	return terms;
}

std::ostream& operator<<(std::ostream& stream, const Terms& terms)
{
	Terms::List::const_iterator i = terms.terms.begin();
	if(i != terms.terms.end())
		stream << **(i++);
	while(i != terms.terms.end())
		stream << ',' << **(i++);
	return stream;
}

} // namespace parser
