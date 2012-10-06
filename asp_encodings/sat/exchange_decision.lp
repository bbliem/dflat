clause(C) :- pos(C,_).
clause(C) :- neg(C,_).
atom(A) :- pos(_,A).
atom(A) :- neg(_,A).

% Guess truth value for each bag element except introduced clauses since these can only be true because they are enforced by the guess
guessable(A) :- current(A), atom(A).
guessable(C) :- current(C), clause(C), not introduced(C).
1 { map(X,true), map(X,false) } 1 :- guessable(X).

% Add enforced clauses (i.e. clauses made true by the guess)
map(C,true) :- pos(C,A), current(C), map(A,true).
map(C,true) :- neg(C,A), current(C), map(A,false).

% Make sure each vertex has an assignment: Anything not assigned yet is assumed to be false.
map(X,false) :- current(X), not map(X,true).

noMatch(I) :- childTuple(I), mapped(I,C,false), removed(C), clause(C). % Killed tuples do not match
noMatch(I) :- childTuple(I), mapped(I,X,V1), map(X,V2), V1 != V2. % Maybe not worse than the next two commented-out lines?
%noMatch(I) :- map(X,true), childTuple(I), mapped(I,X,false). % What was guessed to true cannot have been false before
%noMatch(I) :- map(X,false), childTuple(I), mapped(I,X,true). % What guessed to false cannot have been true before

nonLeaf :- childTuple(_).
match :- childTuple(I), not noMatch(I).
:- nonLeaf, not match.



#hide.
#show map/2.
