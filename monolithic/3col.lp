node(X) :- edge(X,Y).
node(Y) :- edge(X,Y).

1 { r(X), g(X), b(X) } 1 :- node(X).

:- edge(X,Y), r(X), r(Y).
:- edge(X,Y), g(X), g(Y).
:- edge(X,Y), b(X), b(Y).
