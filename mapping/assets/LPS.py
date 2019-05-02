#!/usr/bin/python

from sys import argv
import networkx as nx
import matplotlib
import matplotlib.pyplot as plt
import pylab as PL
import random, sys

def miller_rabin_pass(a, s, d, n):
	a_to_power = pow(a, d, n)
	if a_to_power == 1:
		return True
	for i in range(s-1):
		if a_to_power == n - 1:
			return True
		a_to_power = (a_to_power * a_to_power) % n
	return a_to_power == n - 1


def miller_rabin(n):
	d = n - 1
	s = 0
	while d % 2 == 0:
		d >>= 1
		s += 1
	for repeat in xrange(20):
		a = 0
		while a == 0:
			a = random.randrange(n)
		if not miller_rabin_pass(a, s, d, n):
			return False
	return True

def ramanujan_graph(p):
	if not miller_rabin(p):
		print "p is not prime"
		return None
	g = nx.Graph()

	for i in range(p):
		g.add_node(i)
	for i in range(p):
		if i == 0:
			g.add_edge(0,p-1)
			g.add_edge(0,1)
		else:
			g.add_edge(i,i-1)
			g.add_edge(i,(i+1)%p)
			g.add_edge(i,(i**(p-2))%p)

	g=nx.Graph(g) #remove parallel edges
	g.remove_edges_from(g.selfloop_edges()) #remove self loops
	return g

g = ramanujan_graph(int(argv[1]))
if g is None:
	exit()
PL.figure()
nx.draw_circular(g, with_labels=True)
plt.show()