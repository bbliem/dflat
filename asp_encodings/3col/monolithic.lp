color(red;green;blue).
vertex(X) :- edge(X,_).
vertex(X) :- edge(_,X).

1 { map(X,C) : color(C) } 1 :- vertex(X).
:- edge(X,Y), map(X,C), map(Y,C).

#hide.
#show map/2.
