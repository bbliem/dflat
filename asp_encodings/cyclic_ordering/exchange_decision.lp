n(N) :- N = #count { current(V) }.
1 { map(V,1..N) } 1 :- current(V), n(N).
:- map(V1,K), map(V2,K), V1 != V2.

lt(V1,V2) :- map(V1,K1), map(V2,K2), K1 < K2.
sat(A,B,C) :- order(A,B,C), lt(A,B), lt(B,C).
sat(A,B,C) :- order(A,B,C), lt(B,C), lt(C,A).
sat(A,B,C) :- order(A,B,C), lt(C,A), lt(A,B).

:- order(A,B,C), not sat(A,B,C).

noMatch(I) :- succ(I,V1,V2), lt(V2,V1).
match :- childTuple(I), not noMatch(I).

nonLeaf :- childTuple(I).
:- nonLeaf, not match.

#hide.
#show map/2.
#show n/1.
