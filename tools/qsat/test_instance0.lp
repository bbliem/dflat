% Ex Ay Ez: -x & (x | -y | z)
% Expect 1 solution
levels(3).

atom(0,x).
atom(1,y).
atom(2,z).

clause(c0).
clause(c1).

neg(c0,x).

pos(c1,x).
neg(c1,y).
pos(c1,z).
