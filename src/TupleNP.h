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

#include "Tuple.h"

class TupleNP : public Tuple
{
public:
	TupleNP();

	virtual bool operator<(const sharp::Tuple&) const;
	virtual bool operator==(const sharp::Tuple&) const;
	virtual void unify(const sharp::Tuple& old);

	virtual bool matches(const Tuple& other) const;
	virtual TupleNP* join(const Tuple& other) const;
	virtual void declare(std::ostream& out, const sharp::TupleTable::value_type& tupleAndSolution, const char* predicateSuffix) const;
	virtual const Assignment& getAssignment() const;
	virtual unsigned int getCurrentCost() const;
	virtual unsigned int getCost() const;

#ifdef VERBOSE
	virtual void print(std::ostream&) const;
#endif

	Assignment assignment;
	// TODO: We might distinguish tuples with cost information from those without, but OTOH the memory consumption should not be that critical
	unsigned int currentCost;
	unsigned int cost;
};
