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
#include "../Instance.h"

namespace parser {

Driver::Driver(std::istream& input, const Predicates& hyperedgePredicateNames)
	: input(input)
	, hyperedgePredicateNames(hyperedgePredicateNames)
{
}

Driver::~Driver()
{
}

Instance Driver::parse()
{
	Instance instance;
	Scanner scanner(&input);
	Parser parser(scanner, *this, instance);
	int res = parser.parse();
	if(res != 0)
		throw std::runtime_error("Parse error");
	return instance;
}

void Driver::processFact(Instance& instance, const std::string& predicate, const std::vector<std::string>& arguments)
{
	if(hyperedgePredicateNames.find(predicate) == hyperedgePredicateNames.end()) {
		std::stringstream fact;
		fact << predicate;
		if(!arguments.empty()) {
			char sep = '(';
			for(const std::string& arg : arguments) {
				fact << sep << arg;
				sep = ',';
			}
			fact << ')';
		}
		instance.addNonEdgeFact(fact.str());
	}

	else {
		Instance::Edge hyperedge;
		for(const std::string& arg : arguments)
			hyperedge.emplace_back(std::string(arg));
		instance.addEdgeFact(predicate, hyperedge);
	}
}

} // namespace parser
