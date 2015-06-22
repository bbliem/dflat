%dflat: --tables -e vertex -e edge -d td -n semi --no-empty-leaves --no-empty-root
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
:- extend(R), extend(S), childItem(R,in(X)), not childItem(S,in(X)).

item(in(X)) :- extend(R), childItem(R,in(X)), not removed(X).

{ item(in(X)) : introduced(X) }.
:- edge(X,Y), item(in(X)), item(in(Y)).

% We use negative costs since this is a maximization problem and D-FLAT currently only minimizes.
% Leaf costs
cost(-C) :- initial, C = #count{ X : item(in(X)) }.
% Exchange costs
cost(CC - IC) :- numChildNodes == 1, extend(R), childCost(R,CC), IC = #count{ X : item(in(X)), introduced(X) }.
% Join costs
cost(C1 + C2 - CC) :- numChildNodes == 2, extend(R1), extend(R2), childCost(R1,C1), childCost(R2,C2), commonCost(R1,R2,CC).
commonCost(R1,R2,-CC) :- childRow(R1,N1), childRow(R2,N2), N1 < N2, CC = #count { X : childItem(R1,in(X)), childItem(R2,in(X)) }.

#show extend/1.
#show item/1.
#show cost/1.
