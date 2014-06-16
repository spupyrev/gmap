#include "metrics.h"
#include "segment.h"
#include <algorithm>

void findAverageDistances(Graph& g, double& avgGeom, double& avgIdeal)
{
	avgGeom = 0;
	avgIdeal = 0;
	double cnt = 0;

	for (int i = 0; i < (int)g.nodes.size(); i++)
		for (int j = i+1; j < (int)g.nodes.size(); j++)
		{
			Node* s = g.nodes[i];
			Node* t = g.nodes[j];

			double geomDistance = s->getPos().Distance(t->getPos());
			double idealDistance = g.getShortestPath(s, t, true);
			//not connected
			if (idealDistance < 0) continue;

			avgGeom += geomDistance;
			avgIdeal += idealDistance;
			cnt++;
		}

	avgGeom /= cnt;
	avgIdeal /= cnt;
	assert(avgGeom > EPS && avgIdeal > EPS);
}

double computeMdsStressRelative(Graph& g)
{
	//TODO: 1. scale before calculations!
	//TODO: 2. merge two stresses

	if (g.edges.size() <= 0) return UNDEF;

	double m = 0;
	double sumGeom = 0;
	double sumIdeal = 0;
	for (int i = 0; i < (int)g.edges.size(); i++)
 	{
		Edge* e = g.edges[i];
		Node* s = g.findNodeById(e->s);
		Node* t = g.findNodeById(e->t);

		double geomDistance = s->getPos().Distance(t->getPos());
		double idealDistance = e->getLen();

		m += e->getWeight();
		sumGeom += geomDistance;
		sumIdeal += idealDistance;
	}

	double mdsStress = 0;
	for (int i = 0; i < (int)g.edges.size(); i++)
 	{
		Edge* e = g.edges[i];
		Node* s = g.findNodeById(e->s);
		Node* t = g.findNodeById(e->t);

		double geomDistance = s->getPos().Distance(t->getPos());
		double idealDistance = e->getLen();

		if (idealDistance < 0) return UNDEF;

		double w = e->getWeight() / m;
		double gd = geomDistance / sumGeom;
		double id = idealDistance / sumIdeal;

		mdsStress += w * Sqr2(Abs(gd - id)/max(gd, id));
	}

	return 1.0 - mdsStress;
}

double computeSparseStress(Graph& g)
{
	//TODO: 1. scale before calculations!
	//TODO: 2. merge two stresses

	if (g.edges.size() <= 0) return UNDEF;

	double mdsStress = 0;

	for (int i = 0; i < (int)g.edges.size(); i++)
 	{
		Edge* e = g.edges[i];
		Node* s = g.findNodeById(e->s);
		Node* t = g.findNodeById(e->t);

		double geomDistance = s->getPos().Distance(t->getPos());
		double idealDistance = e->getLen();

		if (idealDistance < 0) return UNDEF;

		double diffDistance = geomDistance - idealDistance;

		mdsStress += Sqr2(diffDistance) / Sqr2(idealDistance);
	}

	return sqrt(mdsStress);
}

double computeScalingFactor(Graph& g)
{
	double num = 0;
	double den = 0;

	for (int i = 0; i < (int)g.nodes.size(); i++)
		for (int j = i+1; j < (int)g.nodes.size(); j++)
		{
			Node* s = g.nodes[i];
			Node* t = g.nodes[j];

			double geomDistance = s->getPos().Distance(t->getPos());
			double idealDistance = g.getShortestPath(s, t, true);
			//not connected
			if (idealDistance < 0) continue;

			num += geomDistance / idealDistance;
			assert(idealDistance > EPS);
			den += Sqr2(geomDistance) / Sqr2(idealDistance);
		}

	assert(Abs(den) > EPS);
	return num / den;
}

double computeFullStress(Graph& g)
{
	if (g.edges.size() <= 0) return UNDEF;

	double s = computeScalingFactor(g);
	assert(s > 0.0);
	double stress = 0;
	for (int i = 0; i < (int)g.nodes.size(); i++)
		for (int j = i+1; j < (int)g.nodes.size(); j++)
		{
			Node* sn = g.nodes[i];
			Node* tn = g.nodes[j];

			double geomDistance = sn->getPos().Distance(tn->getPos());
			double idealDistance = g.getShortestPath(sn, tn, true);
			//not connected
			if (idealDistance < 0) continue;

			double wij = 1.0 / Sqr2(idealDistance);

			double delta = s*geomDistance - idealDistance;
			stress += Sqr2(delta) / Sqr2(idealDistance);
		}

	return stress;
}

double computeDistortion(Graph& g)
{
	if (g.edges.size() <= 0) return UNDEF;

	double avgGeom = 0;
	double avgIdeal = 0;
	findAverageDistances(g, avgGeom, avgIdeal);

	double sumAB = 0, sumA = 0, sumB = 0;
	for (int i = 0; i < (int)g.nodes.size(); i++)
		for (int j = i+1; j < (int)g.nodes.size(); j++)
		{
			Node* s = g.nodes[i];
			Node* t = g.nodes[j];

			double geomDistance = s->getPos().Distance(t->getPos());
			double idealDistance = g.getShortestPath(s, t, true);
			//not connected
			if (idealDistance < 0) continue;

			sumAB += (geomDistance - avgGeom)*(idealDistance - avgIdeal);
			sumA += (geomDistance - avgGeom)*(geomDistance - avgGeom);
			sumB += (idealDistance - avgIdeal)*(idealDistance - avgIdeal);
		}

	if (Abs(sumAB) < EPS) return 0.5;

	double distortion = sumAB / (sqrt(sumA)*sqrt(sumB));
	//normalizing
	return (1.0 + distortion)/2.0;
}

vector<Node*> findClosestNodes(Graph& g, int K, Node* s)
{
	vector<pair<double, Node*> > distances;
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		Node* t = g.nodes[i];
		distances.push_back(make_pair(s->getPos().Distance(t->getPos()), t));
	}

	sort(distances.begin(), distances.end());
	vector<Node*> res;
	for (int i = 0; i < (int)distances.size() && i < K; i++)
		res.push_back(distances[i].second);
	return res;
}

double computeThreshold(Graph& g, int K, Node* s)
{
	vector<pair<double, Node*> > distances;
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		Node* t = g.nodes[i];
		double sp = g.getShortestPath(s, t, true);
		if (sp == -1) continue;
		distances.push_back(make_pair(sp, t));
	}

	sort(distances.begin(), distances.end());
	if ((int)distances.size() <= K) return distances.back().first;
	return distances[K-1].first;
}

double computeNeigPreservation(Graph& g)
{
	int K = 20;
	if (g.edges.size() <= 0) return UNDEF;

	double np = 0, cnt = 0;
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		Node* s = g.nodes[i];

		vector<Node*> closestNodes = findClosestNodes(g, K, s);
		double threshold = computeThreshold(g, K, s);

		double good = 0, all = 0;
		for (int j = 0; j < (int)closestNodes.size(); j++)
		{
			double sp = g.getShortestPath(s, closestNodes[j], true);

			if (sp != -1 && sp <= threshold) good++;
			all++;
		}

		if (all > 0)
		{
			np += good/all;
			cnt++;
		}
	}

	if (cnt == 0) return UNDEF;
	return np/cnt;
}

Rectangle computeBoundingBox(const Graph& g)
{
	if (g.nodes.size() <= 0) return Rectangle();

	Point pLB, pRT;
	pLB.x = pRT.x = g.nodes[0]->getPos().x;
	pLB.y = pRT.y = g.nodes[0]->getPos().y;

	for (int i = 1; i < (int)g.nodes.size(); i++)
	{
		pLB.x = min(pLB.x, g.nodes[i]->getPos().x);
		pRT.x = max(pRT.x, g.nodes[i]->getPos().x);
		pLB.y = min(pLB.y, g.nodes[i]->getPos().y);
		pRT.y = max(pRT.y, g.nodes[i]->getPos().y);
	}

	return Rectangle(pLB, pRT);
}

double computeUniform(const Graph& g)
{
	Rectangle bb = computeBoundingBox(g);

	//number of cells
	int W = (int)sqrt((double)g.nodes.size() + 1.0);
	int H = (int)sqrt((double)g.nodes.size() + 1.0);

	double entropy = 0;
	for (int i = 0; i < W; i++)
		for (int j = 0; j < H; j++) {
			double cellWidth = bb.getWidth()/W;
			double cellHeight = bb.getHeight()/H;
			Rectangle cell(bb.xl + cellWidth * i, bb.yl + cellHeight * j, bb.xl + cellWidth * (i+1), bb.yl + cellHeight * (j+1));

			//observed frequency (words inside cell)
			double p = 0;
			for (int k = 0; k < (int)g.nodes.size(); k++)
				if (cell.contains(g.nodes[k]->getPos())) p++;
			
			p /= (double)g.nodes.size();
			if (Abs(p) < EPS) continue;

			//expected frequency
			double q = (double) 1.0 / (W * H);
				
			entropy += p * log(p / q);
		}

	double maxEntropy = log((double)g.nodes.size());

	return 1.0 - entropy / maxEntropy;
}

double computeCompactness(const Graph& g)
{
	Rectangle bb = computeBoundingBox(g);

	double totalArea = bb.getWidth() * bb.getHeight();
	if (Abs(totalArea) < EPS) return UNDEF;

	double usedArea = 0;
	for (int i = 0; i < (int)g.nodes.size(); i++)
	{
		double w = g.nodes[i]->getDoubleAttr("width");
		double h = g.nodes[i]->getDoubleAttr("height");

		usedArea += w*h;
	}

	return usedArea/totalArea;
}

double computeAspectRatio(const Graph& g)
{
	if (g.nodes.size() <= 0) return UNDEF;

	Rectangle bb = computeBoundingBox(g);
	bb = computeBoundingBox(g);

	if (Abs(bb.getHeight()) < EPS) return UNDEF;

	double mx = max(bb.getWidth(), bb.getHeight());;
	double mn = min(bb.getWidth(), bb.getHeight());;
	return mx / mn;
}

double computeCrossings(Graph& g, double& minCrossAngle, double& avgCrossAngle)
{
	minCrossAngle = avgCrossAngle = UNDEF;
	if (g.nodes.size() <= 0) return UNDEF;

	VD crossAngles;
	int cr = 0;
	for (int i = 0; i < (int)g.edges.size(); i++)
		for (int j = i+1; j < (int)g.edges.size(); j++)
		{
			Edge* e1 = g.edges[i];
			Edge* e2 = g.edges[j];

			Node* s1 = g.findNodeById(e1->s);
			Node* t1 = g.findNodeById(e1->t);
			Node* s2 = g.findNodeById(e2->s);
			Node* t2 = g.findNodeById(e2->t);

			if (Segment::EdgesIntersect(s1->getPos(), t1->getPos(), s2->getPos(), t2->getPos()))
			{
				cr++;
				double angle = Segment::CrossingAngle(s1->getPos(), t1->getPos(), s2->getPos(), t2->getPos());
				assert(angle >= 0.0 && angle <= 1.5707963268);
				crossAngles.push_back(angle);
			}
		}

	minCrossAngle = MinimumValue(crossAngles);
	avgCrossAngle = AverageValue(crossAngles);
	return cr;
}

double computeModularity(Graph& g)
{
	try
	{
		double res = 0;

		double m = 0;
		for (int i = 0; i < (int)g.edges.size(); i++)
		{
			double w = g.edges[i]->getWeight();
			m += w;
		}

		for (int i = 0; i < (int)g.edges.size(); i++)
		{
			Node* s = g.findNodeById(g.edges[i]->s);
			Node* t = g.findNodeById(g.edges[i]->t);

			string cluster_s = s->getAttr("cluster");
			string cluster_t = t->getAttr("cluster");
			if (cluster_s != cluster_t) continue;

			double w = g.edges[i]->getWeight();
			double deg_s = g.weightedDegree(s);
			double deg_t = g.weightedDegree(t);

			res += (w - deg_s*deg_t/(2.0*m));
		}

		res /= (2.0*m);

		return res;
	}
	catch (int e)
	{
		if (e == 1) return UNDEF;
		throw e;
	}
}

double computeCoverage(Graph& g)
{
	try
	{
		double res = 0;
		double sumWeight = 0;

		for (int i = 0; i < (int)g.edges.size(); i++)
		{
			Node* s = g.findNodeById(g.edges[i]->s);
			Node* t = g.findNodeById(g.edges[i]->t);

			string cluster_s = s->getAttr("cluster");
			string cluster_t = t->getAttr("cluster");
			double w = g.edges[i]->getWeight();

			sumWeight += w;
			if (cluster_s == cluster_t)
				res += w;
		}

		if (sumWeight < EPS) return UNDEF;
		return res/sumWeight;
	}
	catch (int e)
	{
		if (e == 1) return UNDEF;
		throw e;
	}
}

double computeConductance(Graph& g)
{
	try
	{
		map<string, double> clusterWeight;
		map<string, double> intraEdges;
		for (int i = 0; i < (int)g.nodes.size(); i++)
		{
			string cl = g.nodes[i]->getAttr("cluster");
			clusterWeight[cl] = 0;
			intraEdges[cl] = 0;
		}

		double sum_w = 0;
		for (int i = 0; i < (int)g.edges.size(); i++)
		{
			Node* s = g.findNodeById(g.edges[i]->s);
			Node* t = g.findNodeById(g.edges[i]->t);
			string cluster_s = s->getAttr("cluster");
			string cluster_t = t->getAttr("cluster");
			double w = g.edges[i]->getWeight();
			sum_w += w;

			clusterWeight[cluster_s] += w;
			if (cluster_s != cluster_t)
				clusterWeight[cluster_t] += w;

			if (cluster_s == cluster_t) continue;

			intraEdges[cluster_s] += w;
			intraEdges[cluster_t] += w;
		}

		double cond = 0, cnt = 0;
		for (map<string, double>::iterator iter = clusterWeight.begin(); iter != clusterWeight.end(); iter++)
		{
			string cl = (*iter).first;
			
			double cut = intraEdges[cl];
			double weight = (double)(*iter).second;

			if (min(weight, sum_w - weight) < EPS) continue;

			cond += cut/min(weight, sum_w - weight + cut);
			cnt++;
		}

		if (cnt < EPS) return UNDEF;
		return 1.0 - cond/cnt;
	}
	catch (int e)
	{
		if (e == 1) return UNDEF;
		throw e;
	}
}

Metrics computeMetrics(Graph& g)
{
	Metrics m;
	m.n = g.nodes.size();
	m.m = g.edges.size();
	m.c = g.numberOfClusters();

	m.sparseStress = computeSparseStress(g);
	m.fullStress = computeFullStress(g);
	//m.distortion = computeDistortion(g);
	//m.neigPreservation = computeNeigPreservation(g);
	//m.compactness = computeCompactness(g);
	//m.uniform = computeUniform(g);
	//m.aspectRatio = computeAspectRatio(g);
	m.crossings = computeCrossings(g, m.minCrossAngle, m.avgCrossAngle);

	m.modularity = computeModularity(g);
	m.coverage = computeCoverage(g);
	m.conductance = computeConductance(g);

	return m;
}
