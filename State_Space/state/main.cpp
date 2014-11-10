#include <iostream>
#include "State_Space_Generator.h"

int main (int argc, char * argv[])
{

  SCHED_ALGO algo;

  if (!strcmp (argv[1], "OTH"))
    algo = OTH;
  else if (!strcmp (argv[1], "RM"))
    algo = RM;
  else if (!strcmp (argv[1], "EDF"))
    algo = EDF;
  else if (!strcmp (argv[1], "LLF"))
    algo = LLF;
  else if (!strcmp (argv[1], "SJF"))
    algo = SJF;
  else if (!strcmp (argv[1], "DLJ"))
    algo = DLJ;
  else if (!strcmp (argv[1], "PVAL"))
    algo = PVAL;
  else if (!strcmp (argv[1], "CON"))
    algo = CON;
  else if (!strcmp (argv[1], "MLF"))
    algo = MLF;
  else if (!strcmp (argv[1], "PVALM"))
    algo = PVALM;
  else if (!strcmp (argv[1], "OPT"))
    algo = OPT;

	long long num = atoi (argv[2]);
  
	if (argc != 4)
		State_Space_Gen::start_simulation ();

  for (long long i = 0; i < num;i++)
  {
    State_Space_Gen* gen = new State_Space_Gen (algo);

		if (argc == 4)
			gen->not_matlab ();

    if (algo == PVALM)
    {
      gen->pval_modify ();
      delete gen;

      exit (0);
    }
    
    gen->init_states ();

    gen->generate_states (2);
   
//    gen->print_states ();
    if (algo == CON)
    {
      gen->generate_con_opt_eqn ();
    }
    else
      gen->generate_simulatanoeus_eqn ();

    gen->gen_ddln_miss_eqn ();

    gen->gen_ddln_met_eqn ();

		gen->gen_cpu_util_eqn ();

    if (algo !=CON)
    {
      gen->solve_simul_eqn ();

			if (argc != 4)
			{
				gen->compute_cpu_util ();
      
				gen->compute_ddln_miss ();

				gen->compute_ddln_met ();
			}
    }

		if (argc != 4)
	    gen->print_result ();



    gen->pval_modify ();

    delete gen;
  }

	if (argc != 4)
		State_Space_Gen::stop_simulation ();
}
