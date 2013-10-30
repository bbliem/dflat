1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
% Some child rows cannot be used since they are killed
:- clause(C), removed(C), extend(R), not childConsequentItem(R,C).

% True atoms and satisfied clauses remain so unless removed
item(X) :- extend(R), childItem(R,X), current(X).
consequentItem(C) :- extend(R), childConsequentItem(R,C), current(C).

% Guess truth value of introduced atoms
{ item(A) : atom(A) : introduced(A) }.
% Through the guess, clauses may become satisfied
consequentItem(C) :- current(C;A), pos(C,A), item(A).
consequentItem(C) :- current(C;A), neg(C,A), not item(A).

#hide.
#show item/1.
#show consequentItem/1.
#show extend/1.
