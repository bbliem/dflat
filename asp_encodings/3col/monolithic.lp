color(red;green;blue).
node(X) :- edge(X,_).
node(X) :- edge(_,X).

1 { map(X,C) : color(C) } 1 :- node(X).
:- edge(X,Y), map(X,C), map(Y,C).

#hide.
#show map/2.
