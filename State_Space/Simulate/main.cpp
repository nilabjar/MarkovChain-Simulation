
#include "Utility.h"
#include "Simulator.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>

using namespace std;

// Simulator expects the total time it needs to run ... pass it as a command prompt

int main (int argc, char * argv[])
{
	if (argc < 2)
	{
		cout << "Usage : Simulate <time in seconds>" << endl;
		exit (0);
	}
	srand (time (NULL));

	long tot_time = atoi (argv[1]);

	Simulator* sim = Simulator::instance ();

	sim->total_sim_run (tot_time *1000);

	Utility::instance ()->total_runs_ = tot_time*2;
	
	sim->start_simulation ();

	sim->print_results ();
}