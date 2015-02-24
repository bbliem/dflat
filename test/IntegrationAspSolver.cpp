/*{{{
Copyright 2012-2015, Bernhard Bliem
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
#include <gtest/gtest.h>

#include "Application.h"

namespace {
	// http://stackoverflow.com/questions/5419356/redirect-stdout-stderr-to-a-string
	struct RedirectionGuard
	{
		RedirectionGuard(std::ios& oldStream, std::ios& newStream)
			: oldStream(oldStream), oldBuffer(oldStream.rdbuf(newStream.rdbuf()))
		{}

		~RedirectionGuard()
		{
			oldStream.rdbuf(oldBuffer);
		}

	private:
		std::ios& oldStream;
		std::streambuf* oldBuffer;
	};
}

TEST(IntegrationAspSolver, TestProgram1)
{
	std::istringstream instance(R"(
		% a | b.
		% c :- b.
		% d :- not c.
		rule(r1).
		rule(r2).
		rule(r3).
		atom(a).
		atom(b).
		atom(c).
		atom(d).
		head(r1,a).
		head(r1,b).
		head(r2,c).
		pos(r2,b).
		head(r3,d).
		neg(r3,c).
	)");

	// Redirect stdin and stdout
	std::ostringstream newCout;
	RedirectionGuard coutGuard(std::cout, newCout);
	RedirectionGuard cinGuard(std::cin, instance);

	Application app("test");
	const char* argv[] = {"-s", "asp", "-e", "atom", "-e", "rule", "-e", "head", "-e", "pos", "-e", "neg", "-n", "normalized", "--seed", "0", "--output", "quiet", "--depth", "1"};
	app.run(20, argv);
	EXPECT_EQ("Solutions:\n"
	          "\n"
	          "┣━ [1] a d r1 r2 r3 \n"
	          "┗━ [1] b c r1 r2 r3 \n",
	          newCout.str());
}
