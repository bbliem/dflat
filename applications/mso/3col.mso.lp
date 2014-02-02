% \exists R, G, B:
% \forall x, y:
% (R(x) | G(x) | B(x)) &
% (-R(x) | -G(x)) &
% (-R(x) | -B(x)) &
% (-G(x) | -B(x)) &
% (-edge(x,y) | -R(x) | -R(y)) &
% (-edge(x,y) | -R(x) | -G(y)) &
% (-edge(x,y) | -G(x) | -B(y))

% XXX In the paper we required a more restrictive normal form
length(2).
or(0) :- not incomplete(0).
sVar(1,r).
sVar(1,g).
sVar(1,b).
and(1) :- not incomplete(1).
iVar(2,x).
iVar(2,y).

pos(c1,in(x,r)).
pos(c1,in(x,g)).
pos(c1,in(x,b)).
neg(c2,in(x,r)).
neg(c2,in(x,g)).
neg(c3,in(x,r)).
neg(c3,in(x,b)).
neg(c4,in(x,g)).
neg(c4,in(x,b)).
neg(c5,edge(x,y)).
neg(c5,in(x,r)).
neg(c5,in(y,r)).
neg(c6,edge(x,y)).
neg(c6,in(x,g)).
neg(c6,in(y,g)).
neg(c7,edge(x,y)).
neg(c7,in(x,b)).
neg(c7,in(y,b)).
