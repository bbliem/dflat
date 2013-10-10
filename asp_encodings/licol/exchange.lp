1 { extend(R) : childRow(R,N) } 1 :- childNode(N).

item(map(X,C)) :- extend(R), childItem(R,map(X,C)), current(X).
1 { item(map(X,C)) : color(X,C) } 1 :- introduced(X).
:- edge(X,Y), item(map(X,C)), item(map(Y,C)).

#hide.
#show item/1.
#show extend/1.
