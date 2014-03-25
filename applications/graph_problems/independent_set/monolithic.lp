{ in(X) : vertex(X) }.
:- edge(X,Y), in(X;Y).
#maximize { 1,X : in(X) }.

#show in/1.
