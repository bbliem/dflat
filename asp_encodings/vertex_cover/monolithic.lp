1 { map(X,in), map(X,out) } 1 :- vertex(X).
:- edge(X,Y), map(X,out), map(Y,out).

cost(C) :- C = #count{ map(X,in) }.

#minimize[ cost(C) = C ]. % This works here, but we must not use minimization in exchange.lp because the global optimum might stem from a locally non-optimal tuple



#hide.
#show cost/1.
#show map/2.
