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

#pragma once
//}}}
#include <vector>
#include <map>
#include <string>

namespace options {

class Option;
class Observer;

class OptionHandler
{
public:
	static const std::string GENERAL_SECTION; // Section that is used by default

	OptionHandler();

	// Adds the given option to the given section. When calling, there must not be an option with the same name in any section. If the section does not exist yet, it is added.
	void addOption(Option& opt, const std::string& section = GENERAL_SECTION);

	// Parses the command-line and then notifies observers. (They could then, e.g., determine which conditions are satisfied.)
	// Throws an exception if the command-line is not well-formed (e.g., if unknown options are used).
	// Afterwards you should call checkConditions() to check for semantic validity of the parsed options.
	// We assume that argv[0] contains the first option, NOT the binary name
	void parse(int argc, const char* const* const argv);

	// After parsing, this can be used to check if the parsed options are semantically valid.
	// That is: The conditions of all options are checked (in particular, all choice options without defaults must have been used).
	// Throws an exception if a condition is violated.
	void checkConditions();

	void printHelp(std::ostream& out) const;

	// Registers an observer that is notified when parsing is done.
	void registerObserver(Observer& observer);

private:
	void checkConditions() const;

	typedef std::vector<const Option*> OptionList;
	typedef std::vector<std::pair<std::string, OptionList>> SectionList;
	SectionList sections;

	// Given an option name, we want to find the option quickly.
	typedef std::map<std::string, Option*> NameToOption;
	NameToOption names;

	typedef std::vector<Observer*> Observers;
	Observers observers;
};

} // namespace options
