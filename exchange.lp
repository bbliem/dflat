% TODO: Incorporate (old)mAtom/Rule into the program instead of these auxiliary predicates
oldM(M,A) :- oldMAtom(M,A).
oldM(M,R) :- oldMRule(M,R).
mAtom(X) :- m(X), atom(X).
mRule(X) :- m(X), rule(X).


introduced(X) :- current(X), not before(X).
removed(X) :- before(X), not current(X).

% Which tuple to use?
1 {chosenOldM(I) : oldMi(I)} 1 :- oldMi(X).

% Only atoms and rules that are not removed persist
m(X) :- chosenOldM(M), oldM(M,X), not removed(X).

%%%%%%%%%%%%%%%%%%%%%
% Atom introduction %
%%%%%%%%%%%%%%%%%%%%%
1 {true(A), false(A)} 1 :- atom(A), introduced(A).
m(A) :- true(A).
m(R) :- head(R,A), current(R), true(A).
m(R) :- neg(R,A), current(R), true(A).
m(R) :- pos(R,A), current(R), false(A).

%%%%%%%%%%%%%%%%%%%%%
% Rule introduction %
%%%%%%%%%%%%%%%%%%%%%
m(R) :- introduced(R), head(R,A), m(A).
m(R) :- introduced(R), neg(R,A), m(A).
m(R) :- introduced(R), pos(R,A), current(A), not m(A).

%%%%%%%%%%%%%%%%
% Rule removal %
%%%%%%%%%%%%%%%%
:- rule(R), removed(R), chosenOldM(M), not oldM(M,R).



#hide.
#show mAtom/1.
#show mRule/1.
#show chosenOldM/1.
