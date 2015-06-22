% guess, if the vertex is in the subset
0 {out(X)} 1 :- vertex(X).

% check subset for cycles
path(X,Y) :- edge(X,Y), not out(X), not out(Y).
path(X,Z) :- path(X,Y), edge(Y,Z), not out(X), not out(Y), not out(Z).
path(X,Z) :- edge(X,Y), path(Y,Z), not out(X), not out(Y), not out(Z).
cycle :- path(X,X).

% kill answer sets in which the subset contains a cycle
:- cycle.

% kill answer sets in which the cardinality of the subset exceeds k
:- k(K), C= #count {X : out(X)}, C > K.

#show out/1.
