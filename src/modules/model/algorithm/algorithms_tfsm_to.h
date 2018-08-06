#ifndef ALGORITHMS_TFSM_TO_H
#define ALGORITHMS_TFSM_TO_H
#include "algorithms.h"

class Algorithms_TFSM_TO : public Algorithms
{
public:
    Algorithms_TFSM_TO();

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

    void checkingExperimentBenchmarks();

    void checkingSequenceBenchmarks();

    std::vector<sequence> removePrefixes(std::vector<sequence> E);
};

#endif // ALGORITHMS_TFSM_TO_H