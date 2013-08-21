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
#include <map>

#include "Option.h"

namespace options {

class OptionHandler
{
public:
	typedef std::string Section;
	static const Section GENERAL_SECTION; // Section that is used by default

	// Adds the given option to the given section. When calling, there must not be an option with the same name in any section.
	void add(Option& opt, const Section& section = GENERAL_SECTION);
	void parse(int argc, char** argv);
	void printHelp() const;

private:
	typedef std::vector<const Option*> OptionList;
	typedef std::vector<std::pair<Section, OptionList> > SectionList;

	SectionList sections;

	// Given an option name, we want to find the option quickly.
	typedef std::map<std::string, Option*> NameToOption;
	NameToOption names;
};

} // namespace options
