from networkx.drawing import nx_agraph
import numpy as np
import networkx as nx

def dot_to_adjacency_matrix(dotGraph):
    G = nx_agraph.from_agraph(dotGraph)
    return nx.to_numpy_matrix(G, nodelist=G.nodes)

def gradientDescent(adjMatrix, data, iterations, learning_rate = 0.1):
    for i in range(0, iterations):
        difference = derivativeVector(adjMatrix, data)
        for j in range(len(data)):
            for k in range(len(data[j]):
                data[j][k] -= difference[j][k] * learning_rate
    return data

# Following implements derivatives from Cox, Cox (1991) paper
def derivativeVector(adjMatrix, data):
    result = [[]]
    i = 0
    for point in data:
        j = 0
        for theta in point: 
            if j = 0:
                result[i][j] = derivative = 1 / 2 * math.sqrt(T(data)/S(data, adjMatrix)) * (T(data) ** (-1) * derivS1() - S(data, adjMatrix) * T(data) ** (-2) ^ 2 * derivT1(data, point))
            else:
                result[i][j] = derivative = 1 / 2 * math.sqrt(T(data)/S(data, adjMatrix)) * (T(data) ** (-1) * derivS2() - S(data, adjMatrix) * T(data) ** (-2) ^ 2 * derivT2())
            j = j + 1
        i = i + 1

    return result

def derivS2(data, adjMatrix, point, pointIdx):
    sum = 0
    for i in range(len(data)):
        sum += (S(data) ** (-1) * (d(data[i], point) - adjMatrix[i][pointIdx]) / d(data[i], point) - T(data) ** (-1)) * math.sin(point[1]) * math.cos(data[i][1]) - math.cos(point[1]) * math.sin(data[i][1]) * math.cos(point[0] - data[i][0])

    return math.sqrt(S(data)/T(data)) * sum

def derivS1(data, adjMatrix, point, pointIdx):
    sum = 0
    for i in range(len(data)):
        sum += (S(data) ** (-1) * (d(data[i], point) - adjMatrix[i][pointIdx]) / d(data[i], point) - T(data) ** (-1)) * math.sin(point[1]) * math.sin(data[i][1]) * math.sin(point[0] - data[i][0])

    return math.sqrt(S(data)/T(data)) * sum

# data[i] = theta_i
# point = theta_l
def derivT2(data, point):
    sum = 0
    for i in range(len(data)):
       sum += math.sin(point[1] * math.cos(data[i][1]) - math.cos(point[1]) * math.cos(point[0] - data[i][0]))
    return 2 * sum

def derivT1(data, point):
    sum = 0
    for i in range(len(data)):
       sum += math.sin(point[1] * math.sin(data[i][1]) * math.sin(point[0] - data[i][0]))
    return 2 * sum

def T(data):
    sum = 0
    for i in range(len(data)):
        for j in range(len(data)):
            if (i < j):
                sum += d(data[i], data[j]) ** 2

    return sum

def S(data, adjMatrix):
    sum = 0
    for i in rang(len(data)):
        for j in range(len(data)):
            if (i < j):
                sum += d(data[i], data[j]) - adjMatrix[i][j]) ** 2

    return sum

# compares distance between points i and j
# i[0] = theta_i1, i[1] = theta_i2
def d(i, j):
    return math.sqrt(2 - 2 * math.sin(i[1]) * math.sin(j[1]) * math.cos(i[0] - j[0]) - 2 * math.cos(i[1]) * math.cos(j[1]))

    