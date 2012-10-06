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

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "NonNormalizedAlgorithm.h"
#include "Row.h"

using sharp::Table;

NonNormalizedAlgorithm::NonNormalizedAlgorithm(sharp::Problem& problem, const sharp::PlanFactory& planFactory, const std::string& instanceFacts, const char* program, sharp::NormalizationType normalizationType, bool ignoreOptimization, unsigned int level)
	: Algorithm(problem, planFactory, instanceFacts, normalizationType, ignoreOptimization, level), program(program)
{
}

const char* NonNormalizedAlgorithm::getUserProgram(const sharp::ExtendedHypertree& node)
{
	return program;
}
