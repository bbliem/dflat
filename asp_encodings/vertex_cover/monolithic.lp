{ in(X) : vertex(X) }.
:- edge(X,Y), not in(X;Y).
#minimize[ in(_) ].

#hide.
#show in/1.
