#include "fd_adjustment.h"

const double RoutingNode::IdealRadius = 105.0;
const double RoutingNode::IdealHubWidth = 105.0;
const double CostCalculator::InkImportance = 0.01;
const double CostCalculator::HubRepulsionImportance = 100.0;
const double CostCalculator::BundleRepulsionImportance = 100.0;

const int MaxIterations = 100;
const double MaxStep = 50;
const double MinStep = 1;
const double MinRelativeChange = 0.0005;

int stepsWithProgress = 0;
double UpdateMaxStep(double step, double oldEnergy, double newEnergy) 
{
    //cooling factor
    double T = 0.8;
    if (newEnergy + 1.0 < oldEnergy) 
	{
        stepsWithProgress++;
        if (stepsWithProgress >= 5) 
		{
            stepsWithProgress = 0;
            step = min(MaxStep, step / T);
        }
    }
    else 
	{
        stepsWithProgress = 0;
        step *= T;
    }

    return step;
}

/// stop SA if relative changes are small
bool Converged(double step, vector<Point>& oldx, vector<Point>& newx) 
{
    double num = 0, den = 0;
    for (int i = 0; i < (int)oldx.size(); i++) 
	{
        num += (oldx[i] - newx[i]).LengthSquared();
        den += oldx[i].LengthSquared();
    }
    double res = sqrt(num / den);
    return (res < MinRelativeChange);
}

/// force to decrease ink
Point BuildForceForInk(RoutingGraph& vg, RoutingNode* node) 
{
    Point direction = Point();
	vector<RoutingNode*> neighbors = node->Neighbors();
	for (int i = 0; i < (int)neighbors.size(); i++)
	{
		RoutingNode* adj = neighbors[i];
        Point p = (adj->Position() - node->Position());
        direction += p / p.Length();
    }

    //derivative
	Point force = direction * CostCalculator::InkImportance;

    return force;
}

/// direction to increase radii
Point BuildForceForRadius(RoutingGraph& vg, RoutingNode* node)
{
    Point direction = Point();

	double idealR = RoutingNode::IdealRadius;
	vector<Point> touchedObstacles;
	bool res = vg.HubAvoidsObstacles(node, node->Position(), idealR, touchedObstacles);
	assert(res);

	for (int i = 0; i < (int)touchedObstacles.size(); i++)
	{
		Point d = touchedObstacles[i];

        double dist = (d - node->Position()).Length();
		assert(dist <= idealR);
        double lforce = 2.0 * (1.0 - dist / idealR);
        Point dir = (node->Position() - d);
		dir.Normalize();
        direction += dir * lforce;
    }

    //derivative
    Point force = direction * CostCalculator::HubRepulsionImportance;

    return force;
}

/// direction to push a bundle away from obstacle
Point BuildForceForBundle(RoutingGraph& vg, RoutingNode* node)
{
    Point direction = Point();
	vector<RoutingNode*> neigbors = node->Neighbors();
	for (int i = 0; i < (int)neigbors.size(); i++)
	{
		RoutingNode* adj = neigbors[i];
		double idealWidth = RoutingNode::IdealHubWidth;

		vector<pair<Point, Point> > closestPoints;
        bool res = vg.BundleAvoidsObstacles(node, adj, node->Position(), adj->Position(), idealWidth, closestPoints);
		assert(res);

		for (int i = 0; i < (int)closestPoints.size(); i++)
		{
			pair<Point, Point> d = closestPoints[i];

            double dist = (d.first - d.second).Length();
			assert(LessOrEqual(dist, idealWidth));
            double lforce = 2.0 * (1.0 - dist / idealWidth);
            Point dir = (d.second - d.first);
			dir.Normalize();
            direction += dir * lforce;
        }
    }

    //derivative
	Point force = direction * CostCalculator::BundleRepulsionImportance;

    return force;
}

/// Calculate the direction to improve the ink function
Point BuildDirection(RoutingGraph& vg, RoutingNode* node) 
{
    Point forceInk = BuildForceForInk(vg, node);
    Point forceR = BuildForceForRadius(vg, node);
	Point forceBundle = BuildForceForBundle(vg, node);

    Point force = forceInk + forceR + forceBundle;
    if (force.Length() < 0.1) return Point();
    force.Normalize();

    return force;
}

/// Computes cost delta when moving the node
/// the cost will be negative if a new position overlaps obstacles
double CostGain(RoutingGraph& vg, RoutingNode* node, Point newPosition) 
{
    double MInf = -12345678.0;
    double rGain = CostCalculator::RadiusGain(vg, node, newPosition);
    if (rGain < MInf) return MInf;
    double bundleGain = CostCalculator::BundleGain(vg, node, newPosition);
    if (bundleGain < MInf) return MInf;
    double inkGain = CostCalculator::InkGain(vg, node, newPosition);

    return rGain + inkGain + bundleGain;
}

double BuildStepLength(RoutingGraph& vg, RoutingNode* node, Point direction, double maxStep) 
{
    double stepLength = MinStep;

    double costGain = CostGain(vg, node, node->Position() + direction * stepLength);
    if (costGain < 0.01)
        return 0;

    while (2 * stepLength <= MaxStep) 
	{
        double newCostGain = CostGain(vg, node, node->Position() + direction * stepLength * 2);
        if (newCostGain <= costGain)
            break;

        stepLength *= 2;
        costGain = newCostGain;
    }

    return stepLength;
}

/// Move node to decrease the cost of the drawing
/// Returns true iff position has changed
bool TryMoveNode(RoutingGraph& vg, RoutingNode* node, double maxStep) 
{
    Point direction = BuildDirection(vg, node);
    if (direction.Length() < EPS) return false;

    double stepLength = BuildStepLength(vg, node, direction, maxStep);
    if (stepLength < MinStep) 
	{
        //try random direction
        direction = Point::RandomPoint();
        stepLength = BuildStepLength(vg, node, direction, maxStep);
        if (stepLength < MinStep)
            return false;
    }

    Point step = direction * stepLength;
    Point newPosition = node->Position() + step;
    //can this happen?
    //if (vg.PointToStations.ContainsKey(newPosition)) return false;

    vg.MoveNode(node, newPosition);
    return true;
}

bool TryMoveNodes(RoutingGraph& vg, double step) 
{
    bool coordinatesChanged = false;
	vector<RoutingNode*> vNodes = vg.VirtualNodes();
	for (int i = 0; i < (int)vNodes.size(); i++)
	{
		RoutingNode* node = vNodes[i];
        if (TryMoveNode(vg, node, step)) 
		{
            coordinatesChanged = true;
        }
    }

    return coordinatesChanged;
}

void ForceDirectedAdjustment(const DotGraph& g, map<string, SegmentSet*>& trees)
{
	double step = MaxStep;
	double energy = INF;

	RoutingGraph vg = RoutingGraph(g, trees);
	vector<Point> x = vg.VirtualNodesPositions();
	int iteration = 0;
	while (iteration++ < MaxIterations) 
	{
		bool coordinatesChanged = TryMoveNodes(vg, step);
		if (!coordinatesChanged) break;

		double oldEnergy = energy;
		energy = CostCalculator::Cost(vg);

		step = UpdateMaxStep(step, oldEnergy, energy);
		vector<Point> oldX = x;
		x = vg.VirtualNodesPositions();
		if (step < MinStep || Converged(step, oldX, x)) break;
	}

	vg.UpdateTrees(trees);
}

