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

#pragma once
//}}}
#include <ostream>
#include <vector>
#include <algorithm>
#include <cassert>

// Using STL containers with incomplete types is forbidden.
// This is why the template parameter Cs is expected to be a container of smart pointers.
// See also the solutions on:
// http://stackoverflow.com/questions/9860503/templated-recursive-data-types
// See also:
// http://stackoverflow.com/questions/8452566/why-cant-i-replace-stdmap-with-stdunordered-map
// http://stackoverflow.com/questions/11438969/c-struct-that-contains-a-map-of-itself
// In particular, look at bdonlan's answer in http://stackoverflow.com/questions/6527917/how-can-i-emulate-a-recursive-type-definition-in-c

template <typename N, typename Cs>
class DirectedAcyclicGraph
{
public:
	typedef N Node;
	typedef Cs Children;
	typedef typename Children::value_type ChildPtr;
	// Parents are not managed by smart pointers, as this would lead to cycles
	typedef std::vector<DirectedAcyclicGraph*> Parents;

	DirectedAcyclicGraph(Node&& leaf) : node(std::move(leaf)) {}

	// When moving a DirectedAcyclicGraph, we must set the parent pointer of each child to the new address.
	// Make sure that you don't have any other pointers (not managed by this class) pointing to the old one. (I.e., be careful with extension pointers, for instance.)
	DirectedAcyclicGraph(DirectedAcyclicGraph&& other)
		: node(std::move(other.node))
		, children(std::move(other.children))
//		, parents(std::move(other.parents)) // XXX does this make sense?
	{
		for(auto& child : children)
			std::replace(child->parents.begin(), child->parents.end(), &other, this);
	}

	DirectedAcyclicGraph& operator=(DirectedAcyclicGraph&& other)
	{
		assert(this != &other);
		node = std::move(other.node);
		children = std::move(other.children);
//		parents = std::move(other.parents); // XXX does this make sense?
		for(auto& child : children)
			std::replace(child->parents.begin(), child->parents.end(), &other, this);
		return *this;
	}


	DirectedAcyclicGraph(const DirectedAcyclicGraph&) = delete;

	const Node& getRoot() const { return node; }
	const Parents& getParents() const { return parents; }
	const Children& getChildren() const { return children; }

	// Adds the root of "child" to the list of children. Takes ownership of the whole subgraph rooted at "child".
	// Make sure there arise no cycles!
	// Inserts this node into the new child's list of parents.
	void addChild(ChildPtr&& child)
	{
		assert(child);
		child->parents.push_back(this);
		children.insert(children.end(), std::move(child));
	}

	// Print decomposition (multiple lines, with EOL at the end)
	friend std::ostream& operator<<(std::ostream& os, const DirectedAcyclicGraph& tree)
	{
		tree.print(os);
		return os;
	}

protected:
	void print(std::ostream& os, bool root = true, bool last = false, std::string indent = "") const
	{
		os << indent;

		if(!root) {
			if(last) {
#ifndef NO_UNICODE
				os << "┗━ ";
				indent += "   ";
#else
				os << "\\- ";
				indent += "   ";
#endif
			}
			else {
#ifndef NO_UNICODE
				os << "┣━ ";
				indent += "┃  ";
#else
				os << "|- ";
				indent += "|  ";
#endif
			}
		}

		os << node << std::endl;

		size_t i = 0;
		for(const auto& child : children)
			child->print(os, false, ++i == children.size(), indent);
	}

	Node node;
	Children children;
	Parents parents;
};
