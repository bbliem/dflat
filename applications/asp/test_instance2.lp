% Expect 2 answer sets: {a,e} and {a,c,d}
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
