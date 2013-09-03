%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% complete semantics                      %
% using nonnormalized tree decompositions %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% Exactly extend one child row per child node.
1 { extend(R) : childRow(R, N) } 1 :- childNode(N).

%% For every introduced argument guess if it is IN, defended or out.
%% Doing so prevents dealing with changes from DEF to OUT and vice versa.
1 { item(map(A, in)), def(A), out(A) } 1 :- introduced(A), attacked(A).
attacked(A) :- att(_, A).
item(map(A, in)) :- introduced(A), not attacked(A).

%% OUT- and OUTC-marked childs are considered "out".
out(A) :- current(A), childItem(I, map(A, outc)), extend(I).
out(A) :- current(A), childItem(I, map(A, out)),  extend(I).

%% If edgeTo(A, T) is set, it indicates that any current T-mapped argument attacks argument A.
edgeTo(A, T) :- current(A2), item(map(A2, T)), att(A2, A).

%% An argument is DEF if it is attacked by IN-arguments.
item(map(A, def)) :- childItem(I, map(A, defc)), edgeTo(A, in), extend(I).
item(map(A, def)) :- def(A), edgeTo(A, in).
%% An argument is DEFC if one child node marked it DEFC, but it is not marked DEF yet.
item(map(A, defc)) :- current(A), childItem(I, map(A, defc)), not item(map(A, def)), extend(I).
item(map(A, defc)) :- def(A), not item(map(A, def)).

%% An argument is OUT if it is (already OUT or) marked OUTC and attacked by OUT-/OUTC-arguments.
item(map(A, out)) :- out(A2), out(A), att(A2, A).

%% Copy already decided IN-/DEF-/OUT-mappings.
item(map(A, C)) :- current(A), childItem(I, map(A, C)), C != defc, C != outc, extend(I).
%% Copy OUTC-mapping if it is not changed to OUT.
item(map(A, outc)) :- not item(map(A, out)), out(A).

%% Discard sets if any DEFCandidate or OUTCandidate is removed.
:- removed(A), childItem(I, map(A, defc)), extend(I).
:- removed(A), childItem(I, map(A, outc)), extend(I).

%% Discard sets if there is a conflict.
:- item(map(A1, in)), item(map(A2, in)), att(A1, A2).

%% Discard sets if joined rows are mapping pairwise different and at least one IN-mapping is included.
:- extend(I), item(map(A, in)), childItem(I, map(A, X)), current(A), X != in.
%% Discard sets if joined rows have different DEF-/DEFC-mapping. 
:- extend(I), item(map(A, defc)), childItem(I, map(A, X)), current(A), X != defc.
:- extend(I), item(map(A, def)), childItem(I, map(A, X)), current(A), X != defc, X != def.
%% Discard sets if joined rows have different OUT-/OUTC-mapping.
:- extend(I), out(A), childItem(I, map(A, X)), X != outc, X != out.

%% Discard sets if OUT/OUTC appears to be DEF.
:- item(map(A1, in)), out(A2), att(A1, A2).
:- item(map(A1, in)), out(A2), att(A2, A1).

#hide.
#show item/1.
#show extend/1.



