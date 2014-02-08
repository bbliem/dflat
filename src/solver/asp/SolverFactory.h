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

#pragma once
//}}}
#include "config.h"
#include "../../SolverFactory.h"
#include "../../options/MultiValueOption.h"

#ifdef HAVE_WORDEXP_H
#include <vector>
#endif

namespace solver { namespace asp {

class SolverFactory : public ::SolverFactory
{
public:
	SolverFactory(Application& app, bool newDefault = false);

	virtual std::unique_ptr<::Solver> newSolver(const Decomposition& decomposition) const override;

	virtual void select() override;

#ifdef HAVE_WORDEXP_H
	// Called after parsing to notify this observer.
	// Scans for modelines in the encoding files.
	virtual void notify() override;
#endif

private:
	static const std::string OPTION_SECTION;

	options::MultiValueOption optEncodingFiles;
	options::Option           optDefaultJoin;
	options::Option           optLazy;
	options::Option           optTables;
#ifdef HAVE_WORDEXP_H
	options::Option           optIgnoreModelines;
	// To avoid infinite recursions
	std::vector<std::string> modelineStack;
#endif
};

}} // namespace solver::asp
