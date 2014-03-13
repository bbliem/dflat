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

#pragma once
//}}}
#include "Option.h"

namespace options {

class ValueOption : public Option
{
public:
	ValueOption(const std::string& name, const std::string& placeholder, const std::string& description);

	// If the option is actually specified by the user, be sure to call setUsed() *after* calling setValue().
	virtual void setValue(const std::string& value) = 0;
	virtual void printHelp() const override;

protected:
	std::string placeholder;
};

} // namespace options
