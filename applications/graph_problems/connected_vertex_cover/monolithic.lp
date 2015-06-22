% edges are undirected
edge(Y,X) :- edge(X,Y).

% guess for each vertex, if it is in the VC set or not
0 { in(X) } 1 :- vertex(X).

% kill answer sets containing an edge that is not incident to a vertex in the VC set
:- edge(X,Y), not in(X), not in(Y).

% kill answer sets having more than k vertices in the VC set
:- k(K), C = #count {X : in(X)}, C > K.

% kill answer sets in which there is a vertex in the VC set that is not connected
path(X,Y) :- edge(X,Y), in(X), in(Y).
path(X,Z) :- path(X,Y), edge(Y,Z), in(X), in(Y), in(Z).
path(X,Z) :- edge(X,Y), path(Y,Z), in(X), in(Y), in(Z).

:- in(X), in(Y), not path(X,Y).

#show in/1.
