color(red;green;blue).

1 { item(map(X,C)) : color(C) } 1 :- current(X).
:- edge(X,Y), item(map(X,C)), item(map(Y,C)).

noMatch(R) :- childRow(R,_), item(map(X,C)), childItem(R,map(X,OldC)), C != OldC.
match :- childRow(R,_), not noMatch(R).
:- childNode(_), not match.

#hide.
#show item/1.
