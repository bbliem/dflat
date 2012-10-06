nonLeaf :- childTuple(_).
1 { chosenChildTuple(I) : childTuple(I) } 1 :- nonLeaf.
map(X,Y) :- current(X), chosenChildTuple(I), mapped(I,X,Y).

1 { in(X), out(X) } 1 :- introduced(X).
map(X,in) :- in(X).
map(X,out) :- out(X).
:- edge(X,Y), map(X,out), map(Y,out).

currentCost(C) :- C = #count{ map(_,in) }. % Cost of current vertices
cost(C) :- currentCost(C), not nonLeaf.
cost(C) :- chosenChildTuple(I), childCost(I,CC), IC = #count{ in(_) }, C = CC + IC.

#hide.
#show chosenChildTuple/1.
#show currentCost/1.
#show cost/1.
#show map/2.
