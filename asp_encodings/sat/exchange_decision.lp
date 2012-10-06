% Guess truth value for each bag element except introduced clauses since these can only be true because they are enforced by the guess
guessable(A) :- current(A), atom(A).
guessable(C) :- current(C), clause(C), not introduced(C).
1 { guess(X,true), guess(X,false) } 1 :- guessable(X).
map(X,true) :- guess(X,true).

% Add enforced clauses (i.e. clauses made true by the guess)
map(C,true) :- pos(C,A), current(C), guess(A,true).
map(C,true) :- neg(C,A), current(C), guess(A,false).

% Make sure each vertex has an assignment: Anything not assigned yet is assumed to be false.
map(X,false) :- current(X), not map(X,true).

noMatch(I) :- childTuple(I), mapped(I,C,false), removed(C), clause(C). % Killed tuples do not match
noMatch(I) :- childTuple(I), mapped(I,X,V1), guess(X,V2), V1 != V2.

match :- childTuple(I), not noMatch(I).
:- not match.



#hide.
#show map/2.
