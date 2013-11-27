or(L)  :- length(M), L = 0..M-1, L #mod 2 == 0. % existential nodes
and(L) :- length(M), L = 0..M-1, L #mod 2 == 1. % universal nodes

% Guess a branch
1 { extend(L,S) : leafOf(S,N) : atLevel(S,L) } 1 :- childNode(N).
extend(L-1,R) :- extend(L,S), sub(R,S).

% Only join branches that coincide on common atoms
:- extend(L,X;Y), atom(L,A), childItem(X,t(A)), childItem(Y,f(A)).

item(L,t(X)) :- extend(L,S), childItem(S,t(X)), not removed(X).
item(L,f(X)) :- extend(L,S), childItem(S,f(X)), not removed(X).
auxItem(L,sat(C)) :- extend(L,S), childAuxItem(S,sat(C)), not removed(C).

% Guess truth value of introduced atoms
1 { item(L,t(A);f(A)) } 1 :- atom(L,A), introduced(A).

% Through the guess, clauses may become satisfied
auxItem(L,sat(C)) :- length(L), pos(C,A), atom(LA,A), current(C;A), item(LA,t(A)).
auxItem(L,sat(C)) :- length(L), neg(C,A), atom(LA,A), current(C;A), item(LA,f(A)).
% XXX what if clauses become false? Can we optimize?

reject :- length(L), clause(C), removed(C), bag(N,C), atNode(S,N), extend(L,S), not childAuxItem(S,sat(C)).

#hide.
#show length/1.
#show item/2.
#show auxItem/2.
#show extend/2.
#show or/1.
#show and/1.
#show reject/0.
