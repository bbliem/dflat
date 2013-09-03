%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Encoding for admissible extensions
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

%% The argument x is defeated by the set S
defeated(X) :- in(Y), defeat(Y,X).

%% The argument x is not defended by S
not_defended(X) :- defeat(Y,X), not defeated(Y).

%% All arguments x \in S need to be defended by S
:- in(X), not_defended(X).
