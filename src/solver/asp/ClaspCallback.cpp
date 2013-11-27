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
#include "ClaspCallback.h"
#include "../../Debugger.h"

namespace solver { namespace asp {

ClaspCallback::ClaspCallback(const ChildItemTrees& childItemTrees, const Debugger& debugger)
	: childItemTrees(childItemTrees)
	, debugger(debugger)
{
}

ItemTreePtr ClaspCallback::finalize()
{
	if(itemTree)
		itemTree->finalize();
	return std::move(itemTree);
}

void ClaspCallback::warning(const char* msg)
{
	std::cerr << "clasp warning: " << msg << std::endl;
}

void ClaspCallback::event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(e == Clasp::ClaspFacade::event_model && debugger.listensForSolverEvents()) {
		Clasp::SymbolTable& symTab = f.config()->ctx.symTab();
		std::ostringstream msg;
		msg << "Model " << f.config()->ctx.enumerator()->enumerated-1 << ": ";
		for(Clasp::SymbolTable::const_iterator it = symTab.begin(); it != symTab.end(); ++it) {
			if(s.isTrue(it->second.lit) && !it->second.name.empty())
				msg << it->second.name.c_str() << ' ';
		}
		debugger.solverEvent(msg.str());
	}
}

}} // namespace solver::asp
