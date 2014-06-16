#include "rectangle_node.h"

void IntersectRectangleWithTree(RectangleNode* node, const Rectangle& rect, vector<RectangleNode*>& touchedNodes)
{
	if (!node->getRectangle().intersects(rect)) return;

	if (!node->IsLeaf()) 
	{
		IntersectRectangleWithTree(node->getLeft(), rect, touchedNodes);
		IntersectRectangleWithTree(node->getRight(), rect, touchedNodes);
    }
    else 
	{
		touchedNodes.push_back(node);
    }
}
