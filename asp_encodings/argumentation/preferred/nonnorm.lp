%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% preferred semantics                     %
% using nonnormalized tree decompositions %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Use two levels.
levels(2).

% Extend exactly one child row per child node.
1 { extend(0, R) : childRow(R, N) } 1 :- childNode(N).
% Extend level 1 if there exists an extension at level 0 and at least one row at level 1.
0 { extend(1, I) : sub(R, I)      } 1 :- extend(0, R).

% This holds if there exists at least one extension at level 1.
extendsOne(I) :- extend(1, R), sub(I, R). 
extendsOne    :- extend(1, _).
hasChildItems(I)   :- childItem(I, _).

% This predicate indicates whether a childItem at level L exists which maps A to V.
hasChild(L, A, V)  :- childItem(I, map(A, V)), extend(L, I).
% If a particular node doesn't consist of an extension at level 1, use mapping of level 0.
hasChild(1, A, V)  :- childItem(I, map(A, V)), extend(0, I), not extendsOne(I).
removedChild(L, V) :- hasChild(L, A, V), removed(A).

% This holds whenever an item is in-mapped at level L and a particular child is marked differently.
diffColor(L) :- current(A), item(L, map(A, in)), hasChild(L, A, T), T != in.

% In-Items must not attack each other, it is therefore considered conflicting if they do so.
conflict(L)  :- item(L, map(A1, in)), item(L, map(A2, in)), att(A1, A2). 

% Compute at level 0 and also at level 1 if it is not conflicting, there aren't different colors and ..
comp(0).
% .. there doesn't exist an extension, but more elements are marked in or
comp(1)      :- not conflict(1), not diffColor(1), not extendsOne, in(1, A), not in(0, A).
% .. there exists a nonempty extension and no attacking child gets removed
comp(1)      :- not conflict(1), not diffColor(1), extendsOne, not removedChild(1, att).

% Some more user-friendly predicate as substitution for hasChild.
child(L, A, V) :- current(A), hasChild(L, A, V), comp(L).

% For every new argument guess if it is in xor out.
attacked(A) :- att(_, A).
1 { in(0, A), out(0, A) } 1 :- introduced(A), attacked(A).
% No guess necessary.
in(0, A) :- introduced(A), not attacked(A).

1 { in(1, A), out(1, A) } 1 :- introduced(A), not in(0, A).
in(1, A) :- in(0, A).
item(L, map(A, in))  :- in(L, A), comp(L).

% An argument is defeated if it is not in the set and ..
% ..just introduced and attacked by an in-argument
item(L, map(A, def)) :- out(L, A), item(L, map(A2, in)), att(A2, A), comp(L).
% ..or has already been introduced and is attacked by an in-argument.
item(L, map(A, def)) :- child(L, A, T), T != in, item(L, map(A2, in)), att(A2, A).

% An argument attacks if it is not in the set, not marked def and ..
% ..just introduced and attacks an in-argument
item(L, map(A, att)) :- out(L, A), item(L, map(A2, in)), att(A, A2), not item(L, map(A, def)), comp(L).
% ..or has already been introduced and attacks an in-argument.
item(L, map(A, att)) :- child(L, A, T), T != def, T != in, item(L, map(A2, in)), att(A, A2), not item(L, map(A, def)).

% An argument is out if it is not marked def/att.
item(L, map(A, out)) :- out(L, A), not item(L, map(A, def)), not item(L, map(A, att)), comp(L).

% Copy already decided in-/def-mapping.
item(L, map(A, T))   :- child(L, A, T), T != att, T != out.
% Copy att-mapping if not mapped to def.
item(L, map(A, att)) :- child(L, A, att), not item(L, map(A, def)).
% Copy out-mapping if not mapped to def/att.
item(L, map(A, out)) :- child(L, A, out), not item(L, map(A, def)), not item(L, map(A, att)).

% Discard sets if any attacking argument is removed.
:- removedChild(0, att).

% Discard sets if there is a conflict.
:- conflict(0).

% Discard sets if the extended child rows exclude a node from the set, but the same node also is in the set.
:- diffColor(0).

% Discard sets if any valid better solution was found at the root node.
hasAtt(I) :- childItem(I, map(_, att)).
:- root, extend(0, I), hasChildItems(C), sub(I, C), not hasAtt(C).

% Discard sets if empty set was extended.
:- extend(1, I), not hasChildItems(I).

% Discard sets if level 1 was extended but not computable.
:- extendsOne, not comp(1).

% Provide empty counterexample
item(1, counterexample) :- comp(1).

#hide.
#show item/2.
#show extend/2.
#show levels/1.



