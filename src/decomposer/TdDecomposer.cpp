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

#include "TdDecomposer.h"

namespace decomposer {

const std::string TdDecomposer::OPTION_SECTION = "Tree decomposition";

TdDecomposer::TdDecomposer(Application& app, bool newDefault)
	: Decomposer(app, "td", "Tree decomposition", newDefault)
	, optNormalization("n", "normalization", "Use normal form <normalization> for the tree decomposition")
{
	optNormalization.addCondition(selected);
	optNormalization.addChoice("none", "No normalization", true);
	optNormalization.addChoice("semi", "Semi-normalization");
	optNormalization.addChoice("normalized", "Normalization");

	app.getOptionHandler().addOption(optNormalization, OPTION_SECTION);
}

Decomposition TdDecomposer::decompose(const Hypergraph& inst) const
{
	Decomposition td = Node(Hypergraph::Vertices());

	//sharp::ExtendedHypertree* eh = calculateHypertreeDecomposition();

	return td;
}

} // namespace decomposer
