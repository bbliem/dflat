% Condition (2)
:- atom(A), m1(A), not m2(A).
:- atom(A), m2(A), not m1(A).
m(X) :- m1(X).
m(X) :- m2(X).

% If there is no C we must take care not to kill M with constraints later.
% Which C's are not joinable?
notc1c2(I,J) :- atom(A), c1i(I), c2i(J), c1(I,A), not c2(J,A).
notc1c2(I,J) :- atom(A), c1i(I), c2i(J), c2(J,A), not c1(I,A).
notm1c2(J) :- atom(A), c2i(J), m1(A), not c2(J,A).
notm1c2(J) :- atom(A), c2i(J), c2(J,A), not m1(A).
notc1m2(I) :- atom(A), c1i(I), c1(I,A), not m2(A).
notc1m2(I) :- atom(A), c1i(I), m2(A), not c1(I,A).

cExists :- c1i(I), c2i(J), not notc1c2(I,J).
cExists :- c2i(J), not notm1c2(J).
cExists :- c1i(I), not notc1m2(I).

% Guess which of the certificates to join
1 { m1c2(J) : c2i(J), c1m2(I) : c1i(I), c1c2(I,J) : c1i(I) : c2i(J) } 1 :- cExists.
:- c1c2(I,J), notc1c2(I,J).
:- m1c2(J), notm1c2(J).
:- c1m2(I), notc1m2(I).

c(X) :- c1c2(I,J), c1(I,X).
c(X) :- c1c2(I,J), c2(J,X).
c(X) :- m1c2(J), m1(X).
c(X) :- m1c2(J), c2(J,X).
c(X) :- c1m2(I), c1(I,X).
c(X) :- c1m2(I), m2(X).


#hide.
#show m/1.
#show c/1.
#show m1c2/1.
#show c1m2/1.
#show c1c2/2.
#show cExists/0.
