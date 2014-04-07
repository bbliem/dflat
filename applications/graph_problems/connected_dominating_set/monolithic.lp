edge(X,Y) :- edge(Y,X).

{ selected(V) : vertex(V) }.

dominated(V2) :- selected(V1), edge(V1,V2), not selected(V2).

connected(X,Y) :- selected(X), selected(Y), edge(X,Y), X < Y.

connected(X,Z) :- selected(X), selected(Y), selected(Z), connected(X,Y), edge(Y,Z), X < Z, X != Y, Y != Z.
connected(X,Z) :- selected(X), selected(Y), selected(Z), connected(Y,X), edge(Y,Z), X < Z, X != Y, Y != Z.
connected(X,Z) :- selected(X), selected(Y), selected(Z), connected(Y,Z), edge(X,Y), X < Z, X != Y, Y != Z.
connected(X,Z) :- selected(X), selected(Y), selected(Z), connected(Z,Y), edge(X,Y), X < Z, X != Y, Y != Z.

:- selected(X), selected(Y), not connected(X,Y), not connected(Y,X), X < Y.

:- vertex(X), not dominated(X), not selected(X).

#minimize { 1,X : selected(X) }.

#show selected/1.
