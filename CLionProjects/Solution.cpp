//
// Created by Renata Sarmet Smiderle Mendes on 5/1/20.
//

#include "Solution.h"

// Alocando memoria para os vetores e inicializando as variaveis
Solution::Solution(const Instance &instance) : instance(instance) {
    final_total_cost = 0;
    time_spent = 0;
    local_optimum = false;
    allocateMemory();
    initializeArrays();
}

Solution::~Solution() {
    delete [] assigned_facilities;
    delete [] open_facilities;
}

void Solution::allocateMemory() {
    assigned_facilities = new int[instance.getQtyClients()];
    open_facilities = new bool[instance.getQtyFacilities()];
}

void Solution::initializeArrays() {
    for(int i=0; i < instance.getQtyClients(); ++i){
        assigned_facilities[i] = -1; // inicialmente nao está conectado a ninguem
    }

    for(int i=0; i < instance.getQtyFacilities(); ++i){
        open_facilities[i] = false; // inicialmente nenhuma está aberta
    }
}

int *Solution::getAssignedFacilities() const {
    return assigned_facilities;
}

void Solution::setAssignedFacilities(int *assignedFacilities) {
    assigned_facilities = assignedFacilities;
}

int Solution::getAssignedFacilityJ(int j) const {
    return assigned_facilities[j];
}

void Solution::setAssignedFacilityJ(int j, int assignedFacility) {
    assigned_facilities[j] = assignedFacility;
}

double Solution::getFinalTotalCost() const {
    return final_total_cost;
}

void Solution::setFinalTotalCost(double finalTotalCost) {
    final_total_cost = finalTotalCost;
}

void Solution::incrementFinalTotalCost(double increment) {
    final_total_cost += increment;
}

double Solution::getTimeSpent() const {
    return time_spent;
}

void Solution::setTimeSpent(double timeSpent) {
    time_spent = timeSpent;
}

bool Solution::isLocalOptimum() const {
    return local_optimum;
}

void Solution::setLocalOptimum(bool localOptimum) {
    local_optimum = localOptimum;
}

bool *Solution::getOpenFacilities() const {
    return open_facilities;
}

void Solution::setOpenFacilities(bool *openFacilities) {
    open_facilities = openFacilities;
}

bool Solution::getOpenFacilityJ(int j) const {
    return open_facilities[j];
}

void Solution::setOpenFacilityJ(int j, bool openFacility) {
    open_facilities[j] = openFacility;
}

const Instance &Solution::getInstance() const {
    return instance;
}

void Solution::setInstance(const Instance &instance) {
    Solution::instance = instance;
}

int Solution::getQtyFacilities() const {
    return instance.getQtyFacilities();
}

int Solution::getQtyClients() const {
    return instance.getQtyClients();
}

double *Solution::getCostF() const {
    return instance.getCostF();
}

double Solution::getCostFJ(int j) const {
    return instance.getCostFJ(j);
}

void Solution::setCostFJ(int j, double costF) {
    instance.setCostFJ(j, costF);
}

double **Solution::getCostA() const {
    return instance.getCostA();
}

double Solution::getCostAIJ(int i, int j) const {
    return instance.getCostAIJ(i,j);
}


