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
#include <iostream>
#include <iomanip>

#include "Progress.h"
#include "../Application.h"
#include "../Decomposition.h"
#include "../DecompositionNode.h"

namespace {

const char frames[] = { '|', '/', '-', '\\' };
static const std::chrono::milliseconds minFrameDuration(100); // display each frame for at least this amount of time

} // anonymous namespace

namespace printer {

Progress::Progress(Application& app, bool newDefault)
	: Printer(app, "progress", "Progress report", newDefault)
	, nodeCounter(1)
	, curFrame(0)
	, lastIncrement(std::chrono::steady_clock::now())
{
}

void Progress::decomposerResult(const Decomposition& result)
{
	::Printer::decomposerResult(result);
	// XXX This is a dirty hack to get the number of decomposition nodes quickly
	totalNodes = DecompositionNode({}).getGlobalId() - 1;
}

void Progress::solverInvocationResult(const Decomposition& decompositionNode, const ItemTree* result)
{
	if(nodeCounter < totalNodes)
		++nodeCounter;
}

bool Progress::listensForSolverEvents() const
{
	return true;
}

void Progress::solverEvent(const std::string& msg)
{
	const auto now = std::chrono::steady_clock::now();
	if(now - lastIncrement >= minFrameDuration) {
		lastIncrement = now;
		++curFrame;
		assert(sizeof(frames) / sizeof(frames[0]) == 4);
		curFrame %= 4;
		printProgress();
	}
}

void Progress::result(const ItemTreePtr& rootItemTree)
{
	std::cout << "\r                                                        \r" << std::flush;


	Printer::result(rootItemTree);
}

void Progress::enterNode(const Decomposition& decompositionNode)
{
	computationStack.push(&decompositionNode);
	printProgress();
}

void Progress::leaveNode()
{
	computationStack.pop();
	if(!computationStack.empty())
		printProgress();
}

void Progress::printProgress()
{
	static const int digits = std::to_string(totalNodes).length();
	assert(!computationStack.empty());
	const Decomposition& top = *computationStack.top();
	const auto curNode = top.getNode().getGlobalId();
	const auto numChildren = top.getChildren().size();
	const auto bagSize = top.getNode().getBag().size();
	std::cout << '\r'
		<< "Node " << std::setw(digits) << curNode << ", "
		<< std::setw(2) << bagSize << " elements, "
		<< std::setw(2) << numChildren << " child nodes.  "
		<< frames[curFrame] << ' '
		<< std::setw(digits) << nodeCounter << '/' << totalNodes << ' '
		<< std::flush;
}

} // namespace printer
