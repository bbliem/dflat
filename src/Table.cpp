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
#include <cassert>

#include "Table.h"
#include "Application.h"

#ifndef NDEBUG
#include <iostream>
#endif

// IMPORTANT: Change this whenever attributes of Row change
bool RowComparator::operator()(const RowPtr& lhs, const RowPtr& rhs)
{
	// TODO optimize
	// TODO Play with different orders of comparisons to find out which is best
	// XXX can we avoid this additional comparison with a three-way comparison?
	if(lhs->getSelectedEdges() < rhs->getSelectedEdges())
	   return true;
	if(rhs->getSelectedEdges() < lhs->getSelectedEdges())
	   return false;
	if(lhs->getHasForgottenComponent() < rhs->getHasForgottenComponent())
	   return true;
	if(rhs->getHasForgottenComponent() < lhs->getHasForgottenComponent())
	   return false;
	if(lhs->getSelectedVertices() < rhs->getSelectedVertices())
		return true;
	if(rhs->getSelectedVertices() < lhs->getSelectedVertices())
		return false;
	if(lhs->getConnectedViaSelectedEdges() < rhs->getConnectedViaSelectedEdges())
		return true;
	return false;
}

Rows::const_iterator Table::add(RowPtr&& row)
{
	assert(row);
	std::pair<Rows::iterator, bool> result = rows.insert(std::move(row));
	// XXX If an equivalent element already exists in "children", it is unclear to me whether "subtree" is actually moved or not. (Maybe it depends on the implementation?)
	// For the time being, pray that it isn't moved in such a case.
	// http://stackoverflow.com/questions/10043716/stdunordered-settinsertt-is-argument-moved-if-it-exists

// TODO Add optimum cost. When creating a table, set it to "infinity":
//if(!app.isOptimizationDisabled())
//    optimumCost = std::numeric_limits<decltype(optimumCost)>::max());

	if(!result.second) {
		assert(row);
		const RowPtr& origRow = *result.first;
		const unsigned long oldCost = origRow->getCost();
		if(oldCost > row->getCost())
			*origRow = *row;
		else if(oldCost < row->getCost())
			return rows.end();
		else {
			assert(origRow->getCost() == oldCost);
			// TODO If we don't need to enumerate all optimal solutions, we could avoid merging.
			origRow->merge(std::move(*row));
			return rows.end();
		}
	}
	return result.first;
}

//void Table::printExtensions(std::ostream& os) const
//{
//    std::unique_ptr<ExtensionIterator> it(new ExtensionIterator(*node, parent));
//
//    while(it->isValid()) {
//        std::string childIndent = indent;
//        os << indent;
//        ItemTreeNode::Items items = std::move(**it);
//
//        // XXX All this ugly stuff just for pretty printing... (Otherwise ExtensionIterator would not need to be copyable and it might use unique_ptr instead of shared_ptr internally.)
//        std::unique_ptr<ExtensionIterator> currentIt(new ExtensionIterator(*it));
//        ++*it;
//
//        if(!root) {
//            if(lastChild && !it->isValid()) {
//#ifndef NO_UNICODE
//                os << "┗━ ";
//                childIndent += "   ";
//#else
//                os << "\\- ";
//                childIndent += "   ";
//#endif
//            }
//            else {
//#ifndef NO_UNICODE
//                os << "┣━ ";
//                childIndent += "┃  ";
//#else
//                os << "|- ";
//                childIndent += "|  ";
//#endif
//            }
//        }
//
//        // We only print or count children with cost equal to this node's
//        std::vector<Table*> bestChildren;
//        bestChildren.reserve(children.size());
//        for(const auto& child : children)
//            if(child->node->getCost() == node->getCost())
//                bestChildren.push_back(child.get());
//        assert(children.empty() || bestChildren.empty() == false);
//
//        // When limiting the depth causes children not to be extended, print the number of accepting children (with optimum cost)
//        if(maxDepth == 0 && children.empty() == false) {
//            os << '[';
//            if(!printCount)
//                os << ">=";
//            mpz_class count;
//            assert(count == 0);
//            // On the first level we can use the counts inside the nodes
//            // XXX This redundancy is a bit ugly but maybe offers better performance than recalculating the number of accepting children (like below in the "else" branch).
//            if(!parent) {
//                for(const auto& child : bestChildren)
//                    count += child->node->getCount();
//            }
//            else {
//                for(const auto& child : bestChildren)
//                    count += child->node->countExtensions(*currentIt);
//            }
//            os << count << "] ";
//        }
//
//        for(const auto& item : items)
//            os << item << ' ';
//
//        // Print cost
//        if(node->getCost() != 0 && (maxDepth == 0 || children.empty()))
//            os << "(cost: " << node->getCost() << ')';
//
//        os << std::endl;
//
//        if(maxDepth > 0) {
//            size_t i = 0;
//            for(const auto& child : bestChildren)
//                child->printExtensions(os, maxDepth - 1, printCount, false, ++i == bestChildren.size(), childIndent, currentIt.get());
//        }
//    }
//}

void Table::printWithNames(std::ostream& os, const std::vector<unsigned>& names) const
{
	for(const auto& row : rows) {
		row->printWithNames(os, names);
		os << std::endl;
	}
}

#ifndef NDEBUG
void Table::printDebug() const
{
	printWithNames(std::cout, {});
}
#endif
