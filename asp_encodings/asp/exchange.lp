map(0,X,true) :- m(X).
map(1,X,true) :- c(X).
% Assign false to anything not true
map(0,X,false) :- current(X), not map(0,X,true).
map(1,X,false) :- cExists, current(X), not map(1,X,true).

% Which tuple to use?
1 { chosenChildTuple(I) : childTuple(I) } 1 :- childTuple(_).

%%%%%%%%%%%%%%%%%%%%%
% Atom introduction %
%%%%%%%%%%%%%%%%%%%%%
1 { true(A), false(A) } 1 :- atom(A), introduced(A).
m(A) :- true(A).
m(R) :- head(R,A), current(R), true(A).
m(R) :- neg(R,A), current(R), true(A).
m(R) :- pos(R,A), current(R), false(A).

% There can be a C originating from...
% a) the old M in case something was guessed to true, or
cExists :- true(A).
% b) an old C that does not get killed because there is a removed rule that it does not contain
killedOldC(C) :- sub(_,C), removed(R), rule(R), mapped(C,R,false).
cExists :- chosenChildTuple(M), sub(M,C), not killedOldC(C).
% If there is one, we can obtain a C by either one of the following:
% a) From M' by adding any proper subset of {a | true(a)}
% b) From any C' by adding any subset of {a | true(a)}
% Also, we need to add thereby satisfied rules of the reduct

% a) or b)?
0 { chosenOldC(C) : sub(M,C) : not killedOldC(C) } 1 :- cExists, chosenChildTuple(M).
cFromOldC :- chosenOldC(_).
cFromOldM :- cExists, not cFromOldC.

% Guess a subset of {a | true(a)}
cFalse(A) :- false(A), cExists.
1 {cTrue(A), cFalse(A)} 1 :- true(A).

% Add atoms guessed to true
c(A) :- cTrue(A).
% Add rules of the reduct that got satisfied
c(R) :- head(R,A), current(R), cTrue(A).
c(R) :- pos(R,A), current(R), cFalse(A).

% In case of a), the guessed subset must be proper
cTrueIsProperSubset :- true(A), not cTrue(A).
:- cFromOldM, not cTrueIsProperSubset.

% Add rules that disappear in the reduct and thus are trivially satisfied.
c(R) :- true(A), current(R), neg(R,A).

% Only atoms and rules that are not removed persist
m(X) :- chosenChildTuple(M), mapped(M,X,true), not removed(X).
c(X) :- cFromOldM, chosenChildTuple(M), mapped(M,X,true), not removed(X).
c(X) :- chosenOldC(C), mapped(C,X,true), not removed(X).

%%%%%%%%%%%%%%%%%%%%%
% Rule introduction %
%%%%%%%%%%%%%%%%%%%%%
m(R) :- introduced(R), head(R,A), chosenChildTuple(M), mapped(M,A,true).
m(R) :- introduced(R), neg(R,A), chosenChildTuple(M), mapped(M,A,true).
m(R) :- introduced(R), pos(R,A), chosenChildTuple(M), mapped(M,A,false).

% We need to add an introduced rule r to C if...
% a) r disappears in the reduct
c(R) :- introduced(R), neg(R,A), m(A), cExists.
% b) r is satisfied by C
c(R) :- introduced(R), head(R,A), c(A).
c(R) :- introduced(R), pos(R,A), current(A), not c(A), cExists.

%%%%%%%%%%%%%%%%
% Rule removal %
%%%%%%%%%%%%%%%%
:- rule(R), removed(R), chosenChildTuple(M), mapped(M,R,false).

%%%%%%%%
% Root %
%%%%%%%%
nonRoot :- current(X).
:- not nonRoot, chosenChildTuple(M), sub(M,C), not killedOldC(C).


#hide.
#show map/3.
#show chosenChildTuple/1.
