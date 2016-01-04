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
#include "String.h"

String::StringToId String::stringToId;
std::vector<const std::string*> String::strings;

String::String(std::string&& content)
{
	auto result = stringToId.emplace(std::move(content), strings.size());
	id = result.first->second;
	if(result.second)
		strings.push_back(&result.first->first);
}

void String::clear()
{
	strings.clear();
	stringToId.clear();
}

std::ostream& operator<<(std::ostream& os, const String& str)
{
	return os << *str;
}
