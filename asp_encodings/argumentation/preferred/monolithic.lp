%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Encoding for preferred extensions
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% an argument x defeats an argument y if x attacks y
defeat(X,Y) :- att(X,Y),
	       not vaf.

%% Guess a set S \subseteq A
in(X) :- not out(X), arg(X).
out(X) :- not in(X), arg(X).

%% S has to be conflict-free
:- in(X), in(Y), defeat(X,Y).

%% All arguments x \in S need to be defended by S (admissibility)
defeated(X) :- in(Y), defeat(Y,X).
:- in(X), defeat(Y,X), not defeated(Y).

%% Minimize out to get the subset-maximal admissible sets (needs metasp)
#minimize [ out(X) ].