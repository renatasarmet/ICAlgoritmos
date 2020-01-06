#ifndef CLIONPROJECTS_INSTANCE_H
#define CLIONPROJECTS_INSTANCE_H

#include <iostream>
using namespace std;

class Instance {
private:
    int qty_facilities;
    int qty_clients;
    double * cost_f;
    double ** cost_a;
    string input_name;
    string solution_name;
    bool destroyed;
public:
    void setDestroyed(bool destroyed);

public:
    Instance(const string &inputName, const string &solutionName);
    Instance(int qtyFacilities, int qtyClients, const string &inputName, const string &solutionName);
    virtual ~Instance();

    int getQtyFacilities() const;

    void setQtyFacilities(int qtyFacilities);

    int getQtyClients() const;

    void setQtyClients(int qtyClients);

    double *getCostF() const;

    void setCostF(double *costF);

    double getCostFJ(int j) const;

    void setCostFJ(int j, double costF);

    double **getCostA() const;

    void setCostA(double **costA);

    double getCostAIJ(int i, int j) const;

    const string &getInputName() const;

    void setInputName(const string &inputName);

    const string &getSolutionName() const;

    void setSolutionName(const string &solutionName);

    void readFileInstance(string inputType);

    void allocateCosts();

    void showInstance() const;
};


#endif //CLIONPROJECTS_INSTANCE_H
