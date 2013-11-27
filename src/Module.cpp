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
#include <string>

#include "Module.h"
#include "Application.h"
#include "options/Choice.h"

Module::Module(Application& app, options::Choice& choice, const std::string& optionName, const std::string& optionDescription, bool newDefault)
	: app(app)
	, choice(choice)
	, optionName(optionName)
{
	choice.addChoice(optionName, optionDescription, newDefault);
	app.getOptionHandler().registerObserver(*this);
}

void Module::select()
{
	selected.setSatisfied();
}

void Module::notify()
{
	if(choice.getValue() == optionName)
		select();
}
