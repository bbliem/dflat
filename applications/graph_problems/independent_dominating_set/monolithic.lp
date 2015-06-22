{ in(X) : vertex(X) }.
:- in(X), in(Y), edge(X,Y).
dominated(X) :- in(Y), edge(Y,X).
:- vertex(X), not in(X), not dominated(X).
