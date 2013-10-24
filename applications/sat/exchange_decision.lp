% Guess truth value for each bag element except introduced clauses since these can only be true because they are enforced by the guess
guessable(A) :- current(A), atom(A).
guessable(C) :- current(C), clause(C), not introduced(C).
{ true(X) : guessable(X) }.
item(X) :- true(X).

% Add enforced clauses (i.e. clauses made true by the guess)
item(C) :- current(C;A), pos(C,A), true(A).
item(C) :- current(C;A), neg(C,A), not true(A).

noMatch(R) :- childRow(R,_), removed(C), clause(C), not childItem(R,C). % Killed rows do not match
noMatch(R) :- childRow(R,_), current(X), childItem(R,X), not true(X).
noMatch(R) :- childRow(R,_), not introduced(X), not childItem(R,X), true(X).

match :- childRow(R,_), not noMatch(R).
:- not match.

#hide.
#show item/1.
