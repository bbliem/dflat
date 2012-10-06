color(red;green;blue).

1 { map(X,C) : color(C) } 1 :- current(X).
:- edge(X,Y), map(X,C), map(Y,C).

noMatch(I) :- childTuple(I), map(X,C), mapped(I,X,OldC), C != OldC.
match :- childTuple(I), not noMatch(I).
:- not match.

#hide.
#show map/2.
