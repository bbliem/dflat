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

#include <string>

#include "options/Observer.h"
#include "options/Condition.h"

class Application;
namespace options {
	class Choice;
}

class Module : public options::Observer
{
public:
	Module(Application& app, options::Choice& choice, const std::string& optionName, const std::string& optionDescription, bool newDefault = false);

	virtual ~Module() {};

	virtual void notify();

	// Called when this module has been selected using the choice option given to the constructor
	virtual void select();

protected:
	Application& app;
	options::Choice& choice;
	std::string optionName;
	options::Condition selected; // Use this when the module adds custom options that may only be used when it is selected. The condition is set to satisfied in select().
};
