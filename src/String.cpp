/*{{{
Copyright 2012-2014, Bernhard Bliem
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
#include "String.h"

String::Values String::values;

String::String(std::string&& content)
	: it(values.insert(std::move(content)).first)
{
}

String::String(const std::string& content)
	: it(values.emplace(content).first)
{
}

bool String::operator<(const String& rhs) const
{
	return *it < *rhs.it;
}

bool String::operator>(const String& rhs) const
{
	return *it > *rhs.it;
}

bool String::operator==(const String& rhs) const
{
	return *it == *rhs.it;
}

std::ostream& operator<<(std::ostream& os, const String& str)
{
	return os << *str;
}
