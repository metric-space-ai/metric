#!/usr/bin/python3

import networkx as nx
import matplotlib
import matplotlib.pyplot as plt
import pylab as PL
import random, sys


def paley_graph(p):
	if not p%4 == 1:
		return None

	square_list = []
	for i in range(int((p-1)/2)):
		square_list.append((i**2)%p)

	g = nx.Graph()

	for i in range(p):
		g.add_node(i)
	for i in range(p):
		for j in square_list:
			g.add_edge(i, (i+j)%p)
			print(i, (i+j)%p)
	g=nx.Graph(g)  # remove parallel edges
	g.remove_edges_from(g.selfloop_edges())  # remove self loops
	return g


g = paley_graph(13)
print(nx.diameter(g))
PL.figure()
nx.draw_circular(g, with_labels=True)
plt.show()
