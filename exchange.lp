introduced(X) :- current(X), not before(X).
removed(X) :- before(X), not current(X).

% Which tuple to use?
nonLeaf :- oldMi(X).
1 {chosenOldM(I) : oldMi(I)} 1 :- nonLeaf.

% Only atoms and rules that are not removed persist
mAtom(X) :- chosenOldM(M), oldMAtom(M,X), not removed(X).
mRule(X) :- chosenOldM(M), oldMRule(M,X), not removed(X).

%%%%%%%%%%%%%%%%%%%%%
% Atom introduction %
%%%%%%%%%%%%%%%%%%%%%
1 {true(A), false(A)} 1 :- atom(A), introduced(A).
mAtom(A) :- true(A).
mRule(R) :- head(R,A), current(R), true(A).
mRule(R) :- neg(R,A), current(R), true(A).
mRule(R) :- pos(R,A), current(R), false(A).

%%%%%%%%%%%%%%%%%%%%%
% Rule introduction %
%%%%%%%%%%%%%%%%%%%%%
mRule(R) :- introduced(R), head(R,A), mAtom(A).
mRule(R) :- introduced(R), neg(R,A), mAtom(A).
mRule(R) :- introduced(R), pos(R,A), current(A), not mAtom(A).

%%%%%%%%%%%%%%%%
% Rule removal %
%%%%%%%%%%%%%%%%
:- rule(R), removed(R), chosenOldM(M), not oldMRule(M,R).



#hide.
#show mAtom/1.
#show mRule/1.
#show chosenOldM/1.
