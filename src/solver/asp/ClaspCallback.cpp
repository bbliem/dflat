/*
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

#include "ClaspCallback.h"

namespace solver { namespace asp {

ClaspCallback::ClaspCallback(const ChildItemTrees& childItemTrees, bool printModels, bool prune)
	: childItemTrees(childItemTrees)
	, printModels(printModels)
	, prune(prune)
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

void ClaspCallback::state(Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(f.state() == Clasp::ClaspFacade::state_solve && e == Clasp::ClaspFacade::event_state_exit && printModels)
		std::cout << std::endl;
}

void ClaspCallback::event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(e == Clasp::ClaspFacade::event_model && printModels) {
		Clasp::SymbolTable& symTab = f.config()->ctx.symTab();
		std::cout << "Model " << f.config()->ctx.enumerator()->enumerated-1 << ": ";
		for(Clasp::SymbolTable::const_iterator it = symTab.begin(); it != symTab.end(); ++it) {
			if(s.isTrue(it->second.lit) && !it->second.name.empty())
				std::cout << it->second.name.c_str() << ' ';
		}
		std::cout << std::endl;
	}
}

}} // namespace solver::asp
