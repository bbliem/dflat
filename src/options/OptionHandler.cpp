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

const std::string OptionHandler::GENERAL_SECTION = "General options";

OptionHandler::OptionHandler()
{
	// Add the general section without any options before any other sections will be added (to make sure the general section is always shown first)
	sections.push_back(std::make_pair(GENERAL_SECTION, OptionList()));
}

void OptionHandler::addOption(Option& opt, const std::string& section)
{
	assert(names.count(opt.getDashedName()) == 0);
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

				// Mark the option as used
				it->second->setUsed();

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

	// Notify observers
	for(Observers::iterator it = observers.begin(); it != observers.end(); ++it)
		(*it)->notify();

	// Check all conditions
	for(NameToOption::const_iterator it = names.begin(); it != names.end(); ++it)
		it->second->checkConditions();
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

void OptionHandler::registerObserver(Observer& observer)
{
	observers.push_back(&observer);
}

} // namespace options
