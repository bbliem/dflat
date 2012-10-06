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

class NonNormalizedAlgorithm : public Algorithm
{
public:
	NonNormalizedAlgorithm(sharp::Problem& problem, const std::string& instanceFacts, const char* program, sharp::NormalizationType normalizationType = sharp::NoNormalization, bool ignoreOptimization = false, bool multiLevel = false);

protected:
	virtual const char* getUserProgram(const sharp::ExtendedHypertree& node);

private:
	const char* program;
};
