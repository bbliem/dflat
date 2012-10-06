% Suppose that the top level quantifier is \exists

% Predicates computed by the grounder...

itemSet(R,0,N) :- childRow(R,N).
itemSet(S,L+1,N) :- itemSet(R,L,N), sub(R,S).

hasSub(S) :- sub(S,_).

assigned(S,X) :- childItem(S,is(X,_)).
assigned(S,X) :- childItem(S,taken(X)).

removedValueOf(S,X) :- childItem(S,is(X,V)), removed(V).

% The leaf nodes of a characteristic declare which clauses are satisfied.
-leaf(N) :- root, sub(N,_).
leaf(N) :- root, sub(_,N), not -leaf(N).

% A node in the characteristic tree is invalid if it does not assign a variable
invalidItemSet(S) :- root, individualVar(X,L), itemSet(S,L,_), not assigned(S,X).
invalidItemSet(S) :- invalidItemSet(R), sub(R,S).

forallItemSet(R) :- root, childRow(R,_), not invalidItemSet(R).
forallItemSet(S) :- existsItemSet(R), sub(R,S), not leaf(S), not invalidItemSet(S).
existsItemSet(S) :- forallItemSet(R), sub(R,S), not leaf(S), not invalidItemSet(S).

bad(S) :- root, levels(L), itemSet(S,L-1,_), not invalidItemSet(S), clause(C), not childItem(S,satisfied(C)).
-bad(S) :- existsItemSet(S), sub(S,T), not bad(T), not invalidItemSet(T).
bad(S) :- existsItemSet(S), not -bad(S).
bad(S) :- forallItemSet(S), sub(S,T), bad(T), not invalidItemSet(T).

% Actual solving part...

% Guess a path
1 { guessedSet(R,0,N) : childRow(R,N) } 1 :- childNode(N).
1 { guessedSet(S,L+1,N) : sub(R,S) } 1 :- guessedSet(R,L,N), hasSub(R).
extend(L,S) :- guessedSet(S,L,_).

% Joined rows must agree on common vertices, i.e., if a common vertex is assigned to some (resp. no) variables in a row, it must also be assigned to those (resp. no) variables in the matching row. If a common vertex is contained in the interpretation of some set variables, it must also be contained in those sets of the matching row.

:- guessedSet(S0,L,N0), guessedSet(S1,L,N1), N0 != N1, childItem(S0,is(X,V)), childBag(N1,V), not childItem(S1,is(X,V)).
:- guessedSet(S0,L,N0), guessedSet(S1,L,N1), N0 != N1, childItem(S0,contains(X,V)), childBag(N1,V), not childItem(S1,contains(X,V)).

% If an individual variable is set to a vertex in a row, each join partner must have set the variable to either the same vertex or nothing at all.
:- guessedSet(S0,L,N0), guessedSet(S1,L,N1), N0 != N1, childItem(S0,is(X,Y)), childItem(S1,is(X,Z)), Y != Z.
:- guessedSet(S0,L,N0), guessedSet(S1,L,N1), N0 != N1, childItem(S0,is(X,_)), childItem(S1,taken(X)).
:- guessedSet(S0,L,N0), guessedSet(S1,L,N1), N0 != N1, childItem(S0;S1,taken(X)).

% Use item "taken(x)" when an individual variable x has been assigned to a now-removed vertex

satisfied(C)  :- guessedSet(S,_,_), childItem(S,satisfied(C)).
is(X,V)       :- guessedSet(S,_,_), childItem(S,is(X,V)),       current(V).
taken(X)      :- guessedSet(S,_,_), removedValueOf(S,X).
taken(X)      :- guessedSet(S,_,_), childItem(S,taken(X)).
contains(X,V) :- guessedSet(S,_,_), childItem(S,contains(X,V)), current(V).

:- root, extend(0,R), bad(R).
:- root, individualVar(X,_), not taken(X).

% Vertex introduction

{ is(X,V) : individualVar(X,_) } :- introduced(V).
{ contains(X,V) : setVar(X,_) } :- introduced(V).

:- individualVar(X,_), is(X,V), is(X,W), V != W.
:- individualVar(X,_), is(X,_), taken(X).

% assignedByGuess(N,X) means that the guessed row for child node N assigned the individual variable X.
assignedByGuess(N,X) :- guessedSet(S,_,N), assigned(S,X).

% If X is a set variable, x,y are individual variables, the following are atoms:
% 1. x = y
% 2. edge(x,y)
% 3. X(x)

% Which clauses become satisfied by the current row? (Additionally to satisfied clauses "inherited" from extended child rows.)
satisfied(C) :- pos(C,eq(X,Y)), is(X,V), is(Y,V).
satisfied(C) :- neg(C,eq(X,Y)), is(X,V), is(Y,W), V != W.
satisfied(C) :- neg(C,eq(X,Y)), guessedSet(S,_,N), removedValueOf(S,X), not assignedByGuess(N,Y).
satisfied(C) :- neg(C,eq(X,Y)), guessedSet(S,_,N), removedValueOf(S,Y), not assignedByGuess(N,X).

satisfied(C) :- pos(C,edge(X,Y)), is(X,V), is(Y,W), edge(V,W).
satisfied(C) :- neg(C,edge(X,Y)), is(X,V), is(Y,W), not edge(V,W).
satisfied(C) :- neg(C,edge(X,Y)), guessedSet(S,_,N), removedValueOf(S,X), not assignedByGuess(N,Y).
satisfied(C) :- neg(C,edge(X,Y)), guessedSet(S,_,N), removedValueOf(S,Y), not assignedByGuess(N,X).

satisfied(C) :- pos(C,in(X,Y)), is(X,V), contains(Y,V).
satisfied(C) :- neg(C,in(X,Y)), is(X,V), not contains(Y,V).

% When in the root, we do not put any items into the sets to not obtain multiple rows.
item(L-1,satisfied(C)) :- not root, levels(L), satisfied(C).
item(L,is(X,V)) :- not root, individualVar(X,L), is(X,V).
item(L,taken(X)) :- not root, individualVar(X,L), taken(X).
item(L,contains(X,V)) :- not root, setVar(X,L), contains(X,V).



#hide.
#show item/2.
#show extend/2.
% We need to specify the number of levels because for instance in leaves all item sets in this answer set could be empty, therefore no item/2 atom would be in the AS, but also no extend/2 atom would be in the AS because there are no extended rows / subsidiary item sets. We must in this case tell D-FLAT explicitly how many levels the path described by this answer set has.
#show levels/1.
