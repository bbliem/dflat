color(red;green;blue).

1 { map(X,C) : color(C) } 1 :- vertex(X).
:- edge(X,Y), map(X,C), map(Y,C).

#show map/2.
