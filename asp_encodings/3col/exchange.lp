color(red;green;blue).

1 { extend(R) : childRow(R,_) } 1.

item(map(X,C)) :- extend(R), childItem(R,map(X,C)), current(X).
1 { item(map(X,C)) : color(C) } 1 :- introduced(X).
:- edge(X,Y), item(map(X,C)), item(map(Y,C)).

#hide.
#show item/1.
#show extend/1.
