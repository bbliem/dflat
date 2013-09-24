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

#include "SingleValueOption.h"

namespace options {

class Choice : public SingleValueOption
{
public:
	Choice(const std::string& name, const std::string& placeholder, const std::string& description);

	//! This should only be called before setValue() has been called
	void addChoice(const std::string& choiceName, const std::string& description, bool newDefault = false);

	virtual void setValue(const std::string& v) override;
	virtual void checkConditions() const override;
	virtual void printHelp() const override;

private:
	struct Possibility
	{
		Possibility(const std::string& name, const std::string& description) : name(name), description(description) {}
		std::string name;
		std::string description;
	};

	typedef std::vector<Possibility> Possibilities;

	static const int POSSIBILITY_NAME_WIDTH = 18;

	Possibilities possibilities;
	std::string defaultValue;
};

} // namespace options
