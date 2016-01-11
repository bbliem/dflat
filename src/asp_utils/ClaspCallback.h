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

#pragma once
//}}}
#include <clasp/clasp_facade.h>

#include "GringoOutputProcessor.h"

class Application;

namespace asp_utils {

// Gets called by clasp whenever a model has been found
// Contains an item tree
class ClaspCallback : public Clasp::EventHandler
{
public:
	ClaspCallback(const Application&);

	// Call this after all answer sets have been processed. It returns the resulting item tree (and calls finalize() on it).
	virtual ItemTreePtr finalize(bool pruneUndefined, bool pruneRejecting);

	// Called when a model has been found
	virtual bool onModel(const Clasp::Solver&, const Clasp::Model&) override;

	// Returns the item tree that is being constructed by this object.
	// You may use this method before all answer sets have been processed to get the current state of the item tree.
	// To obtain the finished product, you should probably call finalize() instead.
	const ItemTreePtr& getItemTree() const;

	// Call this when clasp's program is available to let this object get the clasp solver literals corresponding to atoms using output predicates
	virtual void prepare(const Clasp::Asp::LogicProgram& claspPrg) {}

protected:
	ItemTreePtr itemTree;
	const Application& app;
	bool prune;
};

} // namespace asp_utils
