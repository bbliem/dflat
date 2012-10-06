color(red;green;blue).

% Which tuple to use?
nonLeaf :- childTuple(_).
1 {chosenChildTuple(I) : childTuple(I)} 1 :- nonLeaf.

map(X,C) :- chosenChildTuple(I), mapped(I,X,C), current(X).
1 {map(X,C) : color(C)} 1 :- introduced(X).
:- edge(X,Y), map(X,C), map(Y,C).

#hide.
#show map/2.
#show chosenChildTuple/1.
