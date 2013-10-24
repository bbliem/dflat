1 { extend(R) : childRow(R,_) } 1.
% Some child rows cannot be used since they are killed
:- clause(C), removed(C), extend(R), not childItem(R,C).

% True atoms and satisfied clauses remain so unless removed
item(X) :- extend(R), childItem(R,X), current(X).

% Guess truth value of introduced atoms
{ item(A) : atom(A) : introduced(A) }.
% Through the guess, clauses may become satisfied
item(C) :- current(C;A), pos(C,A), item(A).
item(C) :- current(C;A), neg(C,A), not item(A).

#hide.
#show item/1.
#show extend/1.
