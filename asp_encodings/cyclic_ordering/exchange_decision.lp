1 { map(V,1..N) } 1 :- current(V), N = #count { current(_) }.
:- map(V1,K), map(V2,K), V1 < V2.

lt(V1,V2) :- map(V1,K1), map(V2,K2), K1 < K2.
sat(A,B,C) :- order(A,B,C), lt(A,B), lt(B,C).
sat(A,B,C) :- order(A,B,C), lt(B,C), lt(C,A).
sat(A,B,C) :- order(A,B,C), lt(C,A), lt(A,B).

:- order(A,B,C), current(A;B;C), not sat(A,B,C).

gtChild(I,V1,V2) :- mapped(I,V1,K1), mapped(I,V2,K2), current(V1;V2), K1 > K2.
noMatch(I) :- lt(V1,V2), gtChild(I,V1,V2).
match :- childTuple(I), not noMatch(I).

:- not match.



#hide.
#show map/2.
