%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% stable semantics                        %
% using nonnormalized tree decompositions %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% Exactly extend one child row per child node.
1 { extend(R) : childRow(R, N) } 1 :- childNode(N).

%% For every introduced argument guess if it is IN xor out/DEF.
{ item(map(A, in)) } :- introduced(A), attacked(A).
attacked(A) :- att(_, A).
item(map(A, in)) :- introduced(A), not attacked(A).

%% An argument is defended if it is not IN the set and ..
%% ..just introduced and attacked by an IN-argument
item(map(A, def)) :- introduced(A), not item(map(A, in)), item(map(A2, in)), att(A2, A).
%% ..or attacked by an IN-argument somewhat above in the decomposition.
item(map(A, def)) :- current(A), not childItem(I, map(A, in)), item(map(A2, in)), att(A2, A), extend(I).

%% Copy already decided IN-/DEF-mappings.
item(map(A, in))  :- current(A), childItem(I, map(A, in)),  extend(I).
item(map(A, def)) :- current(A), childItem(I, map(A, def)), extend(I).

%% Discard sets if any attacking or out-argument is removed.
:- removed(A), not childItem(I, map(A, in)), not childItem(I, map(A, def)), extend(I), childRow(I, N), childBag(N, A).

%% Discard sets if there is a conflict.
:- item(map(A1, in)), item(map(A2, in)), att(A1, A2).

%% Discard sets if joined rows are mapping pairwise different and at least one IN-mapping is included.
:- extend(I), item(map(A, in)), not childItem(I, map(A, in)), current(A), childRow(I, N), childBag(N, A).

#hide.
#show item/1.
#show extend/1.



