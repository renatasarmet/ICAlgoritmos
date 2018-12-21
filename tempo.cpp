#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
using namespace std;

int main(int argc, char *argv[]){
	string auxInputName;
	ifstream inputName("testCases.txt");
	// inputName.open("testCases.txt");
	// fstream timeLog;
	// timeLog.open("timeLogElaborate.txt", std::fstream::in | std::fstream::out | std::fstream::app);
	// double timeSpent;
	// struct timespec start, finish;
	// double minTime = 1000;
	// double maxTime = 0;
	// double avgTime;

	while(!inputName.eof()){
		stringstream stringBuilder;
		inputName >> auxInputName;
		stringBuilder << "./main facilityTestCases/" << auxInputName;
		string completeString = stringBuilder.str();
		cout << endl << auxInputName << endl;
		// timeLog << auxInputName << endl;
		// avgTime = 0;
		// minTime = 1000;
		// maxTime = 0;

		// for(int i = 0; i < 25; i++){
			// clock_gettime(CLOCK_REALTIME, &start);
			int retCode = system(completeString.c_str());
			// clock_gettime(CLOCK_REALTIME, &finish);
			// timeSpent =  (finish.tv_sec - start.tv_sec);
			// timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
			// cout << "Time spent: " << timeSpent << " seconds" << endl;

			// if(timeSpent > maxTime)
			// 	maxTime = timeSpent;
			// else if(timeSpent < minTime)
			// 	minTime = timeSpent;
			// timeLog << i << ": " << timeSpent << endl;
			// avgTime += timeSpent;
		// }
		// timeLog << "Min: " << minTime << endl << "Max:" << maxTime << endl << "Avg:" << avgTime/25 << endl << endl;
	}
	return 0;
}