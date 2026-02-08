#include "GeneticSolver.hpp"

void GeneticSolver::initPopulation(std::vector<Chromosome>& pop, int geneCount) {
    pop.resize(m_populationSize);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // 1. First chromosome: Do absolutely nothing (release everything)
    pop[0].genes.resize(geneCount, false);

    // 2. Second chromosome: Hold jump constantly
    pop[1].genes.resize(geneCount, true);

    // 3. Third chromosome: Pulse (jump every 10 frames)
    pop[2].genes.resize(geneCount);
    for(int i=0; i<geneCount; ++i) pop[2].genes[i] = (i % 20 < 5);

    // Fill the rest randomly
    for (size_t i = 3; i < pop.size(); i++) {
        pop[i].genes.resize(geneCount);
        
        // Use different strategies for different parts of population
        float clickProb; 
        if (i < pop.size() / 2) clickProb = 0.05f; // Sparse clicks (Cube/Robot)
        else clickProb = 0.3f; // Frequent clicks (Ship/Wave)

        bool currentState = false;
        // Random start state
        if (dist(m_rng) < 0.5f) currentState = true;

        for (int j = 0; j < geneCount; j++) {
            // Change state with probability
            if (dist(m_rng) < clickProb) {
                currentState = !currentState;
            }
            pop[i].genes[j] = currentState;
        }
    }
}
