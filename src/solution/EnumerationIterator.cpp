/*
Copyright 2012, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dynasp/dflat/>.

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

#include "EnumerationIterator.h"

namespace solution {

EnumerationIterator::EnumerationIterator()
	: plan(0), left(0), right(0)
{
}

EnumerationIterator::EnumerationIterator(const EnumerationPlan& p)
	: plan(&p), left(0), right(0)
{
	switch(p.getOperation()) {
		case sharp::Plan::LEAF:
			assert(!p.getLeft() && !p.getRight());
			break;
		case sharp::Plan::UNION:
			assert(p.getLeft() && p.getRight());
			left = new EnumerationIterator(*p.getLeft());
			right = new EnumerationIterator(*p.getRight());
			break;
		case sharp::Plan::JOIN:
			assert(p.getLeft());
			left = new EnumerationIterator(*p.getLeft());
			if(p.getRight())
				right = new EnumerationIterator(*p.getRight());
			break;
	}
	materializeAssignment();
}

EnumerationIterator::~EnumerationIterator()
{
	delete left;
	delete right;
}

bool EnumerationIterator::valid() const
{
	return plan != 0;
}

inline void EnumerationIterator::materializeAssignment()
{
	assert(plan);
	assignment = plan->getAssignment();
	switch(plan->getOperation()) {
		case sharp::Plan::LEAF:
			assert(!left && !right);
			break;
		case sharp::Plan::UNION:
			assert(left && right);
			assert(plan->getLeft() && plan->getRight());
			assert(left->valid() || right->valid());
			if(left->valid())
				assignment.insert((**left).begin(), (**left).end());
			else
				assignment.insert((**right).begin(), (**right).end());
			break;
		case sharp::Plan::JOIN:
			assert(left);
			assert(plan->getLeft());
			assert(left->valid());
			assignment.insert((**left).begin(), (**left).end());
			if(right) { // Join
				assert(plan->getRight());
				assert(right->valid());
				assignment.insert((**right).begin(), (**right).end());
			}
			break;
	}
}

const Tuple::Assignment& EnumerationIterator::operator*() const
{
	assert(valid());
	return assignment;
}

EnumerationIterator& EnumerationIterator::operator++()
{
	assert(plan);
	switch(plan->getOperation()) {
		case sharp::Plan::LEAF:
			assert(!left && !right);
			plan = 0; // invalidate
			return *this;
		case sharp::Plan::UNION:
			assert(left && right);
			assert(plan->getLeft() && plan->getRight());
			assert(right->valid());
			if(left->valid())
				++(*left);
			else {
				++(*right);
				if(!right->valid()) {
					plan = 0;
					return *this;
				}
			}
			break;
		case sharp::Plan::JOIN:
			assert(left);
			assert(plan->getLeft());
			assert(left->valid());
			if(right) {
				//Join
				assert(plan->getRight());
				assert(right->valid());
				++(*right);
				if(!right->valid()) {
					++(*left);
					if(!left->valid()) {
						plan = 0;
						return *this;
					}
					delete right;
					right = new EnumerationIterator(*plan->getRight());
					assert(right->valid());
				}
			} else {
				// Extension without join
				++(*left);
				if(!left->valid()) {
					plan = 0;
					return *this;
				}
			}
			break;
	}
	materializeAssignment();

	return *this;
}

} // namespace solution
