#pragma once

#include "common/geometry/segment.h"

class SegmentSet
{
private:
	SegmentSet(const SegmentSet&);
	SegmentSet& operator = (const SegmentSet&);

	vector<Segment> segments;
	set<Segment> hash;

public:
	SegmentSet() {}

	Segment get(int index) const
	{
		return segments[index];
	}

	int count() const
	{
		return (int)segments.size();
	}

	void append(const vector<Segment>& segs)
	{
		for (int i = 0; i < (int)segs.size(); i++)
			append(segs[i]);
	}

	void append(const Segment& seg)
	{
		segments.push_back(seg);
		hash.insert(seg);
	}

	bool contains(const Segment& seg) const
	{
		return (hash.find(seg) != hash.end() || hash.find(Segment(seg.second, seg.first)) != hash.end());
	}

	void clear()
	{
		segments.clear();
		hash.clear();
	}

	double Length() const
	{
		double res = 0;
		for (int i = 0; i < (int)segments.size(); i++)
			res += segments[i].length();
		return res;
	}

	void CheckTree() const
	{
		for (int i = 0; i < (int)segments.size(); i++)
			for (int j = i+1; j < (int)segments.size(); j++)
				assert(segments[i] != segments[j]);
	}

	vector<Point> getPoints() const
	{
		set<Point> all;
		for (int i = 0; i < (int)segments.size(); i++)
		{
			all.insert(segments[i].first);
			all.insert(segments[i].second);
		}

		return vector<Point>(all.begin(), all.end());
	}

};
