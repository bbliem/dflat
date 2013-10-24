{ in(X) : vertex(X) }.

dominated(Y) :- in(X), edge(X,Y).
:- vertex(X), not in(X), not dominated(X).

#minimize[ in(_) ].
