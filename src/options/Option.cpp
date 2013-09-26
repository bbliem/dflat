/*
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

#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include "Option.h"
#include "Condition.h"

namespace options {

Option::Option(const std::string& name, const std::string& description)
	: name(name)
	, dashedName(name.size() > 1 ? "--" + name : "-" + name)
	, description(description)
	, used(false)
{
}

Option::~Option()
{
}

void Option::addCondition(const Condition& condition)
{
	conditions.push_back(&condition);
}

void Option::checkConditions() const
{
	if(!used)
		return;

	for(const Condition* c : conditions) {
		if(c->isSatisfied() == false) {
			std::ostringstream ss;
			ss << "Option '" << name << "' has been used but was not allowed in that context.";
			throw std::runtime_error(ss.str());
		}
	}
}

void Option::printHelp() const
{
	std::cerr << "  " << std::left << std::setw(NAME_WIDTH) << dashedName << " : " << description << std::endl;
}

} // namespace options
