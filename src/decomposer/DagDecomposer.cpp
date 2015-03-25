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
#include <cassert>
#include <stack>
#include <list>

//#include <iostream> // XXX

#include "DagDecomposer.h"
#include "../Hypergraph.h"
#include "../Decomposition.h"
#include "../Application.h"

namespace {

class DirectedGraph
{
public:
	typedef Hypergraph::Vertex Vertex;
	typedef std::list<Vertex> Children;

	DirectedGraph(const Hypergraph& hg)
	{
		for(const auto& v : hg.getVertices())
			adjacencyList[v];
		for(const auto& e : hg.getEdges()) {
			if(e.size() != 2)
				throw std::runtime_error("Tried to build graph from non-binary hypergraph");
			const auto& from = e[0];
			const auto& to = e[1];
			adjacencyList[from].push_back(to);
			inverseAdjacencyList[to].push_back(from);
		}
	}

	const Children& getChildren(const Vertex& v) const
	{
		return adjacencyList.at(v);
	}

	const Children& getParents(const Vertex& v) const
	{
		return inverseAdjacencyList.at(v);
	}

	// Adds to both result and traversed each vertex u such that there is a path from u to v that contains no vertex from traversed.
	void getAncestors(const Vertex& v, std::set<Vertex>& result, std::set<Vertex>& traversed) const
	{
		if(traversed.find(v) == traversed.end()) {
			result.insert(v);
			traversed.insert(v);

			for(const auto& ancestor : inverseAdjacencyList.at(v))
				getAncestors(ancestor, result, traversed);
		}
	}

private:
	std::map<Vertex, Children> adjacencyList;
	std::map<Vertex, Children> inverseAdjacencyList;
};

//class TdDag;
//typedef std::shared_ptr<TdDag> TdDagPtr;
//
//// Nodes of a TdDag object are tree decompositions
//class TdDag : public DirectedAcyclicGraph<DecompositionPtr, std::vector<TdDagPtr>>
//{
//public:
//	using DirectedAcyclicGraph::DirectedAcyclicGraph;
//};

void updateTopmostNodes(std::map<Hypergraph::Vertex, DecompositionPtr>& topmostNodeContaining, const DecompositionPtr& td)
{
	for(const auto& bagElement : td->getNode().getBag())
		topmostNodeContaining.emplace(bagElement, td);
	for(const auto& child : td->getChildren())
		updateTopmostNodes(topmostNodeContaining, child);
}

}

namespace decomposer {

DagDecomposer::DagDecomposer(TreeDecomposer& treeDecomposer, Application& app, bool newDefault)
	: Decomposer(app, "dag", "DAG decomposition (connecting TDs of SCCs)", newDefault)
	, treeDecomposer(treeDecomposer)
{
}

void DagDecomposer::select()
{
	::Decomposer::select();
	treeDecomposer.setSelectedCondition(); // XXX ugly
}

DecompositionPtr DagDecomposer::decompose(const Hypergraph& instance) const
{
	// Kosaraju's algorithm
	using Vertex = Hypergraph::Vertex;
	std::stack<Vertex> traversal1Result;

	DirectedGraph graph(instance);

	// Traversal 1
	std::set<Vertex> remaining = instance.getVertices();

	while(remaining.empty() == false) {
		// Pick any vertex to start with DFS
		std::stack<Vertex> dfsStack;
		Vertex start = *remaining.begin();
		dfsStack.push(start);
		remaining.erase(start);
		while(dfsStack.empty() == false) {
			Vertex v = dfsStack.top();
			// Push all non-visited children of v onto stack
			bool pushedChild = false;
			for(const auto& child : graph.getChildren(v)) {
				if(remaining.find(child) != remaining.end()) {
					pushedChild = true;
					dfsStack.push(child);
					remaining.erase(child);
				}
			}
			if(!pushedChild) {
				dfsStack.pop();
				traversal1Result.push(v);
			}
		}
	}
	assert(traversal1Result.size() == instance.getVertices().size());

	// Traversal 2
	//TdDag tdDag;
	std::map<Vertex, DecompositionPtr> topmostNodeContaining; // store for each vertex the topmost TD node whose bag contains that vertex
	DecompositionPtr result(new Decomposition({{}}, app.getSolverFactory()));
	result->setRoot();
	// Create a TD for each SCC and add the root of this TD as a child to the node with empty bag in result

	std::set<Vertex> visited;
	while(traversal1Result.empty() == false) {
		Vertex v = traversal1Result.top();
		traversal1Result.pop();
		std::set<Vertex> sccVertices;
		graph.getAncestors(v, sccVertices, visited);
		while(traversal1Result.empty() == false && sccVertices.find(traversal1Result.top()) != sccVertices.end())
			traversal1Result.pop();
		// Create SCC
		Hypergraph scc;
		for(const auto& v : sccVertices) {
			scc.addVertex(v);
			for(const auto& child : graph.getChildren(v)) {
				if(sccVertices.find(child) != sccVertices.end())
					scc.addEdge({v, child});
			}
		}

//		std::cout << "SCC:\n";
//		for(const auto& v : scc.getVertices())
//			std::cout << v << ' ';
//		std::cout << '\n';
//		std::cout << "Edges:\n";
//		for(const auto& e : scc.getEdges()) {
//			for(const auto& v : e)
//				std::cout << v << ' ';
//			std::cout << '\n';
//		}
//		std::cout << '\n';

		// FIXME Workaround due to bug in SHARP
		if(scc.getEdges().empty()) {
			assert(scc.getVertices().size() == 1);
			scc.addEdge({*scc.getVertices().begin()});
		}

		// Decompose SCC
		//result->addChild(treeDecomposer.decompose(scc));
		DecompositionPtr td = treeDecomposer.decompose(scc);
		updateTopmostNodes(topmostNodeContaining, td);
		result->addChild(std::move(td));

		// Find out which edges exist between this SCC and other already generated ones
		for(const auto& v : sccVertices) {
			for(const auto& parent : graph.getParents(v)) {
				if(sccVertices.find(parent) == sccVertices.end()) {
					// SCC-crossing edge from parent to v
					DecompositionPtr interfaceNode(new Decomposition({{parent, v}}, app.getSolverFactory()));
					// Add interfaceNode to the parents of the topmost node containing v
					interfaceNode->addChild(topmostNodeContaining.at(v));
					// Add interfaceNode to children of the topmost node containing parent
					topmostNodeContaining.at(parent)->addChild(std::move(interfaceNode));
				}
			}
		}
	}
	assert(visited.size() == instance.getVertices().size());

	// TODO Cover edges between SCCs and ensure connectedness

	return result;
}

} // namespace decomposer
