/*{{{
Copyright 2012-2015, Bernhard Bliem
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
#include <sstream>

#include "Performance.h"
#include "../Application.h"
#include "../Decomposition.h"
#include "../DecompositionNode.h"

namespace printer {

Performance::Performance(Application& app, bool newDefault)
	: Printer(app, "performance", "Performance measurement", newDefault)
{
}

void Performance::solverInvocationResult(const Decomposition& decompositionNode, const ItemTree* result)
{
	assert(&decompositionNode == computationStack.top());
	NodeData& data = nodeData[&decompositionNode];
	data.itemTreeSize = result ? result->estimateSize() : 0;
}

bool Performance::listensForSolverEvents() const
{
	return false;
}

void Performance::result(const ItemTreePtr& rootItemTree)
{
}

void Performance::enterNode(const Decomposition& decompositionNode)
{
	computationStack.push(&decompositionNode);
	NodeData& data = nodeData[&decompositionNode];
	data.startTime = std::chrono::high_resolution_clock::now();
}

void Performance::leaveNode()
{
	const Decomposition* leftNode = computationStack.top();
	computationStack.pop();
	NodeData& data = nodeData[leftNode];
	data.elapsedTime += std::chrono::duration_cast<decltype(data.elapsedTime)>(std::chrono::high_resolution_clock::now() - data.startTime);

	if(computationStack.empty()) {
		std::cout
			<< "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
			<< "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\"" << std::endl
			<< "         xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << std::endl
			<< "         xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns "
			   "http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">" << std::endl
			<< "  <key id=\"time\" for=\"node\"/>" << std::endl
			<< "  <key id=\"item-tree-size\" for=\"node\"/>" << std::endl
			<< "  <graph edgedefault=\"undirected\">" << std::endl;

		printGraphMlElements(std::cout, leftNode);

		std::cout
			<< "  </graph>" << std::endl
			<< "</graphml>" << std::endl;
	}
}

void Performance::printGraphMlElements(std::ostream& out, const Decomposition* node) const
{
	const auto it = nodeData.find(node);
	if(it == nodeData.end()) {
		std::ostringstream msg;
		msg << "No performance data collected for decomposition node " << node->getNode().getGlobalId();
		throw std::runtime_error(msg.str());
	}
	const NodeData& data = it->second;
	out << "    <node id=\"n" << node->getNode().getGlobalId() << "\">" << std::endl;
	out << "      <data key=\"time\">" << data.elapsedTime.count() << "</data>" << std::endl;
	out << "      <data key=\"item-tree-size\">" << data.itemTreeSize << "</data>" << std::endl;
	out << "    </node>" << std::endl;

	for(const auto& child : node->getChildren()) {
		printGraphMlElements(out, child.get());
		out << "    <edge source=\"n" << node->getNode().getGlobalId() << "\""
		                " target=\"n" << child->getNode().getGlobalId() << "\"/>" << std::endl;
	}
}

} // namespace printer
