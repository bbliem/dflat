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
#include <stdexcept>
#include <cassert>

#include "OptionHandler.h"
#include "ValueOption.h"
#include "Observer.h"

namespace options {

const std::string OptionHandler::GENERAL_SECTION = "General options";

OptionHandler::OptionHandler()
{
	// Add the general section without any options before any other sections will be added (to make sure the general section is always shown first)
	sections.emplace_back(GENERAL_SECTION, OptionList());
}

void OptionHandler::addOption(Option& opt, const std::string& section)
{
	assert(names.count(opt.getDashedName()) == 0);
	names[opt.getDashedName()] = &opt;

	for(auto& sec : sections) {
		if(sec.first == section) {
			sec.second.push_back(&opt);
			return;
		}
	}

	// Section does not exist yet
	sections.emplace_back(section, OptionList(1, &opt));
}

void OptionHandler::parse(int argc, char** argv)
{
	for(int i = 1; i < argc; ++i) {
		std::string word = argv[i];

		for(auto& opt : names) {
			// Is it an option with matching name?
			if(opt.first == word) {
				// Does the option take a value?
				if(dynamic_cast<ValueOption*>(opt.second) != 0) {
					++i;
					if(i == argc) {
						std::ostringstream msg;
						msg << "Option '" << opt.second->getName() << "' requires an argument.";
						throw std::runtime_error(msg.str());
					}
					word = argv[i];
					dynamic_cast<ValueOption*>(opt.second)->setValue(word);
				}

				// Mark the option as used
				opt.second->setUsed();

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
	for(Observer* o : observers)
		o->notify();

	// Check all conditions
	for(const auto& o : names)
		o.second->checkConditions();
}

void OptionHandler::printHelp() const
{
	for(const auto& sec : sections) {
		std::cerr << std::endl << sec.first << ':' << std::endl << std::endl;
		for(const Option* o : sec.second)
			o->printHelp();
		std::cerr << std::endl;
	}
}

void OptionHandler::registerObserver(Observer& observer)
{
	observers.push_back(&observer);
}

} // namespace options
