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

const std::string TdDecomposer::NAME = "td";
const std::string TdDecomposer::OPTION_SECTION = "Tree decomposition";

TdDecomposer::TdDecomposer(options::OptionHandler& opts, options::Choice& decomposers, bool newDefault)
	: optNormalization("n", "normalization", "Use normal form <normalization> for the tree decomposition")
	, decomposers(decomposers)
{
	optNormalization.addChoice("none", "No normalization", true);
	optNormalization.addChoice("semi", "Semi-normalization");
	optNormalization.addChoice("normalized", "Normalization");

	optNormalization.addCondition(selected);

	decomposers.addChoice(NAME, "Tree Decomposition", newDefault);
	opts.addOption(optNormalization, OPTION_SECTION);
	opts.registerObserver(*this);
}

void TdDecomposer::notify()
{
	if(decomposers.getValue() == NAME)
		selected.setSatisfied();
}

} // namespace decomposer
