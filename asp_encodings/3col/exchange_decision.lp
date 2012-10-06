color(red;green;blue).
1 {map(X,C) : color(C)} 1 :- current(X).
:- edge(X,Y), map(X,C), map(Y,C).

nonLeaf :- childTuple(I).

noMatch(I) :- childTuple(I), map(X,C), mapped(I,X,OldC), C != OldC.
noMatch(I) :- childTuple(I), map(X,C), mapped(I,X,OldC), C != OldC.
noMatch(I) :- childTuple(I), map(X,C), mapped(I,X,OldC), C != OldC.

match :- childTuple(I), not noMatch(I).
:- nonLeaf, not match.

% TODO: These are auxiliary predicates until the first-order interface is done
mapped(I,X,C) :- mapped(I,0,X,C).
error :- mapped(_,L,_,_), L != 0.
:- error.
map(0,X,C) :- map(X,C).
#hide.
#show map/3.
