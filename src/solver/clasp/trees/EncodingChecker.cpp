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
#include "EncodingChecker.h"

namespace {
	typedef std::pair<Gringo::FWString, unsigned int> Predicate;
	const std::set<Predicate> predicates = {
		{"item",        2},
		{"auxItem",     2},
		{"extend",      2},
		{"currentCost", 1},
		{"cost",        1},
		{"length",      1},
		{"or",          1},
		{"and",         1},
		{"accept",      0},
		{"reject",      0},
	};
}

namespace solver { namespace clasp { namespace trees {

void EncodingChecker::showsig(Gringo::Location const &loc, Gringo::FWSignature sig, bool csp)
{
	const std::string name = (*sig).name();
	const unsigned int arity = (*sig).length();
	assert(!csp);
	showUsed = true;

	if(name != "") {
		if(predicates.find(Predicate{name, arity}) == predicates.end())
			std::cerr << "Warning: Unknown predicate " << name << '/' << arity << " shown at " << loc << '.' << std::endl;
		else {
			if(name == "length")
				lengthShown = true;
			else if(name == "item")
				itemShown = true;
			else if(name == "extend")
				extendShown = true;
			else if(name == "or" || name == "and")
				orAndShown = true;
			else if(name == "accept" || name == "reject")
				acceptRejectShown = true;
		}
	}
}

void EncodingChecker::check()
{
	if(showUsed) {
		if(!lengthShown)
			std::cerr << "Warning: No #show statement for length/1." << std::endl;
		if(!itemShown)
			std::cerr << "Warning: No #show statement for item/2." << std::endl;
		if(!extendShown)
			std::cerr << "Warning: No #show statement for extend/2." << std::endl;
		if(!orAndShown)
			std::cerr << "Warning: No #show statement for or/1 or and/1." << std::endl;
		if(!acceptRejectShown)
			std::cerr << "Warning: No #show statement for accept/0 or reject/0." << std::endl;
	}
	else
		std::cerr << "Warning: No #show statement was used." << std::endl;
}

}}} // namespace solver::clasp::trees
