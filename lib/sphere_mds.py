from networkx.drawing import nx_agraph
import numpy as np
import networkx as nx

def dot_to_adjacency_matrix(dotGraph):
    G = nx_agraph.from_agraph(dotGraph)
    return nx.adjacency_matrix(G)
