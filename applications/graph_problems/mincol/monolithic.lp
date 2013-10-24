color(red;green;blue).
criticalColor(red).

1 { map(X,C) : color(C) } 1 :- vertex(X).
:- edge(X,Y), map(X,C), map(Y,C).

% Use metasp for subset-minimization
#minimize [ map(V,C) : criticalVertex(V) : criticalColor(C) ].

#hide.
#show map/2.
