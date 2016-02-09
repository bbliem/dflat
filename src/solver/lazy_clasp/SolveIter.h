/*{{{
Copyright 2015-2016, Benjamin Kaufmann
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

namespace solver { namespace lazy_clasp {

class SolveIter
{
public:
	typedef Clasp::Model Model;
	virtual ~SolveIter();
	virtual void cancel() = 0;
	virtual bool end()  = 0;
	virtual void next() = 0;
	virtual const Model& model() = 0;
};


class BasicSolveIter : public SolveIter
{
public:
	BasicSolveIter(Clasp::ClaspFacade& f);
	virtual ~BasicSolveIter();
	virtual void cancel() override;
	virtual bool end()    override;
	virtual void next()   override;
	virtual const Model& model() override;

private:
	Clasp::ClaspFacade* clasp_;
	Clasp::BasicSolve   solve_;
	Clasp::ValueRep     res_;
};

}} // namespace solver::lazy_clasp
