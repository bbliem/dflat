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
//}}}
#include <gtest/gtest.h>

#include "Application.h"

namespace {
	// http://stackoverflow.com/questions/5419356/redirect-stdout-stderr-to-a-string
	struct RedirectionGuard
	{
		RedirectionGuard(std::ios& oldStream, const std::ios& newStream)
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

	std::string runSolver(const std::string& instance)
	{
		// Redirect stdin and stdout
		std::istringstream instanceStream(instance);
		std::ostringstream newCout;
		RedirectionGuard coutGuard(std::cout, newCout);
		RedirectionGuard cinGuard(std::cin, instanceStream);

		Application app("test");
		const char* argv[] = {"-s", "asp", "-e", "atom", "-e", "rule", "-e", "head", "-e", "pos", "-e", "neg", "-n", "normalized", "--seed", "0", "--output", "quiet", "--depth", "1"};
		app.run(20, argv);
		String::clear();
		return newCout.str();
	}
}

TEST(IntegrationAspSolver, TestProgram1)
{
	std::string result = runSolver(R"(
		atom(a).
		atom(b).
		atom(c).
		atom(d).
		rule(r1).
		rule(r2).
		rule(r3).
		% a | b.
		head(r1,a).
		head(r1,b).
		% c :- b.
		head(r2,c).
		pos(r2,b).
		% d :- not c.
		head(r3,d).
		neg(r3,c).
	)");

	EXPECT_EQ("Solutions:\n"
	          "\n"
	          "┣━ [1] a d r1 r2 r3 \n"
	          "┗━ [1] b c r1 r2 r3 \n",
	          result);
}

TEST(IntegrationAspSolver, TestProgram2)
{
	std::string result = runSolver(R"(
		atom(a).
		atom(b).
		atom(c).
		atom(d).
		atom(e).
		rule(r0).
		rule(r1).
		rule(r2).
		rule(r3).
		rule(r4).
		% a.
		head(r0,a).
		% b :- not c, not e.
		head(r1,b).
		neg(r1,c).
		neg(r1,e).
		% c :- not b, not e.
		head(r2,c).
		neg(r2,b).
		neg(r2,e).
		% d :- not e.
		head(r3,d).
		neg(r3,e).
		% e :- not d.
		head(r4,e).
		neg(r4,d).
	)");

	EXPECT_EQ("Solutions:\n"
	          "\n"
	          "┣━ [1] a c d r0 r1 r2 r3 r4 \n"
	          "┣━ [1] a b d r0 r1 r2 r3 r4 \n"
	          "┗━ [1] a e r0 r1 r2 r3 r4 \n",
	          result);
}

TEST(IntegrationAspSolver, TestProgram3)
{
	std::string result = runSolver(R"(
		atom(a).
		atom(b).
		rule(r0).
		rule(r1).
		rule(r2).
		% a | b
		head(r0,a).
		head(r0,b).
		% b :- a.
		head(r1,b).
		pos(r1,a).
		% a :- b.
		head(r2,a).
		pos(r2,b).
	)");

	EXPECT_EQ("Solutions:\n"
	          "\n"
	          "┗━ [1] a b r0 r1 r2 \n",
	          result);
}

TEST(IntegrationAspSolver, TestProgram4)
{
	std::string result = runSolver(R"(
		atom(a).
		atom(b).
		atom(c).
		atom(d).
		atom(e).
		rule(r0).
		rule(r1).
		rule(r2).
		rule(r3).
		rule(r4).
		rule(constraint).
		% a.
		head(r0,a).
		% b :- not c, not e.
		head(r1,b).
		neg(r1,c).
		neg(r1,e).
		% c :- not b, not e.
		head(r2,c).
		neg(r2,b).
		neg(r2,e).
		% d :- not e.
		head(r3,d).
		neg(r3,e).
		% e :- not d.
		head(r4,e).
		neg(r4,d).
		% :- a, b, d.
		pos(constraint,a).
		pos(constraint,b).
		pos(constraint,d).
	)");

	EXPECT_EQ("Solutions:\n"
	          "\n"
	          "┣━ [1] a c d r0 r1 r2 r3 r4 constraint \n"
	          "┗━ [1] a e r0 r1 r2 r3 r4 constraint \n",
	          result);
}
