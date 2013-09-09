%%% D-FLAT 0.2 Encoding for the Dalal Operator %%%
%
% Usage: >dflat -p opt-enum -e pos -e neg dalal-dynamic.lp < instance.lp
%
% Data structure: each bag assignment consists of 
% ma/1, mb/1 and cl/1 predicates, denoting the atoms contained in ma and mb
% as well as the satisfied clauses of the current bag

% Make explicit that an atom or clause is not satisified in a child
false(R,ma(X)) :- childRow(R,N), childBag(N,X), not childItem(R,ma(X)).
false(R,mb(X)) :- childRow(R,N), childBag(N,X), not childItem(R,mb(X)).
false(R,cl(X)) :- childRow(R,N), childBag(N,X), not childItem(R,cl(X)).

% For each child node exactly one row (ie bag assignment) is guessed
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).

% Only join rows that coincide on common atoms in both ma and mb
:- extend(X;Y), atom(A), childItem(X,ma(A)), false(Y,ma(A)).
:- extend(X;Y), atom(A), childItem(X,mb(A)), false(Y,mb(A)).

% Do not extend row in case an unsatisfied clause is removed
:- clausea(C), removed(C), extend(R), false(R,cl(C)).
:- clauseb(C), removed(C), extend(R), false(R,cl(C)).

% Keep atoms and satisfied clauses if not removed
item(ma(X)) :- extend(R), childItem(R,ma(X)), current(X). 
item(mb(X)) :- extend(R), childItem(R,mb(X)), current(X).
item(cl(X)) :- extend(R), childItem(R,cl(X)), current(X).

% Guess truth value for every introduced atom
{ item(ma(A)) : atom(A) : introduced(A) }.
{ item(mb(A)) : atom(A) : introduced(A) }.

% add clause to clause set in case it is satisfied
item(cl(C)) :- clausea(C), current(C;A), pos(A,C),     item(ma(A)).
item(cl(C)) :- clausea(C), current(C;A), neg(A,C), not item(ma(A)).
item(cl(C)) :- clauseb(C), current(C;A), pos(A,C),     item(mb(A)).
item(cl(C)) :- clauseb(C), current(C;A), neg(A,C), not item(mb(A)).


% Compute model distance
deltaContains(X) :- item(ma(X)), not item(mb(X)).
deltaContains(X) :- item(mb(X)), not item(ma(X)).

% Compute current costs (cardinality of model distance)
currentC(C) :- C = #count { deltaContains(X) }.

% Compute sum of costs from children
sumChildrenC(S) :- S = #sum [ extend(R) : childCost(R,C) = C ].

% Compute costs that are counted several times 
childDup(R,A) :- current(A), extend(R), not childItem(R, ma(A)), childItem(R, mb(A)).
childDup(R,A) :- current(A), extend(R), childItem(R, ma(A)), not childItem(R, mb(A)).
subtractDup(D) :- D = #count { childDup(_,_) }.

% Overall costs
cost(S - D + C) :- sumChildrenC(S), subtractDup(D), currentC(C). 


%%% Decision Problems %%%
%
% Usage: >dflat -p opt-counting -e pos -e neg dalal-dynamic.lp < instance.lp
% Output for CRED decision Problem:
%  For optimal solutions >= 1, cred = true
% Usage for SKEPT decision problem:
%  For optimal solutions == 0, skept = true
%
% Uncomment one of these facts for deciding credulous/skeptical acceptance
%computecred.
%computeskept.

decision :- computecred.
decision :- computeskept.

% Decision Problem (Cred)
item(cred) :- computecred, introduced(A), atom(A), gamma(C), pos(A,C), item(mb(A)).
item(cred) :- computecred, introduced(A), atom(A), gamma(C), neg(A,C), not item(mb(A)).
item(cred) :- computecred, extend(R), childItem(R,cred).

% Decision Problem (Skept)
item(notskept) :- computeskept, introduced(A), atom(A), gamma(C), pos(A,C), not item(mb(A)).
item(notskept) :- computeskept, introduced(A), atom(A), gamma(C), neg(A,C), item(mb(A)).
item(notskept) :- computeskept, extend(R), childItem(R,notskept).

% At root node manually compute all child rows that are kept (root node is always removal node!)
removedAtRoot(R) :- decision, root, clausea(C), removed(C), childRow(R,N), false(R,cl(C)).
removedAtRoot(R) :- decision, root, clauseb(C), removed(C), childRow(R,N), false(R,cl(C)).

% Compute minimal costs over all kept child rows
costsAtRoot(R,C)       :- decision, root, childRow(R,N), childCost(R,C), not removedAtRoot(R).
greaterCostsAtRoot(R1) :- decision, root, costsAtRoot(R1,C1), costsAtRoot(R2,C2), C1 > C2.
hasMinCostsAtRoot(R)   :- decision, root, costsAtRoot(R,C), not greaterCostsAtRoot(R).

% And remove all that are not minimal
:- decision, root, extend(R), not hasMinCostsAtRoot(R).

% Remove all that do not contain true (cred)
:- computecred, root, not item(cred).

% Remove all entries which are not counter examples for skept acceptance
:- computeskept, root, not item(notskept).


#hide.
#show item/1.
#show extend/1.
#show cost/1.

