#pragma once 

#include "common/geometry/segment.h"
#include "common/geometry/rectangle.h"

class RNode
{
    Rectangle rectangle;
	Segment data;

    RNode* left;
    RNode* right;
    RNode* parent;

	RNode(const RNode&);
	RNode& operator = (const RNode&);

public:
	RNode() {}
	RNode(Rectangle rect, Segment data) : rectangle(rect), data(data), left(NULL), right(NULL), parent(NULL) {}

    /// false if it is an internal node and true if it is a leaf
    bool IsLeaf() const
    {
        return left == NULL;
    }

    RNode* getLeft() const
	{
		return left;
    }

    RNode* getRight() const
	{
		return right;
    }

    void setLeft(RNode* value)
	{
		if (left != NULL && left->parent == this)
			left->parent = NULL;

		left = value;
		if (left != NULL)
			left->parent = this;
    }

    void setRight(RNode* value)
	{
		if (right != NULL && right->parent == this)
			right->parent = NULL;

		right = value;
		if (right != NULL)
			right->parent = this;
    }

    RNode* getParent() const
	{
		return parent;
    }

    Rectangle getRectangle() const
	{
		return rectangle;
    }

    /// <summary>
    /// The actual data if a leaf node, else null or a value-type default.
    /// </summary>
    Segment getData() const
	{
		return data;
	}

    /// brings the first leaf which rectangle was hit and the delegate is happy with the object
    RNode* FirstHitNode(const Point& point) 
	{
		if (rectangle.contains(point)) 
		{
            if (IsLeaf())                    
                return this;

			RNode* node = getLeft()->FirstHitNode(point);
			if (node != NULL) return node;

			return getRight()->FirstHitNode(point);
        }

        return NULL;
    }

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

};

class RTree
{
	RNode* root;

private:
	RTree(const RTree&);
	RTree& operator = (const RTree&);

	RNode* CreateRNode(const vector<RNode*>& nodes);
	void DivideNodes(const vector<RNode*>& nodes, int seed0, int seed1, vector<RNode*>& gr0, vector<RNode*>& gr1, Rectangle& box0, Rectangle& box1);
	void ChooseSeeds(const vector<RNode*>& nodes, int& seed0, int& seed1);

public:
	RTree(const vector<RNode*>& nodes);

	inline RNode* getRoot() const
	{
		return root;
	}
};
