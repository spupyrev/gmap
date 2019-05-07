import numpy as np
from scipy.spatial.distance import pdist, squareform

def smacofSphere(delta, ndim = 3, mds = "ratio", alg = "dual", 
    weightmat = None, init = "torgerson", ties = "primary", 
    verbose = False, penalty = 100, relax = False, modulus = 1, 
    itmax = 1000, eps = 1e-6, spline_degree = 2, spline_intKnots = 2):

    TYPES = {"ratio", "interval", "ordinal", "mspline"}
    ALGS = {"dual", "primal"}

    if mds not in TYPES:
        raise ValueError("smacofSphere: mds must be one of %r." % TYPES)
    
    if alg not in ALGS:
        raise ValueError("smacofSphere: alg must be one of %r." % ALGS)

    if alg == "dual":
        num_samples = delta.shape[0]
        diss = squareform(delta, 'tovector')
        diss = diss[diss != 0]
    p = ndim
    n = num_samples

    nn = n * (n-1)/2
    m = len(diss)

    wghts = np.ones((num_samples, num_samples))
    np.fill_diagonal(wghts, 0)
    wghts = squareform(wghts, 'tovector')
    dhat = np.multiply(np.divide(diss, np.sqrt(np.sum(np.multiply(wghts, np.square(diss))))), np.sqrt(m))

    # initialization
    if (init == "torgerson"):
        values, vectors = np.linalg.eig(-np.divide(doubleCenter(np.square(squareform(diss, 'tomatrix'))), 2))
        values[values<0] = 0
        vectors = vectors[:, values.argsort()[::-1]]
        values = np.sort(values)[::-1]
        normdiag = np.diag(np.sqrt(values[0:num_samples-1]))
        x = np.matmul(vectors[:, 0:p], normdiag)

    xstart = x
    if relax:
        relax = 2
    else:
        relax = 1

    mn = np.insert(np.repeat(0, n), 0, 1)
    diss = squareform(np.insert(np.insert(squareform(diss, 'tomatrix'), 0, 0, axis=1), 0, 0, axis=0), 'tovector')
    
    wghts1 = squareform(np.insert(np.insert(squareform(wghts, 'tomatrix'), 0, 0, axis=1), 0, 0, axis=0), 'tovector')
    wghts2 = squareform(absDif(mn, mn), 'tovector')
    dhat1 = squareform(np.insert(np.insert(squareform(dhat, 'tomatrix'), 0, 0, axis=1), 0, 0, axis=0), 'tovector')  
    dhat2 = np.multiply(np.mean(np.sqrt(np.sum(np.square(x), axis=1))), wghts2)

    n1 = len(squareform(dhat1, 'tomatrix'))
    nn1 = n1 * (n1 - 1)/2
    x = np.insert(x, 0, 0, axis=0)
    w = vmat(wghts1 + np.multiply(penalty, wghts2))
    v = myGenInv(w)
    
    itel = 1
    d = pdist(x, 'euclidean')
    lb = np.divide(np.sum(wghts1 * d * dhat1), np.sum(wghts1 * np.square(d)))

    x = np.multiply(lb, x)
    d = np.multiply(lb, d)
    
    sold1 = np.sum(np.multiply(wghts1, np.square(np.subtract(dhat1, d))))
    sold2 = np.sum(np.multiply(wghts2, np.square(np.subtract(dhat2, d))))
    sold = sold1 + np.multiply(penalty, sold2)
    disobj = transPrep(diss)

    while (True) :
        b = bmat(dhat1, wghts1, d) + np.multiply(penalty, bmat(dhat2, wghts2, d))
        y = np.matmul(v, (np.matmul(b, x)))
        y = x + np.multiply(relax, y - x)
        e = pdist(y, 'euclidean')
        ssma1 = np.sum(np.multiply(wghts1, np.square(dhat1 - e)))
        ssma2 = np.sum(np.multiply(wghts2, np.square(dhat2 - e)))
        ssma = ssma1 + np.multiply(penalty, ssma2)
        
        dhat3 = transform(e, disobj, wghts1, nn1)
        dhat1 = dhat3[0]
        dhat2 = np.multiply(np.mean(e[0:n-1]), wghts2)
        snon1 = np.sum(np.multiply(wghts1, np.square(dhat1 - e)))
        snon2 = np.sum(np.multiply(wghts2, np.square(dhat2 - e)))
        snon = snon1 + np.multiply(penalty, snon2)
        if (((sold - snon) < eps) or (itel == itmax)):
            break
        x = y
        d = e
        sold = snon
        itel = itel + 1
        if (itel == itmax):
            break

    ss = y[0]
    y = y - ss
    y = y[1:delta.shape[0] + 1]
    stress = np.sqrt(snon/nn)
    return (y, stress)
    

def myGenInv(x):
    n = len(x[0])
    nn = 1/n
    return np.subtract(np.linalg.inv(np.add(x, nn)), nn)

def vmat(wghts):
    v = squareform(wghts, 'tomatrix')
    r = np.sum(v, axis = 1)
    return np.subtract(np.diag(r), v)

def bmat(diss, wghts, d, eps = 1e-12):
    z = np.zeros(len(d))
    for i in range(len(d)):
        if d[i] < eps:
            z[i] = 1
        else:
            z[i] = 0
    b = np.divide(np.multiply(np.multiply(wghts, diss), (1-z)), d + z)
    
    b = squareform(b, 'tomatrix')
    r = np.sum(b, axis=1)
    return(np.subtract(np.diag(r), b))

def absDif(x, y):
    x, y = np.asarray(x)[:, None], np.asarray(y)
    return np.abs(np.subtract(x, y))

def doubleCenter(x):
    n = len(x)
    m = len(x[0])
    s = np.divide(np.sum(x), n*m)
    xr = np.divide(np.sum(x, axis=0), m)
    xc = np.divide(np.sum(x, axis=1), n)
    return np.add((np.subtract(x, np.add.outer(xr, xc))), s)

def transPrep(x, trans="none", spline_intKnots = 4, spline_degree = 2, missing="none"):
    knotSeq = None
    base = None
    n = len(x)
    iord = np.argsort(x)
    y = np.sort(x)
    indTieBlock = np.r_[1, (np.r_[2:n+1])[~(y[:n-1] == y[1:])]]
    ties = np.r_[indTieBlock[1:], n+1] - indTieBlock
    nties = len(ties)
    iord_mis = None
    x_unique = x[iord[np.cumsum(ties[:nties]) - 1]]
    base = np.vstack((np.repeat(1, nties), x_unique - x_unique[0])).T
    xInit = np.repeat(0, n)
    xInit[iord] = np.repeat(x_unique, ties)
    return {"x" : x, "x_unique" : x_unique, "n" : n, "trans" : trans, \
        "spline_allKnots" : spline_intKnots, "spline_knotSeq" : knotSeq, \
        "xInit" : xInit, "iord" : iord, "ties" : ties, "nties" : nties, \
        "base" : base, "iord_mis" : iord_mis, "class" : "optScal"}

def transform(Target, x, w, normq):
    n = len(x["x"])
    b = None
    iord3 = x["iord"]
    Result = np.repeat(0.0, n)
    ind_act = x["iord"]
    nties_act = x["nties"]
    y = np.repeat(0.0, nties_act)
    w2 = np.repeat(0.0, nties_act)

    res = weightedMean(y, w2, Target, w, x["iord"], x["ties"], n, nties_act)
    y = res[0]
    w2 = res[1]
    Result[x["iord"]] = x["x"][x["iord"]]
    if (normq > 0):
        Result = np.multiply(Result, np.sqrt(np.divide(normq, np.sum(np.multiply(w, np.square(Result))))))
    return (Result, b, iord3)

def weightedMean(y, sumwvec, target, w, iord, ties, n, nties):
    i = 0
    nprevties = 0
    for k in range(nties):
        sumwt = 0.0
        sumw = 0.0
        for l in range(ties[k]):
            ind = iord[nprevties]
            sumwt += float(w[ind]) * float(target[ind])
            sumw += float(w[ind])
        if sumw > 1e-10:
            y[k] = sumwt / sumw
        else:
            y[k] = 0
        
        sumwvec[k] = sumw
        nprevties += ties[k]

    return(y, sumwvec, target, w, iord, ties, n, nties)

def vecAsDist(x):
    n = int((1 + np.sqrt(1 + 8 * len(x))) / 2)
    e = np.zeros((n,n))
    k = 0
    for i in range(n):
        l = n-i
        ll = np.arange(n)
        e[i + ll, i] = x[k + ll]
        k = k + l
    
    return squareform(e, 'tovector')