% Make explicit that a row interprets an atom as false or a clause as unsatisfied
false(R,X) :- childRow(R,N), bag(N,X), not childItem(R,X).
unsat(R,C) :- childRow(R,N), bag(N,C), not childConsequentItem(R,C).

1 { extend(R) : childRow(R,N) } 1 :- childNode(N).

% Only join rows that coincide on common atoms
:- extend(X;Y), atom(A), childItem(X,A), false(Y,A).

% Some child rows cannot be extended since they are killed
:- clause(C), removed(C), extend(R), unsat(R,C).

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
