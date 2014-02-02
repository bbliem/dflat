% \exists S
% \forall x, y:
% \exists z:
% (-S(x) | -S(y) | -edge(x,y)) &
% (S(x) | S(z)) &
% (S(x) | edge(z,x)) &

% XXX In the paper we required a more restrictive normal form
length(3).
or(0) :- not incomplete(0).
sVar(1,s).
and(1) :- not incomplete(1).
iVar(2,x).
iVar(2,y).
or(2) :- not incomplete(2).
iVar(3,z).

neg(c1,in(x,s)).
neg(c1,in(y,s)).
neg(c1,edge(x,y)).
pos(c2,in(x,s)).
pos(c2,in(z,s)).
pos(c3,in(x,s)).
pos(c3,edge(z,x)).
