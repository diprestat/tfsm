#include "distinguishingautomaton.h"
#include <limits>
#include <algorithm>
#include <iostream>
#include <queue>
#include "../tools.h"
using namespace std;

DistinguishingAutomaton::DistinguishingAutomaton(FSM * S, FSM * M)
{
    this->specification = S;
    this->mutationMachine = M;

    ProductState * initialState = new ProductState(S->initialState, M->initialState, 0, 0);
    this->states.insert(make_pair(initialState->getKey(), initialState));
    this->initialState = initialState;
    this->hasNoSinkState = true;
    this->isConnected = true;
    this->generateNext(initialState);
    this->isConnected = this->isProductConnected();
}

void DistinguishingAutomaton::insertState(ProductState * state, string i, ProductState * newState, bool isTimeout, int id)
{
    if (this->states.find(newState->getKey()) == this->states.end()) {
        this->states.insert(make_pair(newState->getKey(), newState));
    }
    this->transitions.push_back(ProductTransition(state->getKey(), i, newState->getKey(), isTimeout, id, false));
}

void DistinguishingAutomaton::generateNext(ProductState * state)
{
    for (auto mutationTransition : this->mutationMachine->lambda(state->mutationState)) {
        IOTransition * related = NULL;
        for (auto specificationTransition : this->specification->getXi(state->specificationState, mutationTransition->i)) {
            related = new IOTransition(specificationTransition->src, specificationTransition->i, specificationTransition->o, specificationTransition->tgt, specificationTransition->id);
        }
        if (related != NULL) {
            ProductState * newState;
            if (related->o == mutationTransition->o)
                newState = new ProductState(related->tgt, mutationTransition->tgt, 0, 0);
            else {
                newState = new ProductSinkState();
                this->hasNoSinkState = false;
            }
            this->insertState(state, related->i, newState, false, mutationTransition->id);
        }
        delete related;
    }

    TimeoutTransition * related = NULL;
    for (auto specificationTimeout : this->specification->delta(state->specificationState)) {
        related = new TimeoutTransition(specificationTimeout->src, specificationTimeout->t, specificationTimeout->tgt, specificationTimeout->id);//&specificationTimeout;
    }
    int spec_t = related->t;
    if (spec_t != inf)
        spec_t -= state->specificationCounter;

    for (auto mutationTimeout : this->mutationMachine->delta(state->mutationState)) {
        int muta_t = mutationTimeout->t;
        if (muta_t != inf)
            muta_t -= state->mutationCounter;
        if (muta_t > 0) {
            ProductState * newState;
            if (muta_t < spec_t && spec_t != inf) {
                newState = new ProductState(state->specificationState, mutationTimeout->tgt, state->specificationCounter + muta_t, 0);
                this->insertState(state, to_string(muta_t), newState, true, mutationTimeout->id);
            }
            else if (muta_t < spec_t && spec_t == inf) {
                newState = new ProductState(state->specificationState, mutationTimeout->tgt, inf, 0);
                this->insertState(state, to_string(muta_t), newState, true, mutationTimeout->id);
            }
            else if (muta_t == spec_t) {
                newState = new ProductState(related->tgt, mutationTimeout->tgt, 0, 0);
                this->insertState(state, to_string(muta_t), newState, true, mutationTimeout->id);
            }
            else if (muta_t > spec_t && muta_t != inf) {
                newState = new ProductState(related->tgt, state->mutationState, 0, state->mutationCounter + spec_t);
                this->insertState(state, to_string(spec_t), newState, true, mutationTimeout->id);
            }
            else if (muta_t > spec_t && muta_t == inf) {
                newState = new ProductState(related->tgt, state->mutationState, 0, inf);
                this->insertState(state, to_string(spec_t), newState, true, mutationTimeout->id);
            }
        }
    }
    delete related;
    state->isGenerated = true;

    vector<ProductTransition> copyTransitions(this->transitions);
    for (auto transition : copyTransitions) {
        if (transition.src == state->getKey()) {
            ProductState * tgt = this->states.find(transition.tgt)->second;
            if (!tgt->isGenerated)
                this->generateNext(tgt);
        }
    }
}

std::vector<path> DistinguishingAutomaton::revealingPaths(sequence alpha)
{
    vector<path> results;
    path currentPath;
    revealingPathsRecursive(this->initialState, currentPath, results, alpha, 0, 0);
    return results;
}

bool DistinguishingAutomaton::isPathDeterministic(const path p)
{
    for (int id : p) {
        if (this->mutationMachine->isIdTimeout(id)) {
            vector<TimeoutTransition *> xiTimeouts = this->mutationMachine->getXi(this->mutationMachine->getTimeoutFromId(id)->src);
            for (auto otherTimeout : xiTimeouts) {
                if (find(p.begin(), p.end(), otherTimeout->id) != p.end() && otherTimeout->id != id)
                    return false;
            }
        }
        else {
            vector<IOTransition *> xiTransitions = this->mutationMachine->getXi(this->mutationMachine->getTransitionFromId(id)->src, this->mutationMachine->getTransitionFromId(id)->i);
            for (auto otherTransition : xiTransitions) {
                if (find(p.begin(), p.end(), otherTransition->id) != p.end() && otherTransition->id != id)
                    return false;
            }
        }
    }
    return true;
}

void DistinguishingAutomaton::revealingPathsRecursive(ProductState * state, path currentPath, vector<path> &results, sequence alpha, int sequenceIndex, int timeBuffer)
{
    if (state->getKey() == "sink") {
        results.push_back(currentPath);
    }
    else if (sequenceIndex < alpha.size()) {
        ts timed_symbol = alpha[sequenceIndex];
        string symbol = timed_symbol.first;
        int symbol_time = timed_symbol.second;
        int t = symbol_time - timeBuffer;
        if (sequenceIndex > 0) {
            t -= alpha[sequenceIndex -1].second;
            symbol_time -= alpha[sequenceIndex -1].second;
        }
        //Time to spend, so take only timeouts
        if (t > 0) {
            for (auto transition : this->transitions) {
                if (transition.src == state->getKey() && transition.isTimeout) {
                    int timeout = atoi(transition.i.c_str());
                    ProductState * tgtNode = this->states.find(transition.tgt)->second;
                    if (timeout <= t) {
                        path newPath(currentPath);
                        newPath.push_back(transition.id);
                        if (this->isPathDeterministic(newPath)) {
                            this->revealingPathsRecursive(tgtNode, newPath, results, alpha, sequenceIndex, timeBuffer + timeout);
                        }
                    }
                    else {
                        for (auto mutaTimeout : this->mutationMachine->delta(state->mutationState)) {
                            if (timeout < mutaTimeout->t) {
                                path newPath(currentPath);
                                newPath.push_back(transition.id);
                                if (this->isPathDeterministic(newPath)) {
                                    this->revealingPathsRecursive(state, newPath, results, alpha, sequenceIndex, symbol_time);
                                }
                            }
                        }
                    }
                }
            }
        }
        else {
            for (auto transition : this->transitions) {
                if (transition.src == state->getKey() && !transition.isTimeout) {
                    if (transition.i == symbol) {
                        ProductState * tgtNode = this->states.find(transition.tgt)->second;
                        path newPath(currentPath);
                        newPath.push_back(transition.id);
                        if (this->isPathDeterministic(newPath))
                            this->revealingPathsRecursive(tgtNode, newPath, results, alpha, sequenceIndex+1, 0);
                    }
                }
            }
        }
    }
}


void DistinguishingAutomaton::print()
{
    cout << "States : {";
    for (auto s : this->states) {
        cout << s.first << endl;
    }
    cout << "}" << endl;
    cout << "Transitions : {";
    for (auto t : this->transitions) {
        cout << "(" << t.src << "," << t.i << "," << t.tgt << ") : " << t.id << "," << endl;
    }
    cout << "}" << endl;
}

string DistinguishingAutomaton::DijkstraFindMin(map<string, int> distances, set<string> Q) {
    int min = inf;
    string minState = "";
    for (auto state : Q) {
        if (distances.find(state)->second <= min) {
            min = distances.find(state)->second;
            minState = state;
        }
    }
    return minState;
}

void DistinguishingAutomaton::DijkstraUpdateDistancesMin(map<string, int> & distances, map<string, ProductTransition> & predecessors, string s1, string s2, ProductTransition transition) {
    if (distances.find(s2)->second > distances.find(s1)->second + 1) {
        if (distances.find(s1)->second != inf)
            distances.find(s2)->second = distances.find(s1)->second + 1;
        predecessors.find(s2)->second = transition;
    }
}

bool DistinguishingAutomaton::isProductConnected()
{
    queue<string> fifo;
    set<string> marked;
    fifo.push(this->initialState->getKey());
    marked.insert(this->initialState->getKey());
    while (!fifo.empty()) {
        string element = fifo.front();
        fifo.pop();
        for (ProductTransition transition : this->transitions) {
            if (!transition.isTimeout) {
                if (transition.src == element && marked.find(transition.tgt) == marked.end()) {
                    fifo.push(transition.tgt);
                    marked.insert(transition.tgt);
                }
            }
            else {
                if (atoi(transition.i.c_str()) != inf) {
                    if (transition.src == element && marked.find(transition.tgt) == marked.end()) {
                        fifo.push(transition.tgt);
                        marked.insert(transition.tgt);
                    }
                }
            }
        }
    }

    return marked.size() == this->states.size();
}

deque<ProductTransition> DistinguishingAutomaton::Dijkstra(string key)
{
    map<string, ProductTransition> predecessors;
    set<string> Q;
    map<string, int> distances;
    for (const auto &s : this->states) {
        string stateKey = s.first;
        Q.insert(stateKey);
        predecessors.insert(make_pair(stateKey, ProductTransition("", "", "", false, -1, false)));
        distances.insert(make_pair(stateKey, inf));
    }
    distances.find(key)->second = 0;

    while (Q.size() > 0) {
        string s1 = DijkstraFindMin(distances, Q);
        Q.erase(Q.find(s1));
        for (auto transition : this->transitions) {
            if (transition.src == s1) {
                if (atoi(transition.i.c_str()) != inf) {
                    DijkstraUpdateDistancesMin(distances, predecessors, s1, transition.tgt, transition);
                }
            }
        }
    }

    string currentStateKey = "sink";
    deque<ProductTransition> results;

    if (distances.find("sink")->second != inf) {
        while (currentStateKey != key) {
            if (distances.find(currentStateKey)->second == inf) {
                results.clear();
                return results;
            }
            /*
            cout << "Res :" << endl;
            cout << currentStateKey << endl;
            cout << predecessors.find(currentStateKey)->second.getKey() << endl;
            */
            results.push_front(predecessors.find(currentStateKey)->second);
            currentStateKey = predecessors.find(currentStateKey)->second.src;
        }
    }
    return results;
}

void DistinguishingAutomaton::reachableStates(ProductState * state, path currentPath, set<string> &results, sequence alpha, int sequenceIndex, int timeBuffer)
{
    if (state->getKey() != "sink") {
        if (sequenceIndex < alpha.size()) {
            ts timed_symbol = alpha[sequenceIndex];
            string symbol = timed_symbol.first;
            int symbol_time = timed_symbol.second;
            int t = symbol_time - timeBuffer;
            if (sequenceIndex > 0) {
                t -= alpha[sequenceIndex -1].second;
                symbol_time -= alpha[sequenceIndex -1].second;
            }
            //Time to spend, so take only timeouts
            if (t > 0) {
                for (auto transition : this->transitions) {
                    if (transition.src == state->getKey() && transition.isTimeout) {
                        int timeout = atoi(transition.i.c_str());
                        ProductState * tgtNode = this->states.find(transition.tgt)->second;
                        if (timeout <= t) {
                            path newPath(currentPath);
                            newPath.push_back(transition.id);
                            if (this->isPathDeterministic(newPath)) {
                                this->reachableStates(tgtNode, newPath, results, alpha, sequenceIndex, timeBuffer + timeout);
                            }
                        }
                        else {
                            for (auto mutaTimeout : this->mutationMachine->delta(state->mutationState)) {
                                if (timeout <= mutaTimeout->t) {
                                    path newPath(currentPath);
                                    newPath.push_back(transition.id);
                                    if (this->isPathDeterministic(newPath)) {
                                        this->reachableStates(state, newPath, results, alpha, sequenceIndex, symbol_time);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else {
                for (auto transition : this->transitions) {
                    if (transition.src == state->getKey() && !transition.isTimeout) {
                        if (transition.i == symbol) {
                            ProductState * tgtNode = this->states.find(transition.tgt)->second;
                            path newPath(currentPath);
                            newPath.push_back(transition.id);
                            if (this->isPathDeterministic(newPath))
                                this->reachableStates(tgtNode, newPath, results, alpha, sequenceIndex+1, 0);
                        }
                    }
                }
            }
        }
        else {
            results.insert(state->getKey());
        }
    }
}

sequence DistinguishingAutomaton::inputSequenceFromAcceptedLanguage(set<string> beginningStates, sequence prefix)
{
    sequence input;
    if (!this->hasNoSinkState && this->isConnected) {
        set<string> results;
        path currentPath;
        reachableStates(this->initialState, currentPath, results, prefix, 0, 0);
        for (string key : results) {
            deque<ProductTransition> res = Dijkstra(key);
            int time = 0;
            if (prefix.size() > 0)
                time = prefix[prefix.size()-1].second;
            for (auto transition : res) {
                if (transition.isTimeout)
                    time += atoi(transition.i.c_str());
                else
                    input.push_back(ts(transition.i, time));
            }
            if (res.size() > 0)
                return input;
        }
    }
    return input;
}