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

#pragma once

#include <vector>

#include "Option.h"

namespace options {

// An option group is used for checking dependencies of options. For instance, some options may only be used when other options have a certain value.
class Group
{
public:
	// Initially all options contained in this group are not allowed to be used. This is changed by calling allow().
	Group();

	void add(const Option& opt);

	// Allow the options contained in this group to be used.
	void allow();

	// Throw an exception when allow() has not been called but any of the contained options have been used.
	void check() const;

private:
	typedef std::vector<const Option*> OptionList;

	OptionList options;
	bool allowed;
};

} // namespace options
