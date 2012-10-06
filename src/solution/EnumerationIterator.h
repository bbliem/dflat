
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

#pragma once

#include <sharp/main>

#include "../Tuple.h"
#include "EnumerationPlan.h"

namespace solution {

class EnumerationPlan;

// TODO: Maybe make it a "proper" iterator? (E.g., use Boost Iterator?)
class EnumerationIterator : public sharp::Solution
{
public:
	EnumerationIterator();
	explicit EnumerationIterator(const EnumerationPlan& p);
	virtual ~EnumerationIterator();

	//! @return true iff this iterator can be dereferenced, i.e., *this != plan->end()
	bool valid() const;

	const Tuple::Assignment& operator*() const; // dereference
	EnumerationIterator& operator++();

private:
	// Materialize what will be returned upon dereferencing
	void materializeAssignment();

	const EnumerationPlan* plan;
	EnumerationIterator* left;
	EnumerationIterator* right;

	Tuple::Assignment assignment; // Materialized assignment returned when dereferencing
};

} // namespace solution
