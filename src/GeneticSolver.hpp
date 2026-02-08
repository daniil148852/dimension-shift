#pragma once
#include <vector>
#include <random>

struct Chromosome {
    std::vector<bool> genes;
};

class GeneticSolver {
public:
    void initPopulation(std::vector<Chromosome>& pop, int geneCount);

private:
    int m_populationSize = 100;
    std::mt19937 m_rng{std::random_device{}()};
};
