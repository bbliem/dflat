%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% admissible semantics                    %
% using nonnormalized tree decompositions %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% Exactly extend one child row per child node.
1 { extend(R) : childRow(R, N) } 1 :- childNode(N).

%% For every introduced argument guess if it is IN xor out.
1 { item(map(A, in)), out(A) } 1 :- introduced(A).

%% An argument is DEFended if it is not IN the set and ..
%% ..just introduced and attacked by an IN-argument
item(map(A, def)) :- out(A), item(map(A2, in)), att(A2, A).
%% ..or attacked by an IN-argument somewhat above in the decomposition.
item(map(A, def)) :- current(A), not childItem(I, map(A, in)), item(map(A2, in)), att(A2, A), extend(I).

%% An argument ATTacks if it is not IN the set, not marked DEF and ..
%% ..just introduced and attacks an IN-argument
item(map(A, att)) :- out(A), item(map(A2, in)), att(A, A2), not item(map(A, def)).
%% ..or attacks an IN-argument somewhat above in the decomposition.
item(map(A, att)) :- current(A), not childItem(I, map(A, in)), item(map(A2, in)), att(A, A2), not item(map(A, def)), extend(I).

%% Copy already decided IN-/DEF-mappings.
item(map(A, in))  :- current(A), childItem(I, map(A, in)),  extend(I).
item(map(A, def)) :- current(A), childItem(I, map(A, def)), extend(I).
%% Copy ATT-mapping if not mapped to DEF.
item(map(A, att)) :- current(A), childItem(I, map(A, att)), not item(map(A, def)), extend(I).

%% Discard sets if any ATTacking argument is removed.
:- removed(A), childItem(I, map(A, att)), extend(I).

%% Discard sets if there is a conflict.
:- item(map(A1, in)), item(map(A2, in)), att(A1, A2).

%% Discard sets if joined rows are mapping pairwise different and at least one IN-mapping is included.
:- extend(I), item(map(A, in)), not childItem(I, map(A, in)), current(A), childRow(I, N), childBag(N, A).

#hide.
#show item/1.
#show extend/1.



