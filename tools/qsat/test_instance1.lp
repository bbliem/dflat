% Expect 0 solutions
levels(3).

atom(0,a).
atom(1,b).
atom(2,c).

clause(c0).
clause(c1).
clause(c2).
clause(c3).

pos(c0,a).
pos(c0,b).
pos(c0,c).

pos(c1,b).
neg(c1,c).

pos(c2,a).
neg(c2,b).
neg(c2,c).

neg(c3,a).
pos(c3,b).
