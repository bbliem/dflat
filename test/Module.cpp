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
//}}}
#include <gtest/gtest.h>

#include "Application.h"
#include "Module.h"

class StubModule : public Module
{
public:
	int timesSelected = 0;
	int timesNotified = 0;
	static const char* MODULE_NAME;

	StubModule(Application& app, options::Choice& choice)
	: Module(app, choice, MODULE_NAME, "module description")
	{
	}

	virtual void select() override
	{
		Module::select();
		++timesSelected;
	}

	virtual void notify() override
	{
		Module::notify();
		++timesNotified;
	}
};

const char* StubModule::MODULE_NAME = "module name";

struct ModuleTest : public ::testing::Test
{
	Application app{"test"};
	options::Choice choice{"module", "m", "'--module m' selects module <m>"};
	StubModule module{app, choice};

	ModuleTest()
	{
		app.getOptionHandler().addOption(choice);
	}
};

TEST_F(ModuleTest, ObservesOptionHandler)
{
	const char* argv[] = {"--module", StubModule::MODULE_NAME};
	app.getOptionHandler().parse(sizeof(argv)/sizeof(argv[0]), argv);
	EXPECT_EQ(1, module.timesNotified);
}

TEST_F(ModuleTest, NotifyCallsSelectIfChosen)
{
	EXPECT_NE(choice.getValue(), StubModule::MODULE_NAME);
	module.notify();
	EXPECT_EQ(0, module.timesSelected);
	choice.setValue(StubModule::MODULE_NAME);
	module.notify();
	EXPECT_EQ(1, module.timesSelected);
}
