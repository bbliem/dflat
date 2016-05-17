%dflat: -e vertex -e edge -d td -n semi --depth 2 --default-join

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% PREPARATION
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Set depth of the item tree to two levels.
length(2).

% Set type of the nodes on level 0 to 'or'.
% This statement is used to allow the enumeration of all solutions.
or(0).

% Set type of the nodes on level 1 to 'and'.
% This statement ensures that none of the extended rows on level 2
% contains the fact 'reject'. If only a single row fails this
% check, the whole row on the top level is rejected.
and(1).

% Ensure that all the edges are undirected.
edge(Y,X) :- current(X;Y), edge(X,Y).

edge(Y,X) :- current(X), removed(Y), edge(X,Y).
edge(Y,X) :- current(Y), removed(X), edge(X,Y).
edge(Y,X) :- removed(X), removed(Y), edge(X,Y).


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% GUESS
% Guess a branch of each child's semantic tree such that the branch contains no
% "invalid" or "bad" vertices. Then guess a subset of introduced input graph
% vertices (inS) as part of the potentially secure set on level 1. Afterwards,
% guess a subset of the newly introduced vertices which are part of inS to
% form a sub-subset (inX).
% Retain the information from the guessed branch that concerns still-current
% input graph vertices, and set individual and update all the important
% values for the checking part.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Guess exactly one row for each child node which should be extended on level 0.
1 { extend(0,R) : rootOf(R,N) } 1 :- childNode(N).

% Guess exactly one row which should be extended for each level.
1 { extend(L+1,S) : sub(R,S) } 1 :- extend(L,R), sub(R,_).

% Guess a subset of the newly introduced vertices which extend
% the potentially secure set S which we want to check if it is
% really secure.
{ item(1,inS(V)) } :- introduced(V).

% Guess a subset of the newly introduced extensions to inS which
% extends the subset X which we want to investigate on level 2
% of the item tree.
{ item(2,inX(V)) } :- introduced(V), item(1,inS(V)).

% Retain the information about the subsets under investigation.
item(1,inS(V)) :- extend(1,R), current(V), childItem(R,inS(V)).
item(2,inX(V)) :- extend(2,R), current(V), childItem(R,inX(V)).

% Retain the information about attackers and defenders.
auxItem(2,globalDefender(V)) :- extend(2,R), current(V), childAuxItem(R,globalDefender(V)).
auxItem(2,globalAttacker(V)) :- extend(2,R), current(V), childAuxItem(R,globalAttacker(V)).

% Each vertex in X defends itself and is defended by its neighbors in S.
auxItem(2,globalDefender(V)) :- current(V), item(2,inX(V)).
auxItem(2,globalDefender(Y)) :- current(Y), item(2,inX(X)), edge(X,Y), item(1,inS(Y)).

% Each vertex in X is attacked by its neighbors which are not an element of S.
auxItem(2,globalAttacker(Y)) :- current(Y), item(2,inX(X)), edge(X,Y), not item(1,inS(Y)).

% Initialize the fitness counter for introduced vertices in S.
counter(fitness(V),0) :- introduced(V), item(1,inS(V)).


% Initialize the global fitness counter to 0 when we are in a leaf node of the tree decomposition. To
% determine if the current node is really a leaf node, we check if the count of child nodes is 0.
counter(globalFitness,0) :- 0 #count { CN : childNode(CN) } 0.

% When the current node is an exchange node (the number of extended rows is 1 in
% this case), then we have to increasethe counters by the number of attackers and decrease it by the number of
% defenders accordingly.
defender(X,Y) :- extend(R), item(inX(X)), edge(X,Y), childItem(R,inS(Y)).
attacker(X,Y) :- extend(R), item(inX(X)), edge(X,Y), not childItem(R,inS(Y)).

counter(fitness(V),CF + NF) :- current(V), extend(2,R), childCounter(R,fitness(V),CF),
                              NF = #sum { 1,V,X : defender(V,X), removed(X);
                                         -1,V,X : attacker(V,X), removed(X) }.

counter(globalFitness,CF + NF) :- extend(2,R), childCounter(R,globalFitness,CF),
                              NF = #sum { 1,V : childAuxItem(R,globalDefender(V)), removed(V);
                                         -1,V : childAuxItem(R,globalAttacker(V)), removed(V) }.

%Remove fitness counter when its corresponding vertex was removed from the bag.
counterRem(fitness(V)) :- removed(V).


% Initialize the current counters for fitness and the one for global fitness.
currentCounter(fitness(V),0) :- counter(fitness(V),_).
currentCounter(globalFitness,0).

% Initialize cardinality of a leaf node to 0.
counter(cardinality,0) :- 0 #count { CN : childNode(CN) } 0.

% The cardinality of a exchange node is simply the sum of the child costs and the count of introduced input graph vertices which are part of S.
counter(cardinality,CC + C) :- extend(2,R), childCounter(R,cardinality,CC), C = #count { V : item(1,inS(V)), introduced(V) }.

% Define the cardinality current counter as being the count of current input graph vertices which are part of S.
currentCounter(cardinality,LC) :- LC = #count { V : item(1,inS(V)), current(V) }.

% Define the cost as being equivalent to the cardinality.
counter(cost,C) :- counter(cardinality,C).
currentCounter(cost,C) :- currentCounter(cardinality,C).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% CHECK
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Add the fact 'reject' to the resulting row on level 2 if the condition of a
% secure set - The number of attackers must be less or equal to the number of
% defenders. - is violated in order to ensure that the corresponding solution
% candidate on level 0 (the potentially secure set S) is deleted.
% This rule can be checked only in the final tree decomposition node.
reject :- final, counter(globalFitness,F), F < 0.

% Add the fact '_reject' to the resulting row on level 2 if the condition of a
% secure set - The number of attackers must be less or equal to the number of
% defenders. - is violated in order to ensure that the corresponding solution
% candidate on level 0 (the potentially secure set S) is deleted.
% This rule can be checked in all tree decomposition nodes.
reject :- extend(2,R), removed(V),
          childItem(R,inX(V)),
          childCounter(R,fitness(V),CF),
          NF = #sum { 1,V,X : defender(V,X), current(X);
                      1,V,X : defender(V,X), removed(X);
                     -1,V,X : attacker(V,X), current(X);
                     -1,V,X : attacker(V,X), removed(X) },
          CF + NF + 1 < 0.

% Add the fact '_reject' to the resulting row on level 2 if S is empty
% This rule can be checked only in the final tree decomposition node.
reject :- final, childCounter(R,cardinality,0), extend(2,R).

% Add the fact '_accept' to the resulting row on level 2 if we are in the final tree decomposition node and the fact 'reject' has not been added.
accept :- final, not reject.
