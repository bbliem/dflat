% \exists S
% \forall x:
% \exists z:
% (S(x) | S(z)) &
% (S(x) | edge(z,x)) &

% XXX In the paper we required a more restrictive normal form
length(3).
or(0) :- not incomplete(0).
sVar(1,s).
and(1) :- not incomplete(1).
iVar(2,x).
or(2) :- not incomplete(2).
iVar(3,z).

pos(c1,in(x,s)).
pos(c1,in(z,s)).
pos(c2,in(x,s)).
pos(c2,edge(z,x)).
