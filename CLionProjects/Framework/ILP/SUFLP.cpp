#include "gurobi_c++.h"
#include <string>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <fstream>

using namespace std;

#define verbose 2
#define EPSL 0.001
#define TIME_LIMIT 600 // 10 minutes

double solve(int num_facilities, int num_clients, int num_scenarios,
    vector < double > f, vector < vector < double > > c, vector < double > p,
    vector < vector < bool > > d, vector < vector < double > > fs);

void print_instance(int num_facilities, int num_clients, int num_scenarios,
    vector < double > f, vector < vector < double > > c, vector < double > p,
    vector < vector < bool > > d, vector < vector < double > > fs);

int main(int argc, char * argv[]) {
  //*********************************************
  //READING INSTANCE
  //*********************************************
  int num_facilities;
  int num_clients;
  int num_scenarios;

  cin >> num_facilities;
  cin >> num_clients;

  vector < double > f(num_facilities);
  vector < vector < double > > c(num_clients); //c[client][facility]
  double aux_double;
  double aux_bool;

  for(int i = 0; i < num_facilities; i++){
    cin >> f[i];
  }

  for(int j = 0; j < num_clients; j++){
    c[j].resize(num_facilities);
    for(int i = 0; i < num_facilities; i++){
      cin >> aux_double;
      c[j][i] = aux_double;
    }
  }

  cin >> num_scenarios;

  vector < double > p(num_scenarios);
  vector < vector < bool > > d(num_scenarios);    //d[scenario][client]
  vector < vector < double > > fs(num_scenarios); //fs[scenario][facility]
  for(int s = 0; s < num_scenarios; s++){
    cin >> p[s];
  }
  for(int s = 0; s < num_scenarios; s++){
    d[s].resize(num_clients);
    for(int j = 0; j < num_clients; j++){
      cin >> aux_bool;
      d[s][j] = aux_bool;
    }
  }
  for(int s = 0; s < num_scenarios; s++){
    fs[s].resize(num_facilities);
    for(int i = 0; i < num_facilities; i++){
      cin >> fs[s][i];
    }
  }

  //Call the solver
  if(verbose >= 1) print_instance(num_facilities, num_clients, num_scenarios, f, c, p, d, fs);
  solve(num_facilities, num_clients, num_scenarios, f, c, p, d, fs);
}

void print_instance(int num_facilities, int num_clients, int num_scenarios,
    vector < double > f, vector < vector < double > > c, vector < double > p,
    vector < vector < bool > > d, vector < vector < double > > fs){
  cout << num_facilities << " " << num_clients << endl;
  for(int i = 0; i < num_facilities; i++){
    cout << f[i] << " ";
  }
  cout << endl << num_scenarios << endl;
  for(int s = 0; s < num_scenarios; s++){
    cout << p[s] << " ";
  }
  cout << endl;
  for(int s = 0; s < num_scenarios; s++){
    for(int j = 0; j < num_clients; j++){
      cout << d[s][j] << " ";
    }
    cout << endl;
  }
  for(int s = 0; s < num_scenarios; s++){
    for(int i = 0; i < num_facilities; i++){
      cout << fs[s][i] << " ";
    }
    cout << endl;
  }
  return;
}


double solve(int num_facilities, int num_clients, int num_scenarios,
    vector < double > f,
    vector < vector < double > > c,
    vector < double > p,
    vector < vector < bool > > d,
    vector < vector < double > > fs){
  try{
    GRBEnv* env = new GRBEnv();
    GRBModel model = GRBModel(*env);

    //*********************************************
    //VARIABLES
    //*********************************************
    GRBVar * y = model.addVars(num_facilities, GRB_BINARY);

    vector < GRBVar * > ys(num_scenarios);
    for(int s = 0; s < num_scenarios; s++){
      ys[s] = model.addVars(num_facilities, GRB_BINARY);
    }

    vector < vector < GRBVar * > > x(num_scenarios); //x[scenario][facility][client]
    for(int s = 0; s < num_scenarios; s++){
      x[s].resize(num_facilities);
      for(int i = 0; i < num_facilities; i++){
        x[s][i] = model.addVars(num_clients, GRB_BINARY);
      }
    }

    //*********************************************
    //CONSTRAINTS
    //*********************************************
    for(int s = 0; s < num_scenarios; s++){
      for(int j = 0; j < num_clients; j++){
        if(d[s][j] == true){
          GRBLinExpr sum_x = 0;
          for(int i = 0; i < num_facilities; i++){
            sum_x += x[s][i][j];
          }
          model.addConstr(sum_x == 1);
        }
      }
    }

    for(int i = 0; i < num_facilities; i++){
      for(int s = 0; s < num_scenarios; s++){
        for(int j = 0; j < num_clients; j++){
          model.addConstr(x[s][i][j] <= y[i] + ys[s][i]);
        }
      }
    }

    //*********************************************
    //OBJECTIVE
    //*********************************************
    GRBLinExpr obj = 0;
    for(int i = 0; i < num_facilities; i++){
      obj += f[i] * y[i];
      for(int s = 0; s < num_scenarios; s++){
        obj += fs[s][i] * p[s] * ys[s][i];
        for(int j = 0; j < num_clients; j++){
          obj += c[j][i] * p[s] * x[s][i][j];
        }
      }
    }
    model.setObjective(obj, GRB_MINIMIZE);

    model.update();
    model.write("saida.lp");


    if(verbose >= 1){
      cout << "Number of variables: " << model.get(GRB_IntAttr_NumVars) << endl;
      cout << "Number of initial inequalities: " << model.get(GRB_IntAttr_NumConstrs) << endl;
      cout << "Starting Gurobi" << endl;
    }

    //*********************************************
    //SOLVING
    //*********************************************
    model.set(GRB_DoubleParam_TimeLimit, TIME_LIMIT); 
    model.optimize();

    if(verbose >= 1) {
      cout.precision(2);
      cout << fixed;
      cout << "Solution: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
      cout << "Status: " << model.get(GRB_IntAttr_Status) << endl;
      cout << "Gap: " << model.get(GRB_DoubleAttr_MIPGap) * 100 << "%" << endl;
      cout << "Time gurobi solver: " << model.get(GRB_DoubleAttr_Runtime) << endl;
      cout << "Number of explored nodes: " << (int) model.get(GRB_DoubleAttr_NodeCount) << endl << endl;

      cout << "Facilities first stage: ";
      for(int i = 0; i < num_facilities; i++)
        if(y[i].get(GRB_DoubleAttr_X) >= 1 - EPSL) cout << i << " ";
      cout << endl;

      for(int s = 0; s < num_scenarios; s++){
        cout << "Facilities scenario " << s << ": ";
        for(int i = 0; i < num_facilities; i++)
          if(ys[s][i].get(GRB_DoubleAttr_X) >= 1 - EPSL) cout << i << " ";
        cout << endl;
      }

      cout << "Conection" << endl;
      for(int s = 0; s < num_scenarios; s++){
        cout<< "Scenario " << s << ": ";
        for(int j = 0; j < num_clients; j++){
          if(d[s][j] == false){ cout << "-1 "; continue;}
          for(int i = 0; i < num_facilities; i++){
            if(x[s][i][j].get(GRB_DoubleAttr_X) >= 1 - EPSL){
              cout << i << " ";
            }
          }
        }
        cout << endl;
      }
    }
  }  catch (GRBException e){
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  }
  catch (...){
    cout << "Exception during optimization" << endl;
  }
  return 0;
}


