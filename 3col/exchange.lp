% Which tuple to use?
nonLeaf :- childTuple(X).
1 {chosenChildTuple(I) : childTuple(I)} 1 :- nonLeaf.

r(X) :- chosenChildTuple(I), oldR(I,X), not removed(X).
g(X) :- chosenChildTuple(I), oldG(I,X), not removed(X).
b(X) :- chosenChildTuple(I), oldB(I,X), not removed(X).

1 {r(X), g(X), b(X)} 1 :- introduced(X).

% XXX: We generate as input for this program edges to/from nodes that are not even in the current bag. Is this overhead bad?
:- edge(X,Y), r(X), r(Y).
:- edge(X,Y), g(X), g(Y).
:- edge(X,Y), b(X), b(Y).


#hide.
#show r/1.
#show g/1.
#show b/1.
#show chosenChildTuple/1.
