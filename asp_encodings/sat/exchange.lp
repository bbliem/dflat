% Which tuple to use?
nonLeaf :- childTuple(I).
1 {chosenChildTuple(I) : childTuple(I)} 1 :- nonLeaf.
% Some child tuples cannot be used since they are killed
:- clause(C), removed(C), chosenChildTuple(I), not mapped(I,C,true).

% True atoms and satisfied clauses remain so unless removed
map(X,true) :- chosenChildTuple(I), mapped(I,X,true), current(X).

% Guess truth value of introduced atoms
1 { map(A,true), map(A,false) } 1 :- atom(A), introduced(A).
% Through the guess, clauses may become satisfied
map(C,true) :- pos(C,A), current(C), map(A,true).
map(C,true) :- neg(C,A), current(C), map(A,false).

% Make sure each vertex has an assignment: Anything not assigned yet is assumed to be false.
map(X,false) :- current(X), not map(X,true).
% TODO: Is it faster to use 1 {map(X,true), map(X,false)} 1 :- current(X).?



#hide.
#show map/2.
#show chosenChildTuple/1.
