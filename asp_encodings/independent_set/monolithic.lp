{ in(X) : vertex(X) }.
:- edge(X,Y), in(X;Y).
#maximize[ in(_) ].

#hide.
#show in/1.
