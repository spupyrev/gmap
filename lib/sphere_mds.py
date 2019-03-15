import random
import math
import numpy as np
import copy

def dot_to_adjacency_matrix(dotGraph):
    G = nx_agraph.from_agraph(dotGraph)
    return nx.to_numpy_matrix(G, nodelist=G.nodes)

def testMDS():
    rows = 4
    cols = 2
    data = [0] * rows
    for i in range(rows):
        data[i] = [0] * cols

    data[0][0] = 90.0
    data[0][1] = 90.0
    data[1][0] = 110.0
    data[1][1] = 150.0
    data[2][0] = 90.0
    data[2][1] = 210.0
    data[3][0] = 90.0
    data[3][1] = 270.0

    adjMatrix = [0] * rows
    for i in range(rows):
        adjMatrix[i] = [0] * rows

    adjMatrix[0][0] = 0.0
    adjMatrix[0][1] = 1.0
    adjMatrix[0][2] = math.sqrt(3)
    adjMatrix[0][3] = 2.0
    adjMatrix[1][0] = 1.0
    adjMatrix[1][1] = 0.0
    adjMatrix[1][2] = 1.0
    adjMatrix[1][3] = math.sqrt(3)
    adjMatrix[2][0] = math.sqrt(3)
    adjMatrix[2][1] = 1.0
    adjMatrix[2][2] = 0.0
    adjMatrix[2][3] = 1.0
    adjMatrix[3][0] = 2.0
    adjMatrix[3][1] = math.sqrt(3)
    adjMatrix[3][2] = 1.0
    adjMatrix[3][3] = 0.0

    result = gradientDescent(adjMatrix, data)
    return result
    
# create sorted list of dissimilarities between node pairs and their indices
def adjMatrixToSortedTuple(adjMatrix):
    tuples = []
    distances = []

    # only iterate through pairs of points once to prevent duplicates
    for i in range(len(adjMatrix)):
        j = 0
        lati = random.random() * 180.0
        loni = random.random() * 360.0
        pi = (lati, loni)
        while j < i:
            tuples.append((adjMatrix[i][j], i, j))
            latj = random.random() * 180.0
            lonj = random.random() * 360.0
            pj = (latj, lonj)
            distances.append(d(pi, pj))
            j += 1

    temp = zip(tuples, distances)
    temp.sort(key=lambda tup: tup[0][0])

    # sort by dissimilarities between pairs of nodes
    return temp

def gradientDescent(adjMatrix, data, learning_rate = 0.02):
    mag = 1.0
    iteration = 1.0
    result = []
    prevStress = stress(data,adjMatrix)
    curStress = stress(data,adjMatrix)
    lastGradient = []

    # iterate until threshold reached
    while curStress > 0 and iteration < 200.0:
        gradient = derivativeVector(adjMatrix, data)
        # gradient = derivativeVector2(adjMatrix, data)
        
        if (iteration == 1):
            lastGradient = gradient

        mag = gradMag(gradient, data)
        print("Iternation: " + str(iteration))
        print("Gradient Magnitude: " + str(mag))
        print("Gradient: " + str(gradient))
        iteration += 1
        fiveStressAgo = 1.0
        
        if (iteration % 5 == 0):
            fiveStressAgo = stress(data, adjMatrix)

        # create list of configurations in lon, lat form to display process
        copied = copy.deepcopy(data)
        for i in range(len(data)):
            lat = copied[i][0]
            copied[i][0] = copied[i][1] - 180
            copied[i][1] = lat - 90
        result.append(copied)

        # each point
        for j in range(len(data)):
            # each dimension
            for k in range(len(data[j])):
                # update data with gradient elementwise
                data[j][k] -= gradient[j][k] * learning_rate / mag
        
        # decrease learning rate
        curStress = stress(data, adjMatrix)
        #learning_rate = learning_rate * (4.0 ** (cosineSimilarity(lastGradient, gradient) ** 3.0)) * min(1, curStress/prevStress) * 1.3 / (1 + min(1, curStress/fiveStressAgo) ** 5.0)
        learning_rate = learning_rate * min(1, curStress/prevStress)
        prevStress = curStress

        #print("Cosine: " + str(4.0 ** (cosineSimilarity(lastGradient, gradient) ** 3.0)))
        #print("Cur/Prev: " + str(min(1, curStress/prevStress)))
        #print("Cur/5Ago: " + str(1.3 / (1 + min(1, curStress/fiveStressAgo) ** 5.0)))
        print("Learning Rate: " + str(learning_rate))
        print("Stress: " + str(curStress))
        print("data: " + str(data))
        print("D01: " + str(d(data[0], data[1])))
        print("D02: " + str(d(data[0], data[2])))
        print("D03: " + str(d(data[0], data[3])))
        print("D12: " + str(d(data[1], data[2])))
        print("D13: " + str(d(data[1], data[3])))
        print("D23: " + str(d(data[2], data[3])))


    
    # convert into lat lon with domains [-90, 90], [-180, 180]
    for i in range(len(data)):
        lat = data[i][0]
        data[i][0] = data[i][1] - 180
        data[i][1] = lat - 90

    return result

def cosineSimilarity(gradient1, gradient2):
    g1g2sum = 0.0
    g1sum = 0.0
    g2sum = 0.0

    for i in range(len(gradient1)):
        for j in range(len(gradient1[i])):
            g1g2sum += gradient1[i][j] * gradient2[i][j]
            g1sum += gradient1[i][j] ** 2
            g2sum += gradient2[i][j] ** 2
    
    return g1g2sum / math.sqrt(g1sum) * math.sqrt(g2sum)


# returns the magnitude of the gradient to know when gradient descent has found sufficient minimum
def gradMag(gradient, data):
    gradSum = 0.0
    dataSum = 0.0
    for i in range(len(gradient)):
        for j in range(len(gradient[i])):
            gradSum += gradient[i][j] ** 2.0
            dataSum += data[i][j] ** 2.0
    
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
                result[i][j] = 1.0 / 2.0 * math.sqrt(T(data)/S(data, adjMatrix)) * (T(data) ** (-1.0) * derivS1(data, adjMatrix, point, i) - S(data, adjMatrix) * T(data) ** (-2.0) * derivT1(data, point))
            else:
                result[i][j] = 1.0 / 2.0 * math.sqrt(T(data)/S(data, adjMatrix)) * (T(data) ** (-1.0) * derivS2(data, adjMatrix, point, i) - S(data, adjMatrix) * T(data) ** (-2.0) * derivT2(data, point))
            j = j + 1
        i = i + 1

    return result

def derivS2(data, adjMatrix, point, pointIdx):
    sum = 0.0
    for i in range(len(data)):
        if i != pointIdx:
            sum += (S(data, adjMatrix) ** (-1.0) * (d(data[i], point) - adjMatrix[i][pointIdx]) / d(data[i], point) - T(data) ** (-1.0)) * math.sin(math.radians(point[1])) * math.cos(math.radians(data[i][1])) - math.cos(math.radians(point[1])) * math.sin(math.radians(data[i][1])) * math.cos(math.radians(point[0]) - math.radians(data[i][0]))

    return math.sqrt(S(data, adjMatrix)/T(data)) * sum

def derivS1(data, adjMatrix, point, pointIdx):
    sum = 0.0
    for i in range(len(data)):
        if i != pointIdx:
            sum += (S(data, adjMatrix) ** (-1.0) * (d(data[i], point) - adjMatrix[i][pointIdx]) / d(data[i], point) - T(data) ** (-1.0)) * math.sin(math.radians(point[1])) * math.sin(math.radians(data[i][1])) * math.sin(math.radians(point[0]) - math.radians(data[i][0]))

    return math.sqrt(S(data, adjMatrix)/T(data)) * sum

# data[i] = theta_i
# point = theta_l
def derivT2(data, point):
    sum = 0.0
    for i in range(len(data)):
       sum += math.sin(math.radians(point[1]) * math.cos(math.radians(data[i][1])) - math.cos(math.radians(point[1])) * math.cos(math.radians(point[0]) - math.radians(data[i][0])))
    return 2.0 * sum

def derivT1(data, point):
    sum = 0.0
    for i in range(len(data)):
       sum += math.sin(math.radians(point[1]) * math.sin(math.radians(data[i][1])) * math.sin(math.radians(point[0]) - math.radians(data[i][0])))
    return 2.0 * sum

def T(data):
    sum = 0.0
    for i in range(len(data)):
        for j in range(len(data)):
            if (i < j):
                sum += d(data[i], data[j]) ** 2.0

    return sum

def S(data, adjMatrix):
    sum = 0.0
    for i in range(len(data)):
        for j in range(len(data)):
            if (i < j):
                sum += (d(data[i], data[j]) - adjMatrix[i][j]) ** 2.0

    return sum

# compares distance between points i and j
# i[0] = theta_i1, i[1] = theta_i2
def d(i, j):
    return math.sqrt(2.0 - 2.0 * math.sin(math.radians(i[1])) * math.sin(math.radians(j[1])) * math.cos(math.radians(i[0]) - math.radians(j[0])) - 2.0 * math.cos(math.radians(i[1])) * math.cos(math.radians(j[1])))

def stress(data, adjMatrix):
    sum = 0.0
    for i in range(len(data)):
        for j in range(len(data)):
            if (i < j):
                sum += (d(data[i], data[j]) - adjMatrix[i][j]) ** 2

    return sum

if __name__ == '__main__':
    rows = 4
    cols = 2
    data = [0] * rows
    for i in range(rows):
        data[i] = [0] * cols

    #for i in range(4):
    #    data[i][0] = random.random() * 180
    #    data[i][1] = random.random() * 360

    data[0][0] = 90.0
    data[0][1] = 40.0
    data[1][0] = 110.0
    data[1][1] = 80.0
    data[2][0] = 90.0
    data[2][1] = 120.0
    data[3][0] = 90.0
    data[3][1] = 160.0


    adjMatrix = [0] * rows
    for i in range(rows):
        adjMatrix[i] = [0] * rows

    adjMatrix[0][0] = 0.0
    adjMatrix[0][1] = 1.0
    adjMatrix[0][2] = 2.0
    adjMatrix[0][3] = 3.0
    adjMatrix[1][0] = 1.0
    adjMatrix[1][1] = 0.0
    adjMatrix[1][2] = 1.0
    adjMatrix[1][3] = 2.0
    adjMatrix[2][0] = 2.0
    adjMatrix[2][1] = 1.0
    adjMatrix[2][2] = 0.0
    adjMatrix[2][3] = 1.0
    adjMatrix[3][0] = 3.0
    adjMatrix[3][1] = 2.0
    adjMatrix[3][2] = 1.0
    adjMatrix[3][3] = 0.0

    print(gradientDescent(adjMatrix, data))