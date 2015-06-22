1 { p(X,Y) : edge(X,Y) } 1 :- vertex(X).
1 { p(X,Y) : edge(X,Y) } 1 :- vertex(Y).

% We need a distinguished vertex
notFirst(Y) :- vertex(X), vertex(Y), X < Y.
first(X) :- vertex(X), not notFirst(X).

reachable(Y) :- first(X), p(X,Y).
reachable(Y) :- reachable(X), p(X,Y).

:- vertex(X), not reachable(X).

#show p/2.
