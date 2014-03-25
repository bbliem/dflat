1 { map(X,C) : color(X,C) } 1 :- vertex(X).
:- edge(X,Y), map(X,C), map(Y,C).

#show map/2.
