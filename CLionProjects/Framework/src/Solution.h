#ifndef CLIONPROJECTS_SOLUTION_H
#define CLIONPROJECTS_SOLUTION_H

#include <iostream>
#include "Instance.h"

using namespace std;

class Solution {
private:
    int *assigned_facilities; // size qty_clients
    double final_total_cost;
    double time_spent;
    bool local_optimum;
    bool *open_facilities; // size qty_facilities
    Instance instance;
public:
    Solution();

    Solution(const Instance &instance);

    virtual ~Solution();

    void resetSolution();

    Instance &getInstance();

    void allocateMemory();

    void initializeArrays();

    void setInstance(Instance * instance);

    int *getAssignedFacilities() const;

    void setAssignedFacilities(int *assignedFacilities);

    int getAssignedFacilityJ(int j) const;

    void setAssignedFacilityJ(int j, int assignedFacility);

    double getFinalTotalCost() const;

    void setFinalTotalCost(double finalTotalCost);

    void incrementFinalTotalCost(double increment);

    double getTimeSpent() const;

    void setTimeSpent(double timeSpent);

    bool isLocalOptimum() const;

    void setLocalOptimum(bool localOptimum);

    bool *getOpenFacilities() const;

    void setOpenFacilities(bool *openFacilities);

    bool getOpenFacilityJ(int j) const;

    void setOpenFacilityJ(int j, bool openFacility);

    int getQtyFacilities() const;

    int getQtyClients() const;

    double *getCostF() const;

    double getCostFJ(int j) const;

    void setCostFJ(int j, double costF);

    double **getCostA() const;

    double getCostAIJ(int i, int j) const;

    void readFileInitialSol(string initialSolName);

    void showSolution() const;

    void initializeInstance(Instance * _instance);

    void copySolution(Solution *model);

    int getQtyOpenFacilities();

    void connectAndUpdateFacilities(int ** sortedCijID);

    void connectNearest(int ** sortedCijID);

    void printIndividual();

    void printOpenFacilities();
};


#endif //CLIONPROJECTS_SOLUTION_H
