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
#include "../Printer.h"

namespace printer {

class Progress : public Printer
{
public:
	Progress(Application& app, bool newDefault = false);

	virtual void decomposerResult(const Decomposition& result) override;
	virtual void solverInvocationResult(const DecompositionNode& decompositionNode, const ItemTree* result) override;
	virtual bool listensForSolverEvents() const;
	virtual void solverEvent(const std::string& msg);
	virtual void result(const ItemTreePtr& rootItemTree) override;

private:
	void printProgress();

	unsigned int totalNodes;
	unsigned int curNode;

	int curFrame;
};

} // namespace printer
