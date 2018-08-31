#ifndef FSMALGORITHMS_H
#define FSMALGORITHMS_H
#include "algorithms.h"

class Algorithms_FSM : public Algorithms
{
public:
    Algorithms_FSM(bool generateLogs, bool onlyDot);

    FSM * generateSubmachine(CMSat::SATSolver * &solver, FSM * M);

    void computePhiP(CMSat::SATSolver * &solver, FSM * P);

    void computePhiE(CMSat::SATSolver * &solver, std::vector<sequence> E, DistinguishingAutomaton_FSM * D);

    void computePhiM(CMSat::SATSolver * &solver, FSM * S, FSM * M);

    sequence verifyCheckingExperiment(CMSat::SATSolver * &solver,std::vector<sequence> E, FSM * S, DistinguishingAutomaton_FSM * D);

    std::vector<sequence> generateCheckingExperimentTimeouted(std::vector<sequence> Einit, FSM * S, FSM * M);

    std::vector<sequence> generateCheckingExperiment(std::vector<sequence> Einit, FSM * S, FSM * M);

    sequence verifyCheckingSequence(CMSat::SATSolver * &solver,sequence CS, FSM * S, DistinguishingAutomaton_FSM * D);

    sequence generateCheckingSequenceTimeouted(FSM * S, FSM * M);

    sequence generateCheckingSequence(FSM * S, FSM * M);

    FSM * generateRandomSpecification(int nbOfStates, int maxTime, std::set<std::string> I, std::set<std::string> O);

    FSM * generateRandomMutation(FSM * S, int maxTime, int numberOfMutations);

    void checkingExperimentBenchmarks(std::string folder, std::set<int> nbStates, std::set<int> nbMutations, int nbMachines, int timeoutedValue, int maxTimeout);

    void checkingSequenceBenchmarks(std::string folder, std::set<int> nbStates, std::set<int> nbMutations, int nbMachines, int timeoutedValue, int maxTimeout);

    std::vector<sequence> removePrefixes(std::vector<sequence> E);

    FSM * completeMutation(FSM * M);

    InfInt computeNumberOfMutants(FSM * M);
};

#endif // FSMALGORITHMS_H
