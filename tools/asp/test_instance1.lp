% Expect 1 answer set: {a,b}
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
