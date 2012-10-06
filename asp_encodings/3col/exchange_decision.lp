color(red;green;blue).

1 { item(col(X,C)) : color(C) } 1 :- current(X).
:- edge(X,Y), item(col(X,C)), item(col(Y,C)).

noMatch(R) :- childRow(R,_), item(col(X,C)), childItem(R,col(X,OldC)), C != OldC.
match :- childRow(R,_), not noMatch(R).
:- not match.

#hide.
#show item/1.
