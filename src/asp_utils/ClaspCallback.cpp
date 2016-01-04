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
#include <clasp/solver.h>

#include "ClaspCallback.h"
#include "../Application.h"
#include "../Printer.h"

namespace asp_utils {

ClaspCallback::ClaspCallback(const Application& app)
	: app(app)
{
}

ItemTreePtr ClaspCallback::finalize(bool pruneUndefined, bool pruneRejecting)
{
	if(itemTree && itemTree->finalize(app, pruneUndefined, pruneRejecting) == false)
		itemTree.reset();
	return std::move(itemTree);
}

const ItemTreePtr& ClaspCallback::getItemTree() const
{
	return itemTree;
}

bool ClaspCallback::onModel(const Clasp::Solver& s, const Clasp::Model& m)
{
	if(app.getPrinter().listensForSolverEvents()) {
		Clasp::SymbolTable& symbolTable = s.sharedContext()->symbolTable();
		std::ostringstream msg;
		msg << "Model: ";
		for(Clasp::SymbolTable::const_iterator it = symbolTable.begin(); it != symbolTable.end(); ++it) {
			if(m.isTrue(it->second.lit) && !it->second.name.empty())
				msg << it->second.name.c_str() << ' ';
		}
		app.getPrinter().solverEvent(msg.str());
	}
	return true;
}

} // namespace asp_utils
