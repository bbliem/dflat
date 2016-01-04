/*{{{
Copyright 2012-2016, Bernhard Bliem
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
#include <cassert>
#include <stdexcept>

#include "Driver.h"
#include "Terms.h"
#include "../Instance.h"

namespace parser {

Driver::Driver(const std::string& filename, const Predicates& hyperedgePredicateNames)
	: filename(filename)
	, hyperedgePredicateNames(hyperedgePredicateNames)
{
}

Driver::~Driver()
{
}

Instance Driver::parse()
{
	Instance instance;
	scan_begin();
	::yy::Parser parser(*this, instance);
	int res = parser.parse();
	scan_end();
	if(res != 0)
		throw std::runtime_error("Parse error");
	return instance;
}

void Driver::error(const yy::location& l, const std::string& m)
{
	std::ostringstream ss;
	ss << "Parse error." << std::endl << l << ": " << m;
	throw std::runtime_error(ss.str());
}

void Driver::processFact(Instance& instance, const std::string& predicate, const Terms* arguments)
{
	if(hyperedgePredicateNames.find(predicate) == hyperedgePredicateNames.end()) {
		std::stringstream fact;
		fact << predicate;
		if(arguments) {
			assert(arguments->getTerms().empty() == false);
			char sep = '(';
			for(const std::string* term : arguments->getTerms()) {
				fact << sep << *term;
				sep = ',';
			}
			fact << ')';
		}
		instance.addNonEdgeFact(fact.str());
	}

	else {
		Instance::Edge hyperedge;

		if(arguments) {
			for(const std::string* term : arguments->getTerms())
				hyperedge.emplace_back(std::string(*term));
		}

		instance.addEdgeFact(predicate, hyperedge);
	}
}

} // namespace parser
