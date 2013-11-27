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
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include "SingleValueOption.h"

namespace options {

SingleValueOption::SingleValueOption(const std::string& name, const std::string& placeholder, const std::string& description)
: ValueOption(name, placeholder, description)
{
}

void SingleValueOption::setValue(const std::string& v)
{
	if(isUsed() && value != v) {
		std::ostringstream ss;
		ss << "Option '" << getName() << "' only takes a single value, but more than one was specified.";
		throw std::runtime_error(ss.str());
	}
	value = v;
}

} // namespace options
