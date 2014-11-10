
#include "Utility.h"
#include "Simulator.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>

using namespace std;

// Simulator expects the total time it needs to run ... pass it as a command prompt
int
parse_args (int argc, char *argv[]);


int main (int argc, char * argv[])
{
	parse_args (argc, argv);

	srand (time (NULL));

	Simulator* sim = Simulator::instance ();

	sim->start_simulation ();

	sim->print_results ();
}

int
parse_args (int argc, char *argv[])
{
		int i = 1;

		while (i < argc)
		{
			char c = argv[i][0];

			switch (c)
			{
				case 't':
					{
					long tot_time = atoi (argv[++i]);
					Simulator::instance ()->total_sim_run (tot_time *1000);
					Utility::instance ()->total_runs_ = tot_time*2;
					break;
					}
				case 'p':
					Utility::instance ()->state_file (argv[++i]);
					break;
				case 'f':
						Utility::instance ()->use_file (true);            
						break;
				case 'e':
						Utility::instance ()->schedule_events (true);
						break;
          case '?':  // Display help for use of the server.
          default:
						cout << "Usage: -t Simulate <time in seconds> \n\
														-p <p_states file> \n\
														-f <use file or not> \n\
														-e schedule only at events \n"
									<< argv[0]
									<< endl;
						exit (0);
            break;
			}

			i++;
		}// while 

  return 0;
}
