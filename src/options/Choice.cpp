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
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <cassert>

#include "Choice.h"
#include "Condition.h"

namespace options {

Choice::Choice(const std::string& name, const std::string& placeholder, const std::string& description)
: SingleValueOption(name, placeholder, description)
{
}

void Choice::addChoice(const std::string& choiceName, const std::string& description, bool newDefault)
{
	possibilities.emplace_back(choiceName, description);
	if(newDefault) {
		defaultValue = choiceName;
		value = choiceName;
	}
}

void Choice::setValue(const std::string& v)
{
	for(const Possibility& p : possibilities) {
		if(p.name == v) {
			SingleValueOption::setValue(v);
			return;
		}
	}

	std::ostringstream msg;
	msg << '\'' << v << "' is not a permitted value for the option '" << getName() << '\'' << std::endl;
	throw std::runtime_error(msg.str());
}

void Choice::checkConditions() const
{
	SingleValueOption::checkConditions();

	// If this has no default, it must have been used (unless a condition is not satisfied).
	if(defaultValue.empty() == false)
		return;

	for(const Condition* c : conditions)
		if(c->isSatisfied() == false)
			return;

	if(!isUsed()) {
		std::ostringstream ss;
		ss << "Option '" << getName() << "' has not been supplied";
		throw std::runtime_error(ss.str());
	}
}

void Choice::printHelp() const
{
	SingleValueOption::printHelp();
	//std::cerr << "      Permitted values for <" << placeholder << ">:" << std::endl;
	std::cerr << "      Permitted values:" << std::endl;
	for(const Possibility& p : possibilities) {
		std::cerr << "      " << std::right << std::setw(POSSIBILITY_NAME_WIDTH) << p.name << " : " << p.description;
		if(p.name == defaultValue)
			std::cerr << " (default)";
		std::cerr << std::endl;
	}
}

} // namespace options
