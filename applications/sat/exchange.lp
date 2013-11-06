1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
% Some child rows cannot be used since they are killed
:- clause(C), removed(C), extend(R), not childAuxItem(R,C).

% True atoms and satisfied clauses remain so unless removed
item(X) :- extend(R), childItem(R,X), current(X).
auxItem(C) :- extend(R), childAuxItem(R,C), current(C).

% Guess truth value of introduced atoms
{ item(A) : atom(A) : introduced(A) }.
% Through the guess, clauses may become satisfied
auxItem(C) :- current(C;A), pos(C,A), item(A).
auxItem(C) :- current(C;A), neg(C,A), not item(A).

#hide.
#show item/1.
#show auxItem/1.
#show extend/1.
