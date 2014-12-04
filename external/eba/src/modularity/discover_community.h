#pragma once

#include "common/graph/dot_graph.h"

namespace modularity {

vector<vector<DotNode*> > runModularity(ConnectedDotGraph& g);

} // namespace modularity
