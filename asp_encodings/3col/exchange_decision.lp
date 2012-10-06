1 {r(X), g(X), b(X)} 1 :- current(X).
nonLeaf :- childTuple(I).

noMatch(I) :- childTuple(I), r(X), oldG(I,X).
noMatch(I) :- childTuple(I), r(X), oldB(I,X).
noMatch(I) :- childTuple(I), g(X), oldR(I,X).
noMatch(I) :- childTuple(I), g(X), oldB(I,X).
noMatch(I) :- childTuple(I), b(X), oldR(I,X).
noMatch(I) :- childTuple(I), b(X), oldG(I,X).

match :- childTuple(I), not noMatch(I).
:- nonLeaf, not match.

#hide.
#show r/1.
#show g/1.
#show b/1.
