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
#include <stdexcept>

#include "Group.h"

namespace options {

Group::Group()
: allowed(false)
{
}

void Group::add(const Option& opt)
{
	options.push_back(&opt);
}

void Group::allow()
{
	allowed = true;
}

void Group::check() const
{
	if(allowed)
		return;

	for(OptionList::const_iterator it = options.begin(); it != options.end(); ++it) {
		if((*it)->isUsed()) {
			std::ostringstream ss;
			ss << "Option '" << (*it)->getName() << "' has been used but was not allowed in that context.";
			throw std::runtime_error(ss.str());
		}
	}
}

} // namespace options
