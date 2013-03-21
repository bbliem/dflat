1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
item(pos(I,V))   :- extend(R), childItem(R,pos(I,V)), not removed(V).
item(pos(I,_nn)) :- extend(R), childItem(R,pos(I,V)), removed(V).

1 { item(pos(1..N,V)) } 1 :- introduced(V), N = #count { vertex(_) }.
:- item(pos(I,V0;V1)), V0 < V1.

lt(V0,V1) :- item(pos(I0,V0)), item(pos(I1,V1)), vertex(V0;V1), I0 < I1.
sat(A,B,C) :- order(A,B,C), lt(A,B), lt(B,C).
sat(A,B,C) :- order(A,B,C), lt(B,C), lt(C,A).
sat(A,B,C) :- order(A,B,C), lt(C,A), lt(A,B).

:- order(A,B,C), current(A;B;C), not sat(A,B,C).

#hide.
#show extend/1.
#show item/1.
