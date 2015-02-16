#pragma once 

#include "common/geometry/segment.h"
#include "common/geometry/rectangle.h"
#include "common/geometry/geometry_utils.h"
#include "common/graph/dot_graph.h"

#include "segment_set.h"

void ForceDirectedAdjustment(const DotGraph& g, map<string, SegmentSet*>& trees);

class RoutingGraph;

class RoutingNode
{
	friend class RoutingGraph;

	string cluster;
	bool isVirtual;
	vector<RoutingNode*> neighbors;
	Point position;

public:
    static const double IdealRadius;
    static const double IdealHubWidth;

	RoutingNode(const string& cluster, const Point& position): cluster(cluster), position(position)
	{
		isVirtual = true;
	}

	inline Point Position() const
	{
		return position;
	}

	inline vector<RoutingNode*> Neighbors()
	{
		return neighbors;
	}
};

class RoutingGraph
{
	double ink;
	vector<RoutingNode*> nodes;

	vector<Rectangle> hardObstacles;

public:
	RoutingGraph(const DotGraph& g, const map<string, SegmentSet*>& trees)
	{
		//cut long tree segments
		for (auto iter = trees.begin(); iter != trees.end(); iter++)
		{
			string cluster = (*iter).first;
			SegmentSet* tree = (*iter).second;
			vector<Segment> newTree;

			for (int i = 0; i < tree->count(); i++)
			{
				Segment seg = tree->get(i);
				if (seg.length() > 250.0)
				{
					Segment seg1 = Segment(seg.first, seg.middle());
					Segment seg2 = Segment(seg.second, seg.middle());

					newTree.push_back(seg1);
					newTree.push_back(seg2);
				}
				else
				{
					newTree.push_back(seg);
				}
			}

			tree->clear();
			tree->append(newTree);
		}

		ink = 0;

		//create nodes and edges
		for (auto iter = trees.begin(); iter != trees.end(); iter++)
		{
			string cluster = (*iter).first;
			SegmentSet* tree = (*iter).second;

			for (int i = 0; i < tree->count(); i++)
			{
				Segment seg = tree->get(i);

				RoutingNode* node1 = findOrCreateNode(seg.first, cluster);
				RoutingNode* node2 = findOrCreateNode(seg.second, cluster);

				node1->neighbors.push_back(node2);
				node2->neighbors.push_back(node1);

				ink += seg.length();
			}
		}

		//fix real nodes (centers of obstacles)
		for (int i = 0; i < (int)g.nodes.size(); i++)
		{
			hardObstacles.push_back(g.nodes[i]->getBoundingRectangle());

			RoutingNode* realNode = NULL;
			for (int j = 0; j < (int)nodes.size(); j++)
				if (nodes[j]->position == g.nodes[i]->pos)
				{
					realNode = nodes[j];
					break;
				}

			if (realNode == NULL) continue;

			realNode->isVirtual = false;
			//realNode->cluster = "";
		}
	}

	RoutingNode* findOrCreateNode(const Point& pos, const string& cluster)
	{
		RoutingNode* node = findNode(pos, cluster);
		if (node != NULL) return node;

		node = new RoutingNode(cluster, pos);
		nodes.push_back(node);
		return node;
	}

	RoutingNode* findNode(const Point& pos, const string& cluster)
	{
		for (int i = 0; i < (int)nodes.size(); i++)
			if (nodes[i]->position == pos && nodes[i]->cluster == cluster) return nodes[i];
		return NULL;
	}

	~RoutingGraph()
	{
		for (int i = 0; i < (int)nodes.size(); i++)
			delete nodes[i];
	}

	vector<RoutingNode*> VirtualNodes()
	{
		vector<RoutingNode*> res;
		for (int i = 0; i < (int)nodes.size(); i++)
			if (nodes[i]->isVirtual) res.push_back(nodes[i]);

		return res;
	}

	vector<Point> VirtualNodesPositions()
	{
		vector<Point> res;
		for (int i = 0; i < (int)nodes.size(); i++)
			if (nodes[i]->isVirtual) res.push_back(nodes[i]->Position());

		return res;
	}

	void MoveNode(RoutingNode* node, const Point& newPosition)
	{
        //update ink
		for (int i = 0; i < (int)node->neighbors.size(); i++) 
		{
			RoutingNode* adj = node->neighbors[i];
            ink += (newPosition - adj->Position()).Length() - (node->position - adj->Position()).Length();
        }

		node->position = newPosition;
	}

	inline double Ink() const
	{
		return ink;
	}

	void UpdateTrees(map<string, SegmentSet*>& trees)
	{
		//remove old
		for (auto iter = trees.begin(); iter != trees.end(); iter++)
		{
			string cluster = (*iter).first;
			SegmentSet* tree = (*iter).second;
			vector<Segment> segsToAdd;
			for (int i = 0; i < tree->count(); i++)
			{
				Segment seg = tree->get(i);

				RoutingNode* node1 = findNode(seg.first, cluster);
				RoutingNode* node2 = findNode(seg.second, cluster);

				if ((node1 != NULL && !node1->isVirtual) && (node2 != NULL && !node2->isVirtual))
					segsToAdd.push_back(seg);
			}
			
			tree->clear();
			tree->append(segsToAdd);
		}

		//add new
		for (int i = 0; i < (int)nodes.size(); i++)
		{
			RoutingNode* node = nodes[i];
			for (int j = 0; j < (int)node->neighbors.size(); j++)
			{
				RoutingNode* adj = node->neighbors[j];
				Segment seg = Segment(node->position, adj->position);
				string cl = (node->isVirtual ? node->cluster : adj->cluster);
				if (cl != "") 
				{
					SegmentSet* tree = trees[cl];
					if (tree->contains(seg)) continue;
					tree->append(seg);
				}
			}
		}
	}

	bool HubAvoidsObstacles(RoutingNode* node, const Point& nodePosition, double idealR, vector<Point>& touchedObstacles)
	{
		set<Point> res;
		//node boundaries
		for (int i = 0; i < (int)hardObstacles.size(); i++)
		{
			if (hardObstacles[i].Contains(nodePosition)) return false;

			Point closestP = geometry::ClosestPoint(hardObstacles[i], nodePosition);
			if (closestP.Distance(nodePosition) >= idealR) continue;
			if (res.find(closestP) != res.end()) continue;

			res.insert(closestP);
			touchedObstacles.push_back(closestP);
		}

		//other trees
		map<string, Point> clusterToClosest;
		for (int i = 0; i < (int)nodes.size(); i++)
		{
			if (nodes[i]->cluster == node->cluster) continue;

			double curMinDist = INF;
			if (clusterToClosest.find(node->cluster) != clusterToClosest.end())
				curMinDist = clusterToClosest[node->cluster].Distance(nodePosition);

			for (int j = 0; j < (int)nodes[i]->neighbors.size(); j++)
			{
				Segment seg2 = Segment(nodes[i]->position, nodes[i]->neighbors[j]->position);
				Point closestP = geometry::ClosestPoint(seg2, nodePosition);
				double dist = closestP.Distance(nodePosition);
				if (dist >= idealR) continue;
				if (dist >= curMinDist) continue;

				clusterToClosest[nodes[i]->cluster] = closestP;
			}
		}

		for (auto iter = clusterToClosest.begin(); iter != clusterToClosest.end(); iter++)
		{
			assert((*iter).first != node->cluster);
			touchedObstacles.push_back((*iter).second);
		}

		return true;
	}

	bool BundleAvoidsObstacles(RoutingNode* node, RoutingNode* adj, const Point& nodePosition, const Point& adjPosition, 
		double idealDist, vector<pair<Point, Point> >& touchedObstacles)
	{
		//node boundaries
		for (int i = 0; i < (int)hardObstacles.size(); i++)
		{
			assert(node->isVirtual);
			if (!adj->isVirtual && hardObstacles[i].Contains(adjPosition)) continue;

			Point p1, p2;
			if (geometry::Intersect(hardObstacles[i], Segment(nodePosition, adjPosition), p1, p2)) return false;
			if ((p1 - p2).Length() >= idealDist) continue;
			touchedObstacles.push_back(make_pair(p1, p2));
		}

		//other trees
		for (int i = 0; i < (int)nodes.size(); i++)
		{
			if (nodes[i]->cluster == node->cluster) continue;

			Segment seg(nodePosition, adjPosition);
			for (int j = 0; j < (int)nodes[i]->neighbors.size(); j++)
			{
				Segment seg2 = Segment(nodes[i]->position, nodes[i]->neighbors[j]->position);
				if (Segment::EdgesIntersect(seg, seg2)) return false;
			}
		}

		return true;
	}
};

class CostCalculator
{
public:
    static const double InkImportance;
	static const double HubRepulsionImportance;
	static const double BundleRepulsionImportance;

	/// Cost of the whole graph
	static double Cost(RoutingGraph& vg) 
	{
		double cost = 0;

		//ink
		cost += InkImportance * vg.Ink();

        //hubs
		vector<RoutingNode*> vNodes = vg.VirtualNodes();
		for (int i = 0; i < (int)vNodes.size(); i++)
		{
			cost += RadiusCost(vg, vNodes[i], vNodes[i]->Position());
        }

		return cost;
	}

    /// Gain of ink
    static double InkGain(RoutingGraph& vg, RoutingNode* node, const Point& newPosition) 
	{
        //ink
        double oldInk = vg.Ink();
        double newInk = vg.Ink();
		vector<RoutingNode*> neighbors = node->Neighbors();
		for (int i = 0; i < (int)neighbors.size(); i++)
		{
			RoutingNode* adj = neighbors[i];
            Point adjPosition = adj->Position();
            newInk -= (adjPosition - node->Position()).Length();
            newInk += (adjPosition - newPosition).Length();
        }
		return (oldInk - newInk) * InkImportance;
    }

    /// Gain of radii
    static double RadiusGain(RoutingGraph& vg, RoutingNode* node, const Point& newPosition) 
	{
        double gain = 0;

		gain += RadiusCost(vg, node, node->Position());
        gain -= RadiusCost(vg, node, newPosition);

        return gain;
    }

    static double RadiusCost(RoutingGraph& vg, RoutingNode* node, const Point& newPosition)
	{
        double idealR = RoutingNode::IdealRadius;

		vector<Point> touchedObstacles;
        if (!vg.HubAvoidsObstacles(node, newPosition, idealR, touchedObstacles)) 
		{
			return INF;
        }

        double cost = 0;
		for (int i = 0; i < (int)touchedObstacles.size(); i++)
		{
			Point d = touchedObstacles[i];

            double dist = (d - newPosition).Length();
			if (idealR <= dist) continue;

			cost += (1.0 - dist/idealR) * (idealR - dist) * HubRepulsionImportance;
        }

        return cost;
    }

    /// Gain of bundles
    /// if a newPosition is not valid (e.g. intersect obstacles) the result is -inf
    static double BundleGain(RoutingGraph& vg, RoutingNode* node, const Point& newPosition)
	{
        double gain = 0;

		vector<RoutingNode*> neighbors = node->Neighbors();
		for (int i = 0; i < (int)neighbors.size(); i++)
		{
			double lgain = BundleCost(vg, node, neighbors[i], node->Position());
			assert(lgain < INF);
			gain += lgain;
        }

		for (int i = 0; i < (int)neighbors.size(); i++)
		{
            double lgain = BundleCost(vg, node, neighbors[i], newPosition);
			if (lgain >= INF) return -INF;
			gain -= lgain;
        }

        return gain;
    }

    static double BundleCost(RoutingGraph& vg, RoutingNode* node, RoutingNode* adj, const Point& newPosition)
	{
		double idealWidth = RoutingNode::IdealHubWidth;
		vector<pair<Point, Point> > closestPoints;

        //find conflicting obstacles
		if (!vg.BundleAvoidsObstacles(node, adj, newPosition, adj->Position(), idealWidth, closestPoints)) 
		{
            return INF;
        }

		double cost = 0;
		for (int i = 0; i < (int)closestPoints.size(); i++) 
		{
			pair<Point, Point> pair = closestPoints[i];
            double dist = (pair.first - pair.second).Length();

            if (idealWidth <= dist) continue;

			cost += (1.0 - dist / idealWidth) * (idealWidth - dist) * BundleRepulsionImportance;
        }

		return cost;
    }


};
