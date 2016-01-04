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
#include "Decomposition.h"
#include "SolverFactory.h"

Decomposition::Decomposition(Node&& leaf, const SolverFactory& solverFactory)
	: DirectedAcyclicGraph(std::move(leaf))
	, solverFactory(solverFactory)
	, root(false)
	, postJoinNode(false)
{
}

Solver& Decomposition::getSolver()
{
	if(!solver)
		solver = solverFactory.newSolver(*this);

	return *solver;
}

bool Decomposition::isJoinNode() const
{
	return children.size() > 1 && std::all_of(children.begin(), children.end(), [&](const ChildPtr& child) {
			return child->getNode().getBag() == node.getBag();
	});
}

void Decomposition::setRoot(bool root)
{
	this->root = root;
}

bool Decomposition::isRoot() const
{
	return root;
}

void Decomposition::setPostJoinNode(bool postJoinNode)
{
	this->postJoinNode = postJoinNode;
}

bool Decomposition::isPostJoinNode() const
{
	return postJoinNode;
}

int Decomposition::getWidth() const
{
	int width = node.getBag().size() - 1;
	for(const auto& child : children)
		width = std::max(child->getWidth(), width);
	return width;
}

void Decomposition::printGraphMl(std::ostream& out) const
{
	out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
		<< "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\"" << std::endl
		<< "         xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << std::endl
		<< "         xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns "
		   "http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">" << std::endl
		<< "  <key id=\"bag\" for=\"node\"/>" << std::endl
		<< "  <graph edgedefault=\"undirected\">" << std::endl;

	printGraphMlElements(out);

	out << "  </graph>" << std::endl
	    << "</graphml>" << std::endl;
}

void Decomposition::printGraphMlElements(std::ostream& out) const
{
	out << "    <node id=\"n" << node.getGlobalId() << "\">" << std::endl;
	out << "      <data key=\"bag\">";
	std::string separator;
	for(const auto& vertex : node.getBag()) {
		out << separator << vertex;
		separator = ", ";
	}
	out << "</data>" << std::endl;
	out << "    </node>" << std::endl;

	for(const auto& child : children) {
		child->printGraphMlElements(out);
		out << "    <edge source=\"n" << node.getGlobalId() << "\""
		                " target=\"n" << child->node.getGlobalId() << "\"/>" << std::endl;
	}
}
