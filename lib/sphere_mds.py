import random
import math
import numpy as np

def dot_to_adjacency_matrix(dotGraph):
    G = nx_agraph.from_agraph(dotGraph)
    return nx.to_numpy_matrix(G, nodelist=G.nodes)

def testMDS():
    rows = 4
    cols = 2
    data = [0] * rows
    randos = [0] * rows
    for i in range(rows):
        data[i] = [0] * cols
        randos[i] = [0] * cols

    for i in range(4):
        data[i][0] = random.randint(0, 180)
        data[i][1] = random.randint(0, 360)
        randos[i][0] = data[i][0] - 90
        randos[i][1] = data[i][1] - 180

    adjMatrix = [0] * rows
    for i in range(rows):
        adjMatrix[i] = [0] * rows

    adjMatrix[0][0] = 0
    adjMatrix[0][1] = 1
    adjMatrix[0][2] = 2
    adjMatrix[0][3] = 3
    adjMatrix[1][0] = 1
    adjMatrix[1][1] = 0
    adjMatrix[1][2] = 1
    adjMatrix[1][3] = 2
    adjMatrix[2][0] = 2
    adjMatrix[2][1] = 1
    adjMatrix[2][2] = 0
    adjMatrix[2][3] = 1
    adjMatrix[3][0] = 3
    adjMatrix[3][1] = 2
    adjMatrix[3][2] = 1
    adjMatrix[3][3] = 0

    return gradientDescent(adjMatrix, data, 100)

def gradientDescent(adjMatrix, data, iterations, learning_rate = 0.1):
    for i in range(0, iterations):
        difference = derivativeVector(adjMatrix, data)
        for j in range(len(data)):
            for k in range(len(data[j])):
                data[j][k] -= difference[j][k] * learning_rate
    
    for i in range(len(data)):
        data[i][0] -= 90
        data[i][1] -= 180

    return data

# Following implements derivatives from Cox, Cox (1991) paper
def derivativeVector(adjMatrix, data):
    rows = len(data)
    cols = len(data[0])
    result = [0] * rows
    for i in range(rows):
        result[i] = [0] * cols

    i = 0
    for point in data:
        j = 0
        for theta in point: 
            if j == 0:
                result[i][j] = derivative = 1 / 2 * math.sqrt(T(data)/S(data, adjMatrix)) * (T(data) ** (-1) * derivS1(data, adjMatrix, point, i) - S(data, adjMatrix) * T(data) ** (-2) * derivT1(data, point))
            else:
                result[i][j] = derivative = 1 / 2 * math.sqrt(T(data)/S(data, adjMatrix)) * (T(data) ** (-1) * derivS2(data, adjMatrix, point, i) - S(data, adjMatrix) * T(data) ** (-2) * derivT2(data, point))
            j = j + 1
        i = i + 1

    return result

def derivS2(data, adjMatrix, point, pointIdx):
    sum = 0
    for i in range(len(data)):
        if i != pointIdx:
            sum += (S(data, adjMatrix) ** (-1) * (d(data[i], point) - adjMatrix[i][pointIdx]) / d(data[i], point) - T(data) ** (-1)) * math.sin(math.radians(point[1])) * math.cos(math.radians(data[i][1])) - math.cos(math.radians(point[1])) * math.sin(math.radians(data[i][1])) * math.cos(math.radians(point[0]) - math.radians(data[i][0]))

    return math.sqrt(S(data, adjMatrix)/T(data)) * sum

def derivS1(data, adjMatrix, point, pointIdx):
    sum = 0
    for i in range(len(data)):
        if i != pointIdx:
            sum += (S(data, adjMatrix) ** (-1) * (d(data[i], point) - adjMatrix[i][pointIdx]) / d(data[i], point) - T(data) ** (-1)) * math.sin(math.radians(point[1])) * math.sin(math.radians(data[i][1])) * math.sin(math.radians(point[0]) - math.radians(data[i][0]))

    return math.sqrt(S(data, adjMatrix)/T(data)) * sum

# data[i] = theta_i
# point = theta_l
def derivT2(data, point):
    sum = 0
    for i in range(len(data)):
       sum += math.sin(math.radians(point[1]) * math.cos(math.radians(data[i][1])) - math.cos(math.radians(point[1])) * math.cos(math.radians(point[0]) - math.radians(data[i][0])))
    return 2 * sum

def derivT1(data, point):
    sum = 0
    for i in range(len(data)):
       sum += math.sin(math.radians(point[1]) * math.sin(math.radians(data[i][1])) * math.sin(math.radians(point[0]) - math.radians(data[i][0])))
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
    for i in range(len(data)):
        for j in range(len(data)):
            if (i < j):
                sum += (d(data[i], data[j]) - adjMatrix[i][j]) ** 2

    return sum

# compares distance between points i and j
# i[0] = theta_i1, i[1] = theta_i2
def d(i, j):
    return math.sqrt(2 - 2 * math.sin(math.radians(i[1])) * math.sin(math.radians(j[1])) * math.cos(math.radians(i[0]) - math.radians(j[0])) - 2 * math.cos(math.radians(i[1])) * math.cos(math.radians(j[1])))

if __name__ == '__main__':
    rows = 4
    cols = 2
    data = [0] * rows

    for i in range(rows):
        data[i] = [0] * cols

    for i in range(4):
        data[i][0] = random.randint(0, 180)
        data[i][1] = random.randint(0, 360)

    adjMatrix = [0] * rows
    for i in range(rows):
        adjMatrix[i] = [0] * rows

    adjMatrix[0][0] = 0
    adjMatrix[0][1] = 1
    adjMatrix[0][2] = 2
    adjMatrix[0][3] = 3
    adjMatrix[1][0] = 1
    adjMatrix[1][1] = 0
    adjMatrix[1][2] = 1
    adjMatrix[1][3] = 2
    adjMatrix[2][0] = 2
    adjMatrix[2][1] = 1
    adjMatrix[2][2] = 0
    adjMatrix[2][3] = 1
    adjMatrix[3][0] = 3
    adjMatrix[3][1] = 2
    adjMatrix[3][2] = 1
    adjMatrix[3][3] = 0

    print(gradientDescent(adjMatrix, data, 100))