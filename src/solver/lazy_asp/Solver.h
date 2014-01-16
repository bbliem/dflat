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
#include <mutex>
#include <condition_variable>
#include <clasp/clasp_facade.h>

#include "ClaspCallback.h"
#include "../../Decomposition.h"

namespace solver { namespace lazy_asp {

class Solver : public ::Solver
{
public:
	Solver(const Decomposition& decomposition, const Application& app, const std::vector<std::string>& encodingFiles);

	virtual ItemTreePtr compute() override;

	ItemTree::Children::const_iterator nextRow();

	// When the solver is currently in this->compute(), other objects can get the item tree that has been constructed so far with this method.
	const ItemTreePtr& getItemTreeSoFar() const;

	// Call this from a worker thread that performs ASP solving to let the waiting main thread proceed.
	// This method blocks until the worker thread should do more work.
	// The argument is the lock held by the worker thread.
	void proceed(std::unique_lock<std::mutex>&);

private:
	std::vector<std::string> encodingFiles;
	std::unordered_map<std::string, Clasp::Var> itemsToVars;

	void workerThreadMain();
	void aspCallsOnNewRowFromChild(ItemTree::Children::const_iterator newRow, const DecompositionPtr& originatingChild, Clasp::ClaspFacade& clasp);
	bool nextRowCombination(std::vector<std::pair<Decomposition*, ItemTree::Children::const_iterator>>& rowIterators, size_t incrementPos = 1);

	std::unique_ptr<ClaspCallback> claspCallback;
	bool noMoreModels = false;

	std::mutex workerMutex;
	std::condition_variable wakeMainThread;
	std::condition_variable wakeWorkerThread;
	bool wakeMainThreadRequested = false;
	bool wakeWorkerThreadRequested = false;
};

}} // namespace solver::lazy_asp
