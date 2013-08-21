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
#include <cassert>

#include "OptionHandler.h"
#include "ValueOption.h"


namespace options {

const OptionHandler::Section OptionHandler::GENERAL_SECTION = "General options";

void OptionHandler::add(Option& opt, const Section& section)
{
	assert(names.count(opt.getName()) == 0);
	names[opt.getDashedName()] = &opt;

	for(SectionList::iterator it = sections.begin(); it != sections.end(); ++it) {
		if(it->first == section) {
			it->second.push_back(&opt);
			return;
		}
	}

	// Section does not exist yet
	sections.push_back(std::make_pair(section, OptionList(1, &opt)));
}

void OptionHandler::parse(int argc, char** argv)
{
	for(int i = 1; i < argc; ++i) {
		std::string word = argv[i];

		for(NameToOption::iterator it = names.begin(); it != names.end(); ++it) {
			// Does "it" point to an option with matching name?
			if(it->first == word) {
				// Does the option take a value?
				if(dynamic_cast<ValueOption*>(it->second) != 0) {
					++i;
					if(i == argc) {
						std::ostringstream msg;
						msg << "Option '" << it->second->getName() << "' requires an argument.";
						throw std::runtime_error(msg.str());
					}
					word = argv[i];
					dynamic_cast<ValueOption*>(it->second)->setValue(word);
				}
				else {
					// It is not a ValueOption, therefore it must be a flag (i.e., not take an option)
					it->second->setUsed();
				}

				goto nextOption;
			}
		}

		// If this point is reached, none of the options matched
		{
			std::ostringstream msg;
			msg << "Unknown option '" << word << '\'';
			throw std::runtime_error(msg.str());
		}

nextOption:
		;
	}
}

void OptionHandler::printHelp() const
{
	for(SectionList::const_iterator it = sections.begin(); it != sections.end(); ++it) {
		std::cerr << std::endl << it->first << ':' << std::endl << std::endl;
		for(OptionList::const_iterator oit = it->second.begin(); oit != it->second.end(); ++oit)
			(*oit)->printHelp();
		std::cerr << std::endl;
	}
}

} // namespace options
