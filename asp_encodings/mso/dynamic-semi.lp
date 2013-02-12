%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% PREPARATION
% Label the nodes of the children's semantic trees.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
itemSet(0,R) :- root, childRow(R,_).
itemSet(L+1,S) :- itemSet(L,R), sub(R,S).

% Leaves can be evaluated right away (by means of true/1), so they get labeled neither "forall" nor "exists".
% In particular, an "exists" item set is bad only if it has children and all of them are bad, so we do not label leaves with "exist".
forall(S) :- root, childRow(S,_), sub(S,_).
forall(S) :- exists(R), sub(R,S), sub(S,_).
exists(S) :- forall(R), sub(R,S), sub(S,_).

invalid(S) :- iVar(L,X), itemSet(L,S), not assignedIn(X,S).
assignedIn(X,S) :- childItem(S,assign(X,_)).

bad(S) :- levels(L), itemSet(L-1,S), not invalid(S), clause(C), not childItem(S,true(C)).
bad(S) :- forall(S), not invalid(S), sub(S,T), bad(T).
bad(S) :- exists(S), not invalid(S), not good(S).
good(S) :- exists(S), sub(S,T), not invalid(T), not bad(T).


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% GUESS
% Guess a branch of each child's semantic tree such that the branch contains no
% "invalid" or "bad" vertices.  Then guess an assignment of introduced input
% graph vertices to variables.  Retain the information from the guessed branch
% that concerns still-current input graph vertices, and set individual
% variables whose value has been removed to the wildcard "_nn".
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
1 { extend(0,R) : childRow(R,N) } 1 :- childNode(N).
1 { extend(L+1,S) : sub(R,S) } 1 :- extend(L,R), sub(R,_).
:- extend(_,S), bad(S).
:- extend(_,S), invalid(S).

{ assign(X,V) : var(_,X) } :- introduced(V).

assign(X,V) :- extend(_,S), childItem(S,assign(X,V)), not removed(V). % V can also be _nn if X is an individual variable
assign(X,_nn) :- extend(L,S), childItem(S,assign(X,V)), removed(V), iVar(L,X).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% CHECK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% We must not assign an introduced vertex to an already assigned variable. This can also happen for non-introduced vertices if we join incompatible branches.
:- iVar(L,X), assign(X,V;W), V < W.

% If an individual variable is set to a vertex in one branch, each join partner must have set the variable to either the same vertex or nothing at all.
:- extend(L,S0;S1), S0 < S1, childItem(S0;S1,assign(X,_nn)).
%:- extend(L,S0;S1), sVar(L,X), childItem(S0,assign(X,V)), not childItem(S1,assign(X,V)).
:- extend(L,S0;S1), var(L,X), childItem(S0,assign(X,V)), not childItem(S1,assign(X,V)), vertex(V).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% EVALUATION
% Determine clauses that evaluate to true so far.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Which individual variables have been assigned some value (possibly _nn)?
assigned(X) :- iVar(L,X), extend(L,S), assignedIn(X,S).

true(C) :- extend(_,S), childItem(S,true(C)).
true(C) :- pos(C,edge(X,Y)), assign(X,V), assign(Y,W), edge(V,W).
true(C) :- neg(C,edge(X,Y)), assign(X,V), assign(Y,W), vertex(V;W), not edge(V,W).
true(C) :- neg(C,edge(X,Y)), extend(_,S), childItem(S,assign(X,V)), removed(V), not assigned(Y).
true(C) :- neg(C,edge(X,Y)), extend(_,S), childItem(S,assign(Y,V)), removed(V), not assigned(X).
true(C) :- pos(C,in(X,Y)), assign(X,V), assign(Y,V).
true(C) :- neg(C,in(X,Y)), assign(X,V), vertex(V), not assign(Y,V).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% DECLARATION OF RESULTING BRANCH
% Declare resulting item sets.
% "assign(X,V)" means that the variable X has been assigned the value V, where
% V can also be "_nn".
% "true(C)" means that the clause C is true.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

item(L,assign(X,V)) :- var(L,X), assign(X,V).
item(L-1,true(C)) :- levels(L), true(C).

#hide.
#show item/2.
#show extend/2.
#show levels/1.
