% Which tuple to use?
nonLeaf :- childTuple(X).
1 {chosenChildTuple(I) : childTuple(I)} 1 :- nonLeaf.

% Only atoms and clauses that are not removed persist
mAtom(X) :- chosenChildTuple(M), oldMAtom(M,X), not removed(X).
mClause(X) :- chosenChildTuple(M), oldMClause(M,X), not removed(X).

1 {true(A), false(A)} 1 :- atom(A), introduced(A).
mAtom(A) :- true(A).
mClause(C) :- pos(C,A), current(C), true(A).
mClause(C) :- neg(C,A), current(C), false(A).

mClause(C) :- introduced(C), pos(C,A), mAtom(A).
mClause(C) :- introduced(C), neg(C,A), current(A), not mAtom(A).

:- clause(C), removed(C), chosenChildTuple(M), not oldMClause(M,C).



#hide.
#show mAtom/1.
#show mClause/1.
#show chosenChildTuple/1.
