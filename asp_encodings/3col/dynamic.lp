color(red;green;blue).

1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
:- extend(A), extend(B), childItem(A,col(X,VA)), childItem(B,col(X,VB)), VA != VB.

item(col(X,C)) :- extend(I), childItem(I,col(X,C)), current(X).

1 { item(col(X,C)) : color(C) } 1 :- introduced(X).
:- edge(X,Y), item(col(X,C)), item(col(Y,C)).

#hide.
#show item/1.
#show extend/1.
