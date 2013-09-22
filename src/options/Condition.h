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

// This is used for checking dependencies of options. Some options may only be used when other options have a certain value.
class Condition
{
public:
	Condition(bool s = false) : satisfied(s) {}
	void setSatisfied(bool s = true) { satisfied = s; }
	bool isSatisfied() const { return satisfied; }

private:
	bool satisfied;
};

} // namespace options