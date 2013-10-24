color(red;green;blue).

1 { map(X,C) : color(C) } 1 :- vertex(X).
:- edge(X,Y), map(X,C), map(Y,C).

#hide.
#show map/2.

error :- edge(X,_), not vertex(X).
error :- edge(_,X), not vertex(X).
#show error/0.
