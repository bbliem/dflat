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
//}}}
#include <clasp/solver.h>

#include "ClaspCallback.h"
#include "../../Application.h"
#include "../../Debugger.h"

namespace solver { namespace asp {

ClaspCallback::ClaspCallback(const ChildItemTrees& childItemTrees, const Application& app)
	: childItemTrees(childItemTrees)
	, app(app)
{
}

ItemTreePtr ClaspCallback::finalize()
{
	if(itemTree)
		itemTree->finalize();
	return std::move(itemTree);
}

bool ClaspCallback::onModel(const Clasp::Solver& s, const Clasp::Model& m)
{
	if(app.getDebugger().listensForSolverEvents()) {
		Clasp::SymbolTable& symTab = s.sharedContext()->symTab();
		std::ostringstream msg;
		msg << "Model: ";
		for(Clasp::SymbolTable::const_iterator it = symTab.begin(); it != symTab.end(); ++it) {
			if(m.isTrue(it->second.lit) && !it->second.name.empty())
				msg << it->second.name.c_str() << ' ';
		}
		app.getDebugger().solverEvent(msg.str());
	}
	return true;
}

}} // namespace solver::asp
