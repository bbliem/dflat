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
//}}}
#include <clasp/solver.h>
#include <cassert>

#include "SolveIter.h"

namespace solver { namespace lazy_clasp {

SolveIter::~SolveIter() {}

BasicSolveIter::BasicSolveIter(Clasp::ClaspFacade& f)
	: clasp_(&f)
	, solve_(*f.ctx.master(), f.ctx.configuration()->search(0))
	, res_(Clasp::value_free)
{
	if (!f.prepared() || solve_.solver().hasConflict() || !f.enumerator()->start(solve_.solver(), f.assumptions())) {
		res_ = Clasp::value_false;
	}
}

BasicSolveIter::~BasicSolveIter()
{
	BasicSolveIter::cancel();
}

bool BasicSolveIter::end()
{
	if (res_ != Clasp::value_free) { return res_ == Clasp::value_false; }
	Clasp::Solver& s = solve_.solver();
	Clasp::Enumerator* en = clasp_->enumerator();
	for (; res_ != Clasp::value_false;) {
		while ((res_ = solve_.solve()) == Clasp::value_true && !en->commitModel(s)) {
			en->update(s);
		}
		if      (res_ == Clasp::value_true) { return false; }
		else if (res_ == Clasp::value_free) { break; }
		else if (en->commitUnsat(s))        { next(); }
		else if (!en->commitComplete())     {
			en->end(s);
			en->start(s, clasp_->assumptions());
			res_ = Clasp::value_free;
		}
		solve_.reset();
	}
	BasicSolveIter::cancel();
	return true;
}

void BasicSolveIter::next()
{
	if (clasp_) {
		if (res_ == Clasp::value_true && clasp_->enumerator()->commitSymmetric(solve_.solver())) {
			return;
		}
		clasp_->enumerator()->update(solve_.solver());
		res_ = Clasp::value_free;
	}
}

const Clasp::Model& BasicSolveIter::model()
{
	assert(res_ == Clasp::value_true);
	return clasp_->enumerator()->lastModel();
}

void BasicSolveIter::cancel()
{
	if (clasp_) {
		solve_.solver().popRootLevel(solve_.solver().rootLevel());
		clasp_->ctx.detach(solve_.solver());
		res_ = Clasp::value_false;
		clasp_ = 0;
	}
}

}}
