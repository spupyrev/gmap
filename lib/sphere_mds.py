from networkx.drawing import nx_agraph
import numpy as np
import networkx as nx

def dot_to_adjacency_matrix(dotGraph):
    G = nx_agraph.from_agraph(dotGraph)
    return nx.to_numpy_matrix(G, nodelist=G.nodes)

def gradient_descent(dMatrix):
    ilat = Symbol('ilat')
    ilon = Symbol('ilon')
    jlat = Symbol('jlat')
    jlon = Symbol('jlon')
    i = Symbol('i')
    j = Symbol('j')

    deltaLat = radians(jlat-ilat)
    deltaLon = radians(jlon-ilon)

    # Haversine formula
    a = sin(deltaLat/2) * sin(deltaLat/2) + cos(ilat) * cos(jlat) * sin(deltaLon/2) * sin(deltaLon/2)
    geo_dist = 2 * atan2(sqrt(a), sqrt(1-a))

    # cost function
    cost = sympy.sqrt(Sum(lambda n: (geo_dist - dMatrix[0][i][j]) ** 2, (i, 1, j)).doit()  / Sum(lambda n: dMatrix[0][i][j] ** 2, (i, 1, j)).doit())
    
    # partial derivitives
    costpilat = cost.diff(ilat)
    costpilon = cost.diff(ilon)
    costpjlat = cost.diff(jlat)
    costpjlon = cost.diff(jlon)

    grad = [costpilat, costpilon, costpjlat, costpjlon]

    