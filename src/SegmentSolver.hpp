#pragma once
#include <vector>
#include "PhysicsEngine.hpp"
#include "ReplayRecorder.hpp"

class SegmentSolver {
public:
    bool solve(const LevelPhysicsData& levelData, ReplayRecorder& recorder);

private:
    double m_tps = 240.0;
    std::vector<void*> createSegments(const LevelPhysicsData& data) { return {}; }
};
