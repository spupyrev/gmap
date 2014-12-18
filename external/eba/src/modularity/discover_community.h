#pragma once

#include "common/graph/dot_graph.h"

namespace modularity {

vector<vector<DotNode*> > runModularity(DotGraph& g, bool contiguity);

} // namespace modularity
