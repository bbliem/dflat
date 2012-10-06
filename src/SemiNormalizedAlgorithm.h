/*
Copyright 2012, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dynasp/dflat/>.

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

#include "Algorithm.h"

class SemiNormalizedAlgorithm : public Algorithm
{
public:
	//! @param normalizationType either sharp::SemiNormalization or sharp::DefaultNormalization
	SemiNormalizedAlgorithm(sharp::Problem& problem, const sharp::PlanFactory& planFactory, const std::string& instanceFacts, const char* exchangeNodeProgram, const char* joinNodeProgram = 0, sharp::NormalizationType normalizationType = sharp::SemiNormalization, bool ignoreOptimization = false, unsigned int level = 0);

protected:
	virtual const char* getUserProgram(const sharp::ExtendedHypertree& node);

	virtual sharp::Table* computeTable(const sharp::ExtendedHypertree& node, const std::vector<sharp::Table*>& childTables);
	virtual sharp::ExtendedHypertree* prepareHypertreeDecomposition(sharp::ExtendedHypertree* root);

private:
	sharp::Table* defaultJoin(const sharp::ExtendedHypertree& node, const std::vector<sharp::Table*>& childTables) const;

	const char* exchangeNodeProgram;
	const char* joinNodeProgram;
};
