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

#include "Observer.h"

class Application;

namespace options {

class Option;

// Observe the option handler. When notified, the HelpObserver checks whether the help option was given and, if so, prints the usage message and exits (with code 0).
class HelpObserver : public Observer
{
public:
	HelpObserver(const Application& app, const Option& help);

	virtual void notify() override;

private:
	const Application& app;
	const options::Option& help;
};

} // namespace options
