% Expect 1 solution
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
neg(c0,c).

pos(c1,b).
pos(c1,c).

pos(c2,a).
neg(c2,b).
pos(c2,c).

neg(c3,a).
neg(c3,b).
neg(c3,c).
