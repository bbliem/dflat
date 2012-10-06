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

#include "Row.h"

class RowNP : public Row
{
public:
	RowNP();

	virtual bool operator<(const sharp::Row&) const;
	virtual bool operator==(const sharp::Row&) const;
	virtual void unify(const sharp::Row& old);

	virtual bool matches(const Row& other) const;
	virtual RowNP* join(const Row& other) const;
	virtual void declare(std::ostream& out, const sharp::Table::value_type& rowAndSolution, unsigned childNumber) const;
	virtual void declare(std::ostream& out, const sharp::Table::value_type& rowAndSolution, const char* predicateName = "childRow") const;
	virtual const Items& getItems() const;
	virtual unsigned int getCurrentCost() const;
	virtual unsigned int getCost() const;

#ifdef PRINT_COMPUTED_ROWS
	virtual void print(std::ostream&) const;
#endif

	Items items;
	// TODO: We might distinguish rows with cost information from those without, but OTOH the memory consumption should not be that critical
	unsigned int currentCost;
	unsigned int cost;
	
private:
	void declareRowExceptName(std::ostream& out, const std::string& rowName) const;
};
