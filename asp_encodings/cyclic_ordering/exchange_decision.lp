1 { map(V,1..N) } 1 :- current(V), N = #count { current(_) }.
:- map(V1,K), map(V2,K), V1 != V2. % TODO: Use < instead (symmetry)?

lt(V1,V2) :- map(V1,K1), map(V2,K2), K1 < K2.
% TODO: Will using a predicate like relevantOrder/3 improve performance? relevantOrder(A,B,C) :- order(A,B,C), current(A;B;C).
sat(A,B,C) :- order(A,B,C), lt(A,B), lt(B,C).
sat(A,B,C) :- order(A,B,C), lt(B,C), lt(C,A).
sat(A,B,C) :- order(A,B,C), lt(C,A), lt(A,B).

:- order(A,B,C), current(A;B;C), not sat(A,B,C).

gtChild(I,V1,V2) :- mapped(I,V1,K1), mapped(I,V2,K2), current(V1;V2), K1 > K2.
noMatch(I) :- lt(V1,V2), gtChild(I,V1,V2).
match :- childTuple(I), not noMatch(I).

nonLeaf :- childTuple(I).
:- nonLeaf, not match.

% TODO: These are auxiliary predicates until the first-order interface is done
mapped(I,X,C) :- mapped(I,0,X,C).
error :- mapped(_,L,_,_), L != 0.
:- error.
map(0,X,C) :- map(X,C).
#hide.
#show map/3.
