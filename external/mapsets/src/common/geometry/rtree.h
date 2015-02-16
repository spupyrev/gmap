#pragma once 

#include "common/geometry/rectangle.h"
#include <cassert>

template<class T>
class RNode
{
    Rectangle rectangle;
	T data;

    RNode* left;
    RNode* right;
    RNode* parent;

	RNode(const RNode&);
	RNode& operator = (const RNode&);

public:
	RNode() {}
	RNode(Rectangle rect, T data) : rectangle(rect), data(data), left(nullptr), right(nullptr), parent(nullptr) {}

	~RNode()
	{
		if (left != nullptr) 
			delete left;
		if (right != nullptr) 
			delete right;
	}

    /// false if it is an internal node and true if it is a leaf
    bool IsLeaf() const
    {
        return left == nullptr;
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
		if (left != nullptr && left->parent == this)
			left->parent = nullptr;

		left = value;
		if (left != nullptr)
			left->parent = this;
    }

    void setRight(RNode* value)
	{
		if (right != nullptr && right->parent == this)
			right->parent = nullptr;

		right = value;
		if (right != nullptr)
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
    T getData() const
	{
		return data;
	}

    /// brings the first leaf which rectangle was hit and the delegate is happy with the object
    RNode* FirstHitNode(const Point& point) 
	{
		if (rectangle.Contains(point)) 
		{
            if (IsLeaf())                    
                return this;

			RNode* node = getLeft()->FirstHitNode(point);
			if (node != nullptr) return node;

			return getRight()->FirstHitNode(point);
        }

        return nullptr;
    }
};

template<class T>
class RTree
{
	RNode<T>* root;

public:
	RTree(const vector<RNode<T>*>& nodes)
	{
		root = CreateRNode(nodes);
	}

	~RTree()
	{
		delete root;
	}

	inline RNode<T>* getRoot() const
	{
		return root;
	}

private:
	RTree(const RTree&);
	RTree& operator = (const RTree&);

	RNode<T>* CreateRNode(const vector<RNode<T>*>& nodes)
	{
		if (nodes.empty()) return nullptr;
		if (nodes.size() == 1) return nodes[0];

		//Finding the seeds
		int seed0 = -1, seed1 = -1;
		ChooseSeeds(nodes, seed0, seed1);
		assert(seed0 != -1 && seed1 != -1);

		//We have two seeds at hand. Build two groups.
		vector<RNode<T>*> gr0;
		vector<RNode<T>*> gr1;

		gr0.push_back(nodes[seed0]);
		gr1.push_back(nodes[seed1]);

		Rectangle box0 = nodes[seed0]->getRectangle();
		Rectangle box1 = nodes[seed1]->getRectangle();

		//divide nodes on two groups
		DivideNodes(nodes, seed0, seed1, gr0, gr1, box0, box1);

		RNode<T>* ret = new RNode<T>(Rectangle(box0, box1), T());
		ret->setLeft(CreateRNode(gr0));
		ret->setRight(CreateRNode(gr1));

		return ret;
	}

	void ChooseSeeds(const vector<RNode<T>*>& nodes, int& seed0, int& seed1)
	{
		Rectangle b0 = nodes[0]->getRectangle();

		seed0 = 1;
		double area = Rectangle(b0, nodes[seed0]->getRectangle()).area();
		for (int i = 2; i < (int)nodes.size(); i++) 
		{
			double area0 = Rectangle(b0, nodes[i]->getRectangle()).area();
			if (area0 > area) 
			{
				seed0 = i;
				area = area0;
			}
		}

		//Got the first seed seed0
		//Now looking for a seed for the second group
		seed1 = 0;

		//init seed1
		for (int i = 0; i < (int)nodes.size(); i++) 
		{
			if (i != seed0) 
			{
				seed1 = i;
				break;
			}
		}

		area = Rectangle(nodes[seed0]->getRectangle(), nodes[seed1]->getRectangle()).area();

		//Now try to improve the second seed
		for (int i = 0; i < (int)nodes.size(); i++) 
		{
			if (i == seed0) continue;

			double area1 = Rectangle(nodes[seed0]->getRectangle(), nodes[i]->getRectangle()).area();
			if (area1 > area) 
			{
				seed1 = i;
				area = area1;
			}
		}
	}

	void DivideNodes(const vector<RNode<T>*>& nodes, int seed0, int seed1, vector<RNode<T>*>& gr0, vector<RNode<T>*>& gr1, Rectangle& box0, Rectangle& box1)
	{
		int groupSplitThreshold = 2;

		for (int i = 0; i < (int)nodes.size(); i++) 
		{
			if (i == seed0 || i == seed1) continue;

			Rectangle box0_ = Rectangle(box0, nodes[i]->getRectangle());
			double delta0 = box0_.area() - box0.area();

			Rectangle box1_ = Rectangle(box1, nodes[i]->getRectangle());
			double delta1 = box1_.area() - box1.area();

			//keep the tree roughly balanced
			if (gr0.size() * groupSplitThreshold < gr1.size()) 
			{
				gr0.push_back(nodes[i]);
				box0 = box0_;
			} 
			else if (gr1.size() * groupSplitThreshold < gr0.size()) 
			{
				gr1.push_back(nodes[i]);
				box1 = box1_;
			} 
			else if (delta0 < delta1)
			{
				gr0.push_back(nodes[i]);
				box0 = box0_;
			} 
			else if (delta1 < delta0)
			{
				gr1.push_back(nodes[i]);
				box1 = box1_;
			} 
			else if (box0.area() < box1.area())
			{
				gr0.push_back(nodes[i]);
				box0 = box0_;
			} 
			else 
			{
				gr1.push_back(nodes[i]);
				box1 = box1_;
			}
		}
	}
};
