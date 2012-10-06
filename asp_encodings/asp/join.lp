% This algorithm follows the ideas in Jakl et al: Answer-Set Programming with Bounded Treewidth.
map(0,X,true) :- m(X).
map(1,X,true) :- c(X).
% Assign false to anything not true
map(0,X,false) :- current(X), not map(0,X,true).
map(1,X,false) :- cExists, current(X), not map(1,X,true).

1 { chosenChildTupleL(I) : childTupleL(I) } 1.
1 { chosenChildTupleR(I) : childTupleR(I) } 1.

mL(X) :- chosenChildTupleL(I), mapped(I,X,true).
mR(X) :- chosenChildTupleR(I), mapped(I,X,true).

% If atoms coincide, unify the sets mL and mR.
:- atom(A), mL(A), not mR(A).
:- atom(A), mR(A), not mL(A).
m(X) :- mL(X).
m(X) :- mR(X).

cLI(I) :- chosenChildTupleL(L), sub(L,I).
cRI(I) :- chosenChildTupleR(R), sub(R,I).

cL(I,X) :- cLI(I), mapped(I,X,true).
cR(I,X) :- cRI(I), mapped(I,X,true).

% If there is no C we must take care not to kill M with constraints later.
% Which C's are not joinable?
not_cc(L,R) :- atom(A), cLI(L), cRI(R), mapped(L,A,V), not mapped(R,A,V).
not_mc(I) :- atom(A), mL(A), cRI(I), not cR(I,A).
not_mc(I) :- atom(A), cR(I,A), not mL(A).
not_cm(I) :- atom(A), mR(A), cLI(I), not cL(I,A).
not_cm(I) :- atom(A), cL(I,A), not mR(A).

% A certificate exists if there are two joinable child C's or if a child C joins with M
cExists :- cLI(L), cRI(R), not not_cc(L,R).
cExists :- cRI(I), not not_mc(I).
cExists :- cLI(I), not not_cm(I).

%% Guess which of the certificates to join
0 { chosenCL(I) : sub(L,I) } 1 :- chosenChildTupleL(L).
choseCL :- chosenCL(_).
cCandidateL(I) :- chosenCL(I).
cCandidateL(I) :- not choseCL, chosenChildTupleL(I).

0 { chosenCR(I) : sub(R,I) } 1 :- chosenChildTupleR(R).
choseCR :- chosenCR(_).
cCandidateR(I) :- chosenCR(I).
cCandidateR(I) :- not choseCR, chosenChildTupleR(I).

:- cExists, not choseCL, not choseCR.

% Join candidates must coincide on atoms
:- cCandidateL(L), cCandidateR(R), atom(A), mapped(L,A,V), not mapped(R,A,V).

c(X) :- cExists, cCandidateL(I), mapped(I,X,true).
c(X) :- cExists, cCandidateR(I), mapped(I,X,true).



#hide.
#show map/3.
#show chosenChildTupleL/1.
#show chosenChildTupleR/1.
