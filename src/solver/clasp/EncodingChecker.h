/*{{{
Copyright 2012-2016, Bernhard Bliem
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
#include <gringo/input/programbuilder.hh>

namespace solver { namespace clasp {

class EncodingChecker : public Gringo::Input::NongroundProgramBuilder
{
public:
	using Gringo::Input::NongroundProgramBuilder::NongroundProgramBuilder;

	// Call this after parsing. May throw an exception if the encoding fails a check.
	virtual void check() = 0;
};

}} // namespace solver::clasp
