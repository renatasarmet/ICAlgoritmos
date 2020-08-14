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

    Instance();
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

    void setCostFJ(int j, double cost);

    double **getCostA() const;

    void setCostA(double **costA);

    double getCostAIJ(int i, int j) const;

    void setCostAIJ(int i, int j, double cost);

    const string &getInputName() const;

    void setInputName(const string &inputName);

    const string &getSolutionName() const;

    void setSolutionName(const string &solutionName);

    void readFileInstance(string inputType);

    void readFrameworkInstance(int total_qty_facilities, int total_qty_clients, int qty_cli_scenario,
            int *exist_cli_scenario, double *costF, double **costA);

    void allocateCosts();

    void showInstance() const;

    void copyInstance(const Instance *model);
};


#endif //CLIONPROJECTS_INSTANCE_H
