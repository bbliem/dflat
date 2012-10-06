1 { extend(R) : childRow(R,_) } 1.

item(col(X,C)) :- extend(R), childItem(R,col(X,C)), current(X).
1 { item(col(X,C)) : color(X,C) } 1 :- introduced(X).
:- edge(X,Y), item(col(X,C)), item(col(Y,C)).

#hide.
#show item/1.
#show extend/1.
