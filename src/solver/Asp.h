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

#pragma once

#include "../Solver.h"

namespace solver {

class Asp : public Solver
{
public:
	Asp(const Decomposition& decomposition, const Application& app, const std::string& encodingFile, bool tableMode);

	virtual ItemTreePtr compute() override;

	static void declareDecomposition(const Decomposition& decomposition, std::ostream& out);
	static void declareItemTree(std::ostream& out, const ItemTree* itemTree, bool tableMode, unsigned int nodeId, const std::string& itemSetName, const std::string& parent = "", unsigned int level = 0);

protected:
	std::string encodingFile;
	bool tableMode;
};

} // namespace solver
