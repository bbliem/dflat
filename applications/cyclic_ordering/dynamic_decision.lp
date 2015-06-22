%dflat: --tables -d td -n weak -e order --depth 0
1 { item(map(V,1..N)) } 1 :- current(V), N = #count { X : current(X) }.
:- item(map(V1,K)), item(map(V2,K)), V1 != V2.

lt(V1,V2) :- item(map(V1,K1)), item(map(V2,K2)), K1 < K2.
sat(A,B,C) :- order(A,B,C), lt(A,B), lt(B,C).
sat(A,B,C) :- order(A,B,C), lt(B,C), lt(C,A).
sat(A,B,C) :- order(A,B,C), lt(C,A), lt(A,B).

:- order(A,B,C), current(A), current(B), current(C), not sat(A,B,C).

% Now check if we can find a compatible predecessor in each child table
%gtChild(R,V1,V2) :- childItem(R,map(V1,K1)), childItem(R,map(V2,K2)), current(V1), current(V2), K1 > K2.
%incompatible(R) :- lt(V1,V2), gtChild(R,V1,V2).
%match(N) :- childRow(R,N), not incompatible(R).
%:- childNode(N), not match(N).

% D-FLAT currently requires us to use extension pointers.
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
:- extend(R), childItem(R,map(V1,K1)), childItem(R,map(V2,K2)), current(V1), current(V2), K1 > K2, lt(V1,V2).
:- extend(R), extend(S), childItem(R,map(V,K)), current(V), not childItem(S,map(V,K)).

#show extend/1.
#show item/1.
