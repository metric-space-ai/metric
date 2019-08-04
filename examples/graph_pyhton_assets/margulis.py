#!/usr/bin/python
import networkx as nx
import matplotlib
import matplotlib.pyplot as plt
import pylab as PL
import random, sys

def margulis_graph(n):
	#Graph has n^2 vertices
	g = nx.Graph()

	for i in range(n):
		for j in range(n):
			g.add_node((i,j))

	for (i,j) in g.nodes():
		g.add_edge((i,j),((i+2*j)%n,j))
		g.add_edge((i,j),(i,(2*i+j)%n))
		g.add_edge((i,j),(i,(2*i+j+1)%n))
		g.add_edge((i,j),((i+2*j+1)%n,j))
	g=nx.Graph(g) #remove parallel edges
	g.remove_edges_from(g.selfloop_edges()) #remove self loops
	return g

g = margulis_graph(5)
print nx.diameter(g)
PL.figure()
nx.draw_circular(g, with_labels=True)
plt.show()