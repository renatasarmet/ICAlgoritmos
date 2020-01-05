//
// Created by Renata Sarmet Smiderle Mendes on 5/1/20.
//

#ifndef CLIONPROJECTS_SOLUTION_H
#define CLIONPROJECTS_SOLUTION_H

#include <iostream>
#include "Instance.h"

using namespace std;

class Solution {
private:
    int * assigned_facilities; // size qty_clients
    double final_total_cost;
    double time_spent;
    bool local_optimum;
    bool * open_facilities; // size qty_facilities
    Instance instance;
public:
    Solution(const Instance &instance);

    virtual ~Solution();

public:
    const Instance &getInstance() const;

    void allocateMemory();

    void initializeArrays();

    void setInstance(const Instance &instance);

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

};


#endif //CLIONPROJECTS_SOLUTION_H
