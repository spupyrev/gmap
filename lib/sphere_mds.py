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

    return gradientDescent(adjMatrix, data)

def gradientDescent(adjMatrix, data, learning_rate = 0.2):
    mag = 1
    iteration = 1

    # iterate until threshold reached
    while mag > 0.00005:
        gradient = derivativeVector(adjMatrix, data)
        mag = gradMag(gradient, data)
        print("Iternation: " + str(iteration))
        print("Gradient Magnitude: " + str(mag))
        iteration += 1
        for j in range(len(data)):
            for k in range(len(data[j])):
                data[j][k] -= gradient[j][k] * learning_rate
    
    # convert into lat lon with domains [-90, 90], [-180, 180]
    for i in range(len(data)):
        lat = data[i][0]
        data[i][0] = data[i][1] - 180
        data[i][1] = lat - 90

    return data

# returns the magnitude of the gradient to know when gradient descent has found sufficient minimum
def gradMag(gradient, data):
    gradSum = 0
    dataSum = 0
    for i in range(len(gradient)):
        for j in range(len(gradient[i])):
            gradSum += gradient[i][j] ** 2
            dataSum += data[i][j] ** 2
    
    return math.sqrt(gradSum) / math.sqrt(dataSum)
        

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

    print(gradientDescent(adjMatrix, data))