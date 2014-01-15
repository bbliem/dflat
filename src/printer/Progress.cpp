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
#include <iomanip>

#include "Progress.h"
#include "../Application.h"
#include "../DecompositionNode.h"

namespace {

const char frames[] = { '|', '/', '-', '\\' };

} // anonymous namespace

namespace printer {

Progress::Progress(Application& app, bool newDefault)
	: Printer(app, "progress", "Progress report", newDefault)
	, curNode(1)
	, curFrame(0)
{
}

void Progress::decomposerResult(const Decomposition&)
{
	// XXX This is a dirty hack to get the number of decomposition nodes quickly
	totalNodes = DecompositionNode({}).getGlobalId() - 1;
	printProgress();
}

void Progress::solverInvocationResult(const DecompositionNode& decompositionNode, const ItemTree* result)
{
	++curNode;
	printProgress();
}

bool Progress::listensForSolverEvents() const
{
	return true;
}

void Progress::solverEvent(const std::string& msg)
{
	++curFrame;
	assert(sizeof(frames) / sizeof(frames[0]) == 4);
	curFrame %= 4;
	printProgress();
}

void Progress::result(const ItemTreePtr& rootItemTree)
{
	std::cout << '\r' << std::flush;
	Printer::result(rootItemTree);
}

void Progress::printProgress()
{
	static const int digits = std::to_string(totalNodes).length();
	std::cout << '\r' << std::setw(digits) << curNode << '/' << totalNodes << ' ' << frames[curFrame] << std::flush;
}

} // namespace printer
