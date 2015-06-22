1 { p(X,Y) : edge(X,Y) } 1 :- vertex(X).
1 { p(X,Y) : edge(X,Y) } 1 :- vertex(Y).

% For all partitions into two sets there must be a crossing edge.
s1(X) | s2(X) :- vertex(X).
s1(X) :- saturate, vertex(X).
s2(X) :- saturate, vertex(X).
:- not saturate.

% Found crossing edge?
saturate :- p(X,Y), s1(X), s2(Y).

% Not a partition due to a set being empty?
% This is the case if the other set contains all elements.
all1 :- all1_upto(X), sup(X).
all1_upto(X) :- inf(X), s1(X).
all1_upto(Y) :- all1_upto(X), succ(X,Y), s1(Y).

all2 :- all2_upto(X), sup(X).
all2_upto(X) :- inf(X), s2(X).
all2_upto(Y) :- all2_upto(X), succ(X,Y), s2(Y).

saturate :- all1.
saturate :- all2.

notSucc(X,Z) :- vertex(X), vertex(Y), vertex(Z), X < Y, Y < Z.
succ(X,Y) :- vertex(X), vertex(Y), X < Y, not notSucc(X,Y).
notInf(X) :- succ(_,X).
inf(X) :- vertex(X), not notInf(X).
notSup(X) :- succ(X,_).
sup(X) :- vertex(X), not notSup(X).

#show p/2.
