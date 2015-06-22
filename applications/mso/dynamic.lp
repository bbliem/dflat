%dflat: -e edge -d td -n semi --no-empty-leaves --no-empty-root
% This encoding requires semi-normalization

var(L,X) :- iVar(L,X).
var(L,X) :- sVar(L,X).
clause(C) :- pos(C,_).
clause(C) :- neg(C,_).
atom(A) :- pos(_,A).
atom(A) :- neg(_,A).

% Guess a branch for each child i-tree
extend(0,R) :- root(R).
1 { extend(L+1,S) : sub(R,S) } 1 :- extend(L,R), sub(R,_).

% Preserve and extend assignment
{ assign(X,V) : var(_,X) } :- introduced(V).
assign(X,V)   :- extend(_,S), childItem(S,assign(X,V)), not removed(V).
assign(X,_nn) :- extend(L,S), childItem(S,assign(X,V)),     removed(V), iVar(L,X).
assigned(X) :- iVar(_,X), assign(X,_).
incomplete(L) :- iVar(L,X), not assigned(X).

% Check: Only join compatible branches; the resulting assignment must be valid
:- iVar(L,X), assign(X,V), assign(X,W), V != W.
:- extend(L,S), extend(L,T), atNode(S,NS), atNode(T,NT), NS > NT, childItem(S,assign(X,_nn)), childItem(T,assign(X,_nn)).
:- extend(L,S), extend(L,T), var(L,X), childItem(S,assign(X,V)), not childItem(T,assign(X,V)), vertex(V).

% Truth values of atoms
true (X) :- extend(_,S), childItem(S,true (X)).
false(X) :- extend(_,S), childItem(S,false(X)).

true (edge(X,Y)) :- atom(edge(X,Y)), assign(X,V), assign(Y,W), edge(V,W).
false(edge(X,Y)) :- atom(edge(X,Y)), assign(X,V), assign(Y,W), vertex(V), vertex(W), not edge(V,W).
wasAssigned(X) :- iVar(L,X), extend(L,S), childItem(S,assign(X,_)).
false(edge(X,Y)) :- atom(edge(X,Y)), extend(_,S), childItem(S,assign(X,V)), removed(V), not wasAssigned(Y).
false(edge(X,Y)) :- atom(edge(X,Y)), extend(_,S), childItem(S,assign(Y,V)), removed(V), not wasAssigned(X).
% The following rule should take care of the others deriving false(edge(X,Y)).
% It is slower, though. Useful for the paper since it is shorter and more consistent with the semantic tree approach.
%false(edge(X,Y)) :- atom(edge(X,Y)), assign(X,_nn), assign(Y,_nn), not true(edge(X,Y)).

true (in(X,Y))   :- atom(in(X,Y)), assign(X,V), assign(Y,V).
false(in(X,Y))   :- atom(in(X,Y)), assign(X,V), vertex(V), not assign(Y,V).

% Truth values of clauses
possiblyTrue(C) :- pos(C,A), not false(A).
possiblyTrue(C) :- neg(C,A), not true (A).
reject :- clause(C), not possiblyTrue(C).

trueClause(C) :- pos(C,A), true(A).
trueClause(C) :- neg(C,A), false(A).
notAllTrue :- clause(C), not trueClause(C).
accept :- not notAllTrue.

% Declare resulting item sets
item(L,assign(X,V)) :- var(L,X), assign(X,V).
item(L,true (X))    :- length(L), true (X).
item(L,false(X))    :- length(L), false(X).



#show length/1.
#show item/2.
#show extend/2.
#show or/1.
#show and/1.
#show accept/0.
#show reject/0.
