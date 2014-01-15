% This encoding requires semi-normalization

var(L,X) :- iVar(L,X).
var(L,X) :- sVar(L,X).
clause(C) :- pos(C,_).
clause(C) :- neg(C,_).
atom(A) :- pos(_,A).
atom(A) :- neg(_,A).

% 3-Col example. TODO put into separate file {{{
% \exists R, G, B:
% \forall x, y:
% (R(x) | G(x) | B(x)) &
% (-R(x) | -G(x)) &
% (-R(x) | -B(x)) &
% (-G(x) | -B(x)) &
% (-edge(x,y) | -R(x) | -R(y)) &
% (-edge(x,y) | -R(x) | -G(y)) &
% (-edge(x,y) | -G(x) | -B(y))

% XXX In the paper we required a more restrictive normal form
length(2).
or(0).
sVar(1,r).
sVar(1,g).
sVar(1,b).
and(1).
iVar(2,x).
iVar(2,y).

pos(c1,in(x,r)).
pos(c1,in(x,g)).
pos(c1,in(x,b)).
neg(c2,in(x,r)).
neg(c2,in(x,g)).
neg(c3,in(x,r)).
neg(c3,in(x,b)).
neg(c4,in(x,g)).
neg(c4,in(x,b)).
neg(c5,edge(x,y)).
neg(c5,in(x,r)).
neg(c5,in(y,r)).
neg(c6,edge(x,y)).
neg(c6,in(x,g)).
neg(c6,in(y,g)).
neg(c7,edge(x,y)).
neg(c7,in(x,b)).
neg(c7,in(y,b)).
% }}}
% Guess a branch for each child i-tree
extend(0,R) :- root(R).
1 { extend(L+1,S) : sub(R,S) } 1 :- extend(L,R), sub(R,_).

% In the root, each individual variable must have been assigned
assigned(X) :- iVar(L,X), extend(L,S), childItem(S,assign(X,_)).
:- final, iVar(_,X), not assigned(X).

% Preserve and extend assignment
{ assign(X,V) : var(_,X) } :- introduced(V).
assign(X,V)   :- extend(_,S), childItem(S,assign(X,V)), not removed(V).
assign(X,_nn) :- extend(L,S), childItem(S,assign(X,V)),     removed(V), iVar(L,X).

% Check: Only join compatible branches; the resulting assignment must be valid
:- iVar(L,X), assign(X,V;X,W), V != W.
:- extend(L,S;L,T), S != T, childItem(S,assign(X,_nn);T,assign(X,_nn)).
:- extend(L,S;L,T), var(L,X), childItem(S,assign(X,V)), not childItem(T,assign(X,V)), vertex(V).

% Truth values of atoms
true (X) :- extend(_,S), childItem(S,true (X)).
false(X) :- extend(_,S), childItem(S,false(X)).

true (edge(X,Y)) :- atom(edge(X,Y)), assign(X,V;Y,W), edge(V,W).
false(edge(X,Y)) :- atom(edge(X,Y)), assign(X,V;Y,W), vertex(V;W), not edge(V,W).
false(edge(X,Y)) :- atom(edge(X,Y)), extend(_,S), childItem(S,assign(X,V)), removed(V), not assigned(Y).
false(edge(X,Y)) :- atom(edge(X,Y)), extend(_,S), childItem(S,assign(Y,V)), removed(V), not assigned(X).

true (in(X,Y))   :- atom(in(X,Y)), assign(X,V), assign(Y,V).
false(in(X,Y))   :- atom(in(X,Y)), assign(X,V), vertex(V), not assign(Y,V).

% Truth values of clauses
possiblyTrue(C) :- pos(C,A), not false(A).
possiblyTrue(C) :- neg(C,A), not true (A).
reject :- clause(C), not possiblyTrue(C).

% TODO no effect yet
%trueClause(C) :- pos(C,A), true(A).
%trueClause(C) :- neg(C,A), false(A).
%notAllTrue :- clause(C), not trueClause(C).
%accept :- not notAllTrue.

% Declare resulting item sets
item(L,assign(X,V)) :- var(L,X), assign(X,V).
item(L,true (X))    :- length(L), true (X).
item(L,false(X))    :- length(L), false(X).



#show length/1.
#show item/2.
#show extend/2.
#show or/1.
#show and/1.
%#show accept/0.
#show reject/0.
