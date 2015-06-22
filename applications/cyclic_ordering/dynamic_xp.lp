%dflat: --tables -d td -n weak -e order
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
:- extend(R), extend(S), childItem(R,pos(I,V)), vertex(V), not childItem(S,pos(I,V)).
:- extend(R), extend(S), R < S, childItem(R,pos(I,_nn)), childItem(S,pos(I,_nn)).
item(pos(I,V))   :- extend(R), childItem(R,pos(I,V)), not removed(V).
item(pos(I,_nn)) :- extend(R), childItem(R,pos(I,V)), removed(V).

1 { item(pos(1..N,V)) } 1 :- introduced(V), N = #count { X : vertex(X) }.
:- item(pos(I,V0)), item(pos(I,V1)), V0 < V1.

lt(V0,V1) :- item(pos(I0,V0)), item(pos(I1,V1)), vertex(V0), vertex(V1), I0 < I1.
sat(A,B,C) :- order(A,B,C), lt(A,B), lt(B,C).
sat(A,B,C) :- order(A,B,C), lt(B,C), lt(C,A).
sat(A,B,C) :- order(A,B,C), lt(C,A), lt(A,B).

:- order(A,B,C), current(A), current(B), current(C), not sat(A,B,C).

#show extend/1.
#show item/1.
