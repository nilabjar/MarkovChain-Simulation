#include "State_Space_Generator.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <time.h>


#define  BUFSIZE 256

Engine * State_Space_Gen::ep = 0;

State_Space_Gen::State_Space_Gen (SCHED_ALGO algo)
  : algo_(algo)
{

  if (algo_ == SCHED_ALGO::PVAL)
    this->read_p_values ();

  ifstream task ("tasks.txt");

  {
    int id, arr, arr_rate, svc, svc_rate;
    
    task >> id;
    task >> arr;
    task >> arr_rate;
    task >> svc;
    task >> svc_rate;
    
    Task tmp_task (id, arr, arr*arr_rate, svc, svc*svc_rate);
    task_1 = tmp_task;
  }

  {
    int id, arr, arr_rate, svc, svc_rate;
    
    task >> id;
    task >> arr;
    task >> arr_rate;
    task >> svc;
    task >> svc_rate;
    
    Task tmp_task (id, arr, arr*arr_rate, svc, svc*svc_rate);
    task_2 = tmp_task;
  }

  task.close ();

//  this->start_simulation ();

  expt_num = 0;

  //ifstream state_p ("state_p_vec.txt");

  //int state_id; 
  //float pval;
  //while (!state_p.eof ())
  //{
  //  state_p >> state_id;
  //  state_p >> pval;
  //  this->state_p_vec[state_id] = pval;
  //}

  //state_p.close ();

	ifstream state_idsf ("state_ids.txt");

	int var;
	int state;
	if (state_idsf != 0)
	{
		while (!state_idsf.eof ())
		{
			state_idsf >> var;
			state_idsf >> state;

			this->state_ids[state] = var;
		}
	}

	state_idsf.close ();

	this->no_matlab_ = false;
}

void State_Space_Gen::generate_states(int num_of_tasks)
{
  // initial state 
  cur_state_id = 1;

  State in_state;
  in_state.id = cur_state_id;
  in_state.task_1 = this->task_1;
  in_state.task_2 = this->task_2;

  char buf[10];
  memset (buf, 0 , 10);

  sprintf (buf, "%d%d%d%d", 
            in_state.task_1.arr_.cur_,
            in_state.task_1.svc_.cur_,
            in_state.task_2.arr_.cur_,
            in_state.task_2.svc_.cur_);

  int state_val = atoi (buf);

  in_state.state_name = buf;
	in_state.config = state_val;
	in_state.p_val_ = -1;

  new_states.push_back (in_state);

  while (new_states.size ())
  {
    State* parent = &*(new_states.begin ());

    //calculate the out states ..
    State new_state = *parent;

    Event ev;
    float rate = new_state.task_1.next_arrival_stage (ev);

		//here check a new arrival occurs or not
		if (new_state.task_1.arr_.cur_ == 1)
		{
			if (!(new_state.task_1.svc_.cur_ > 0 && new_state.task_2.svc_.cur_ > 0))
				new_state.cpu_holder_ = new_state.task_1.id_;
			else
			{
				scheduling_event_states (&new_state);
			}
		}
			

    State* next_state_p;

    // there will be a out state for each
    // arrival and deadline for each task ..
    // this will be independant of the scheduling 
    // algorithm ....
    // The states depending on the scheduling 
    // algorithm used will be determined by some othe
    // delegate function

    // task_1 arrival process


    next_state_p = &*(this->find_next_state (new_state));

    State_Type type = State_Type::non_contend;

    create_edge (parent, next_state_p, rate,ev, type, 0);

    // task_2 arrival process
    new_state = *parent;
    rate = new_state.task_2.next_arrival_stage (ev);

		if (new_state.task_2.arr_.cur_ == 1)
		{
			if (!(parent->task_1.svc_.cur_ > 0 && parent->task_2.svc_.cur_ > 0))
				new_state.cpu_holder_ = parent->task_2.id_;
			else
			{
				scheduling_event_states (&new_state);
			}
		}
		
		
		next_state_p = &*(this->find_next_state (new_state));

    create_edge (parent, next_state_p, rate, ev, type, 0);

  // now determine the states depending on service completion ....

//    this->scheduling_states (parent);

		if (new_state.cpu_holder_ != -1)
		{
			if (new_state.cpu_holder_ == 1)
			{
				float rate = new_state.task_1.next_service_stage (ev);

				// check if service ends ...
				if (new_state.task_1.svc_.cur_ = 0)
					new_state.cpu_holder_ = new_state.task_2.id_;

				State* next_state_p = &*(this->find_next_state (new_state));

		    create_edge (parent, next_state_p, rate, ev, type, this->other_var_no);
			}
			else if (new_state.cpu_holder_ == 2)
			{
				float rate = new_state.task_2.next_service_stage (ev);

				// check if service ends ...
				if (new_state.task_2.svc_.cur_ = 0)
					new_state.cpu_holder_ = new_state.task_1.id_;

		    State* next_state_p = &*(this->find_next_state (new_state));

				create_edge (parent, next_state_p, rate, ev, type, this->other_var_no);
			}

		}
    
    old_states.push_back (*parent);
    new_states.pop_front ();
  }
}

int State_Space_Gen::scheduling_event_states (State* parent)
{
		char buf[10];
		memset (buf, 0 , 10);

		sprintf (buf, "%d%d%d%d", 
				    parent->task_1.arr_.cur_,
						parent->task_1.svc_.cur_,
						parent->task_2.arr_.cur_,
						parent->task_2.svc_.cur_);

		int state_val = atoi (buf);

		parent->state_name = buf;

	 // float p_val = this->state_p_vec[state_val];
		parent->config = state_val;

		float p_value;

		this->schedule_task (parent, buf, p_value);
		this->state_p_vec[state_val] = p_value;
		parent->p_val_ = p_value;

		if (p_value == 0)
			parent->cpu_holder_ = parent->task_2.id_;
		else
			parent->cpu_holder_ = parent->task_1.id_;

		return 0;
}

State_List::iterator State_Space_Gen::find_next_state (State& state)
{
  State_List::iterator astate = 
    this->find_a_state (state, this->new_states);

  if (astate != this->new_states.end ())
    return astate;
  else
  {
    astate = find_a_state (state, this->old_states);
    if (astate != this->old_states.end ())
      return astate;
    else
    {
			// here creating a new state ...
			char buf[10];
			memset (buf, 0 , 10);

			sprintf (buf, "%d%d%d%d", 
								state.task_1.arr_.cur_,
								state.task_1.svc_.cur_,
								state.task_2.arr_.cur_,
								state.task_2.svc_.cur_);

			int state_val = atoi (buf);

			state.state_name = buf;
			state.config = state_val;
			state.p_val_ = -1;
//			state.cpu_holder_ = -1;

			// assign the state id .... this needs to be standardised .... 
			if (this->state_ids.size () == 0)
				state.id = ++cur_state_id;
			else
			{
				state.id = this->state_ids[state.config];
			}


      state.in_edges.clear ();
      state.out_edges.clear ();
      this->new_states.push_back (state);
      astate = new_states.end ();
      return (--astate);
    }
  }
}

State_List::iterator State_Space_Gen::find_a_state (State& state, State_List& list)
{
  return (std::find (list.begin (), list.end (), state));
}

int State_Space_Gen::print_states ()
{
  for (State_List::iterator it = old_states.begin ();
        it != old_states.end ();
        it++)
  {
    std::cout << "\nstate number " 
          << it->id << "("
          << it->task_1.arr_.cur_ << ","
          << it->task_1.svc_.cur_ << ","
          << it->task_2.arr_.cur_ << ","
          << it->task_2.svc_.cur_ << ")"
          << endl;
  }

  return 1;
}

int State_Space_Gen::scheduling_states (State* parent)
{
  // form state val
  char buf[10];
  memset (buf, 0 , 10);

  sprintf (buf, "%d%d%d%d", 
            parent->task_1.arr_.cur_,
            parent->task_1.svc_.cur_,
            parent->task_2.arr_.cur_,
            parent->task_2.svc_.cur_);

  int state_val = atoi (buf);

  parent->state_name = buf;

 // float p_val = this->state_p_vec[state_val];
  parent->config = state_val;

  float p_value;
  this->schedule_task (parent, buf, p_value);
  this->state_p_vec[state_val] = p_value;
	parent->p_val_ = p_value;

  State_Type type;
  if (p_value == 0.5)
  {
    type = State_Type::contend;
    this->other_var_no++;

    this->state_p_vec[state_val] = other_var_no;

    this->con_var_nos[other_var_no] = state_val;
  }
  else
    type = State_Type::non_contend;
                            
  // first task 1 .... 
  if ((parent->task_1.svc_.cur_ > 0 && parent->task_2.svc_.cur_ == 0)||
     //(parent->task_1.svc_.cur_ > 0 && parent->task_2.svc_.cur_ > 0 && p_vector[parent->id] > 0))
     (parent->task_1.svc_.cur_ > 0 && parent->task_2.svc_.cur_ > 0 && p_value > 0))
  {
    State new_state = *parent;
    Event ev;
    ev.state = na;

    float rate = new_state.task_1.next_service_stage (ev);

    if (parent->task_2.svc_.cur_ > 0)// if there is contention ...
      rate *= p_value;
		else // there is no contention ... so set p_val 
			parent->p_val_ = 1.0;

    State* next_state_p = &*(this->find_next_state (new_state));

    create_edge (parent, next_state_p, rate, ev, type, this->other_var_no);
  }

  // then task 2
  if ((parent->task_2.svc_.cur_ > 0 && parent->task_1.svc_.cur_ == 0)
        ||
     //(parent->task_1.svc_.cur_ > 0 && parent->task_2.svc_.cur_ > 0 && (1 - p_vector[parent->id]) > 0))
      (parent->task_1.svc_.cur_ > 0 && parent->task_2.svc_.cur_ > 0 && (1 - p_value) > 0))
  {
    State new_state = *parent;
    Event ev;
    ev.state = na;

    float rate = new_state.task_2.next_service_stage (ev);

    if (parent->task_1.svc_.cur_ > 0)
//      rate *= (1 - p_vector[parent->id]);
      rate *= (1 - p_value);
		else // there is no contention ... so set p_val 
			parent->p_val_ = 0.0;

    State* next_state_p = &*(this->find_next_state (new_state));

    create_edge (parent, next_state_p, rate,ev, type,-this->other_var_no);
  }

	if (parent->task_1.svc_.cur_ == 0 && parent->task_2.svc_.cur_ == 0)
		parent->p_val_ = -1;

  return 0;
}

int State_Space_Gen::generate_simulatanoeus_eqn ()
{
	if (this->no_matlab_)
		return 0;

//  ofstream out ("prob_eqn.txt");
  ofstream eqn_out ("eqn.txt");

  double* buffer = new double [old_states.size ()];
  double* col_buffer = new double [old_states.size ()];

  //vector<double> buffer (old_states.size (), 0);
  //vector<double> col_buffer (old_states.size (), 0);


  int matrix_size = this->old_states.size ()*this->old_states.size ();
  this->rate_matrix = new double [matrix_size];
  memset (rate_matrix, 0 , matrix_size);
//  this->rate_matrix.insert (rate_matrix.begin (), matrix_size, 0);
  
  int index = 0;
	
  for (State_List::iterator it = old_states.begin ();
    it != old_states.end ();it++)
  {
    memset (buffer, 0 , old_states.size ()*sizeof(double));
    memset (col_buffer, 0 , old_states.size ()*sizeof(double));
//    vector<double> buffer (old_states.size (), 0);
//    vector<double> col_buffer (old_states.size (), 0);

    double pos_wt = 0;
    
    for (int i = 0;i < it->out_edges.size ();i++)
    {
        pos_wt += it->out_edges[i].rate;
        col_buffer[it->out_edges[i].end - 1] = -it->out_edges[i].rate;
    }
    
    buffer[it->id - 1] = pos_wt;
    col_buffer[it->id - 1] += pos_wt;
    col_buffer[old_states.size () - 1] = 1; //for the normalizing equation ...

    for (int j = 0;j < it->in_edges.size ();j++)
    {
      buffer[it->in_edges[j].end - 1] += -it->in_edges[j].rate;
    }

    for (int i = 0;i < old_states.size (); i++)
    {
//      out << buffer[i] << "\t";
      //if (buffer[i] != 0)
      //  eqn_out << buffer[i] << "p(" << (i+1) << ")\t";
    }

//    copy the buffer on to the rate matrix 
    memcpy (rate_matrix + index,
            col_buffer,
            old_states.size ()*sizeof(double));
    //rate_matrix.insert (index,
    //                    col_buffer.begin (),
    //                    col_buffer.end ());

    index += old_states.size ();


    //eqn_out << endl;
    //out << ";" << endl;
  }

  //for (int i = 0;i < matrix_size;i++)
  //  eqn_out << "\n" << rate_matrix[i] << endl;

  eqn_out.close ();
//  out.close ();

  delete [] buffer;
  delete [] col_buffer;

  return 1;
}

int State_Space_Gen::generate_con_opt_eqn ()
{
//  ofstream out ("prob_eqn.txt");
  ofstream eqn_out ("confun.m");

	eqn_out << "function [c, ceq] = confun(p)" << endl;

	eqn_out << "ceq = [ ";


  double* buffer = new double [old_states.size ()];
  double* col_buffer = new double [old_states.size ()];

  ostringstream str;

  int index = 0;
  for (State_List::iterator it = old_states.begin ();
    it != old_states.end ();it++)
  {
    ostringstream s;
    memset (buffer, 0 , old_states.size ()*sizeof(double));
    memset (col_buffer, 0 , old_states.size ()*sizeof(double));
//    vector<double> buffer (old_states.size (), 0);
//    vector<double> col_buffer (old_states.size (), 0);

    double pos_wt = 0;

    //calculate the positive wt
    for (int i = 0;i < it->out_edges.size ();i++)
    {
      if(it->out_edges[i].type == State_Type::contend)
      {
        int var = it->out_edges[i].variable_no;
        int no_states = this->old_states.size ();

        s << "+ " 
          << it->out_edges[i].rate*2 // the 2 is used to compensate for the ).5 used for p_value ....
          << ((var > 0)?"*p(": "*(1 - p(")
          << ((var > 0)? var : -var) + no_states
          << ((var > 0)?")*":"))*")
          << "p(" << it->id << ") ";

        continue;
      }

      pos_wt += it->out_edges[i].rate;
    }
    
    buffer[it->id - 1] = pos_wt;
    col_buffer[it->id - 1] += pos_wt;
    col_buffer[old_states.size () - 1] = 1;

    for (int j = 0;j < it->in_edges.size ();j++)
    {
      if(it->in_edges[j].type == State_Type::contend)
      {
        int var = it->in_edges[j].variable_no;

        s << "- " << it->in_edges[j].rate*2
          << ((var > 0)?"*p(": "*(1 - p(")
          << ((var > 0)? var : -var) + this->old_states.size ()
          << ((var > 0)?")*":"))*")
          << "p(" << it->in_edges[j].end << ") ";

        continue;
      }

      buffer[it->in_edges[j].end - 1] += -it->in_edges[j].rate;
    }

    for (int i = 0;i < old_states.size (); i++)
    {
//      out << buffer[i] << "\t";
      if (buffer[i] != 0)
      {
          s << ((buffer[i] > 0)? " + ":" - ")
            << ((buffer[i] > 0)? buffer[i] : -buffer[i])
            << "*p(" << (i+1) << ") ";
      }
    }

    index += old_states.size ();

    eqn_out << s.str () << ";" << endl;

//    out << ";" << endl;
    str << "p(" << it->id << ") + ";
  }


  str << " - 1 ];";

  eqn_out << str.str () << endl;

	eqn_out << endl << "c = [];" << endl;

  eqn_out.close ();

  ofstream var_nus ("var_nos.txt");

  for (map<int, int>::iterator it = this->con_var_nos.begin ();
    it != con_var_nos.end ();
    it++)
  {
    var_nus << it->first 
            << "\t"
            << it->second
            << endl;
  }

  var_nus.close ();

  delete [] buffer;
  delete [] col_buffer;

	ofstream out_ub ("ub.txt");

	for (int i = 0;i < old_states.size () + this->other_var_no;i++)
		out_ub << 1 << endl;

	out_ub.close ();

	ofstream out_lb ("lb.txt");

	for (int i = 0;i < old_states.size () + this->other_var_no;i++)
		out_lb << 0 << endl;

	out_lb.close ();

	ofstream state_idf ("state_ids.txt");

	for each (State s in this->old_states)
	{
		state_idf << s.id
							<< "\t"
							<< s.config
							<< endl;
	}

	state_idf.close ();

  return 1;
}

int State_Space_Gen::gen_ddln_miss_eqn ()
{
  ofstream out ("ddln_miss.txt");
  // parse through each state 

  WTS_ARRAY wts_1 (old_states.size (), 0);
  this->ddln_miss.push_back (wts_1);

  WTS_ARRAY wts_2 (old_states.size (), 0);
  this->ddln_miss.push_back (wts_2);

  // ostringstream has memory leak
  ostringstream task1_str;
  ostringstream task2_str;

	int index = 0;
  for (State_List::iterator it = this->old_states.begin ();
        it != this->old_states.end ();it++, index++)
  {
    for (int i = 0; i < it->out_edges.size ();i++)
    {
      if (it->out_edges[i].ev.state == missed)
      {
        stringstream s;
        s << it->out_edges[i].rate 
          << "*p(" 
          //<< it->state_name;
          << it->id
          << ")";

        if (it->out_edges[i].ev.task_id == 1)
        {
          task1_str << " + " << s.str ();
//          ddln_miss[0][it->id - 1] = it->out_edges[i].rate;
          ddln_miss[0][index] = it->out_edges[i].rate;
        }
        else
        {
          task2_str << " + " << s.str ();
//          ddln_miss[1][it->id - 1] = it->out_edges[i].rate;
					ddln_miss[1][index] = it->out_edges[i].rate;
        }
      }
    }
  }
  out << "DM1 = " << task1_str.str () << endl;
  out << "DM2 = " << task2_str.str () << endl;
  out.close ();

	if (this->algo_ == SCHED_ALGO::CON || this->no_matlab_)
	{
		ofstream out_obj ("objfun.m");

		out_obj << "function f = objfun(p)" << endl;
		out_obj << "f = ";
		out_obj << task1_str.str () 
						<< " " 
						<< task2_str.str ()
						<< ";"
						<< endl;

		out_obj.close ();

		ofstream out_1 ("miss_1.m");
		
		out_1 << "function f = miss_1(p)" << endl;
		out_1 << "f = ";
		out_1 << task1_str.str () 
						<< ";"
						<< endl;

		out_1.close ();

		ofstream out_2 ("miss_2.m");

		out_2 << "function f = miss_2(p)" << endl;
		out_2 << "f = ";
		out_2 << task2_str.str () 
						<< ";"
						<< endl;

		out_2.close ();
	}

  return 1;
}

int State_Space_Gen::create_edge (State* parent,
                                  State* next_state, 
                                  float rate, 
                                  Event ev,
                                  State_Type type,
                                  int var_no)
{
  //if (parent->id == next_state->id)
  //  return 1;

  Edge a_edge;
  a_edge.rate = rate;
  a_edge.end = parent->id;
  a_edge.ev = ev;
  a_edge.type = type;
  a_edge.variable_no = var_no;

  next_state->in_edges.push_back (a_edge);

  a_edge.end = next_state->id;
  parent->out_edges.push_back (a_edge);

  return 1;
}

int State_Space_Gen::solve_simul_eqn ()
{

  //initialize the last row of the rate matrix
  long index = (old_states.size () - 1)*old_states.size ();
  long total_size = (old_states.size ())*old_states.size ();

  //for (int i = index;i < total_size;i++)
  //  rate_matrix[i] = 1;  //initialize the b-vecotr

  b_vector = new double[old_states.size ()];

  for (int i = 0;i < old_states.size ();i++)
    b_vector[i] = 0;
  b_vector[old_states.size ()-1] = 1;

	if (no_matlab_)
	{
		this->generate_sparse_map ();
		ofstream out ("b_vector.txt");

		for (int i = 0;i < old_states.size ();i++)
			out << b_vector[i] << endl;

		out.close ();

		//create_sparse_matrice ();

		this->create_sparse_matrice_from_map ();

		return 1;
	}


  // start matlab ..

 // if (!(ep = engOpen("\0"))) {
	//	cout << "\nCan't start MATLAB engine\n";
	//	return -1;
	//}

//   create a mxArray 
  mxArray * rates = mxCreateDoubleMatrix (this->old_states.size (),
                                          this->old_states.size (), 
                                          mxREAL);

	memcpy((void *)mxGetPr(rates), (void *)rate_matrix, total_size*sizeof(double));

  mxArray * b_array = mxCreateDoubleMatrix(this->old_states.size (),
                                           1, 
                                           mxREAL);

  memcpy((void *)mxGetPr(b_array), (void *)b_vector, old_states.size ()*sizeof (double));

  engEvalString (ep, "clear");

  engPutVariable(ep, "A", rates);
  engPutVariable(ep, "b", b_array);

  char buffer[BUFSIZE+1];
  buffer[BUFSIZE] = '\0';
	engOutputBuffer(ep, buffer, BUFSIZE);

	engEvalString(ep, "P = A\\b");
//	engEvalString(ep, "P = a\\b");

//  cout << buffer+2 << endl;

  // get the result 
  mxArray* result;
  if ((result = engGetVariable (ep, "P")) == NULL)
    cout << "No P value found" << endl;
  {
    state_probs = new double[old_states.size ()];
    memset (state_probs, 0 , old_states.size ()*sizeof(double));
    double* probs = mxGetPr (result);

    memcpy (state_probs, probs, old_states.size ()* sizeof(double));

    ofstream out ("prob.txt");

    int i = 0;
    for (State_List::iterator it = old_states.begin ();
          it != old_states.end ();
          it++, i++)
    {
//      cout << "Prob " << it->config << " is " << state_probs[i] << endl;
      out << it->id 
          << "\t" 
          <<it->config 
          << "\t" 
          << state_probs[i] 
          << endl;
    }

    delete [] rate_matrix;
    delete [] b_vector;
    out.close ();
  }

  mxDestroyArray (rates);
  mxDestroyArray (b_array);
  mxDestroyArray (result);

//  engClose(ep);
  return 0;
}

int State_Space_Gen::compute_ddln_miss ()
{
  ddln_miss_1 = 0;

  for (int i = 0;i < this->ddln_miss[0].size ();i++)
  {
    ddln_miss_1 += ddln_miss[0][i]*state_probs[i];
  }

  ddln_miss_2 = 0;

  for (int i = 0;i < this->ddln_miss[0].size ();i++)
  {
    ddln_miss_2 += ddln_miss[1][i]*state_probs[i];
  }

  ofstream out ("ddln_miss_val.csv", ios::app);

//  out << "DM1 = " << ddln_miss_1 << endl;
//  out << "DM2 = " << ddln_miss_2 << endl;
  out << this->expt_num 
    << "," 
    << ddln_miss_1 
    << "," 
    << ddln_miss_2 
    << "," 
    << ddln_miss_1 + ddln_miss_2;
//    << endl;

//  cout << "DM1 = " << ddln_miss_1 << endl;
//  cout << "DM2 = " << ddln_miss_2 << endl;

  out.close ();

//  delete [] state_probs;

  return 1;
}


int State_Space_Gen::start_simulation ()
{
   if (!(ep = engOpen("\0"))) {
		cout << "\nCan't start MATLAB engine\n";
		return -1;
	}

	 //create sparse matrice
	 engEvalString (ep, "i = [];");
	 engEvalString (ep, "j = [];");
	 engEvalString (ep, "s = [];");
	 engEvalString (ep, "m = 100;");
	 engEvalString (ep, "n = 100;");
	 engEvalString (ep, "a = sparse ( i, j, s, m, n );");
  
  return 0;
}

int State_Space_Gen::stop_simulation ()
{
  engClose(ep);
  return 0;
}

int State_Space_Gen::print_result ()
{
	// commneting printing of state probabilities ....

  ofstream vec ("state_p_vec.txt", ios::app);

  vec << "\n\n NEW P VECTOR" << endl;
  vec << "EXPT NUM " << this->expt_num << endl;

  vec << "Deadline Misses are " << endl;

  vec << "DM1 = " << ddln_miss_1 << endl;
  vec << "DM2 = " << ddln_miss_2 << endl;

  for (map<int, float>::iterator it = this->state_p_vec.begin ();
    it != this->state_p_vec.end ();it++)
  {
    vec << it->first << "\t" << it->second << endl;
  }

  vec.close ();

	if (this->con_var_nos.size () == 0)
	{
		ifstream var_nos ("var_nos.txt");

		int var;
		int state;

		if (var_nos != 0)
		{
			while (!var_nos.eof ())
			{
				var_nos >> var;
				var_nos >> state;

				this->con_var_nos[var] = state;
			}
		}

		var_nos.close ();
	}

	ofstream out_p0 ("p0.txt");


	int i = 0;

	

	// the starting set of probabilities ....
  for (State_List::iterator it = old_states.begin ();
          it != old_states.end ();
          it++, i++)
    {
    //  out_p0 << state_probs[i] 
//			out_p0 << state_probs[it->id] 
//          << endl;

			it->prob_ = state_probs[i];
    }

	old_states.sort ();

	for each (State s in old_states)
	{
		out_p0 << s.prob_ << endl;
	}

	// now output the p_vector values ....
  for (map<int, int>::iterator it = this->con_var_nos.begin ();
    it != this->con_var_nos.end ();
    it++)
  {
    out_p0 << this->state_p_vec[it->second] << endl;
  }

	out_p0.close ();

	// writing down the deadline misses and met and percentages ...

	ofstream out ("system_data.csv", ios::app);

	std::string algo;

	switch (this->algo_)
	{
	case SCHED_ALGO::RM:
		algo = "RM";
		break;
	case SCHED_ALGO::EDF:
		algo = "EDF";
		break;
	case SCHED_ALGO::LLF:
		algo = "LLF";
		break;
	case SCHED_ALGO::SJF:
		algo = "SJF";
		break;
	case SCHED_ALGO::MLF:
		algo = "MLF";
		break;
	case SCHED_ALGO::OPT:
		algo = "OPT";
		break;
	}

	out << this->task_1.svc_.size_ << ","
			<< algo << ","
			<< this->ddln_miss_1 << ","
			<< this->ddln_met_1 << ","
			<< ddln_miss_2 << ","
			<< ddln_met_2  << ","
			<< ddln_miss_1 + ddln_miss_2  << ","
			<< ddln_met_1 + ddln_met_2 << ","
			<< (ddln_miss_1 + ddln_miss_2)/(ddln_met_1 + ddln_met_2) << ","
			<< cpu_util_ << "," 
			<< (ddln_miss_1 + ddln_miss_2)/(ddln_miss_1 + ddln_miss_2 + ddln_met_1 + ddln_met_2)
			<< endl;

	out.close ();

	return 0;
}

void State_Space_Gen::generate_states_names ()
{
  ofstream out ("state_names.txt");

  out.close ();
}

void State_Space_Gen::schedule_task (State* parent, const char * state, float& p_value)
{
  p_value = 0;

  // using random value of p_value .... 
  if (!(parent->task_1.svc_.cur_ > 0 && parent->task_2.svc_.cur_ > 0))
    return; // will not be used, no need to compute ...
    
  switch (this->algo_)
  {
  case SCHED_ALGO::LLF:
    return LLF (parent, state, p_value);
  case SCHED_ALGO::EDF:
    return EDF (parent, state, p_value);
  case SCHED_ALGO::RM:
    return RM (parent, state, p_value);
  case SCHED_ALGO::SJF:
    return SJF (parent, state, p_value);
  case SCHED_ALGO::DLJ:
    return DLJ (parent, state, p_value);
  case SCHED_ALGO::RND:
    return RND (parent, state, p_value);
  case SCHED_ALGO::MLF:
    return MLF (parent, state, p_value);
  case SCHED_ALGO::OPT:
    return OPT (parent, state, p_value);
	case SCHED_ALGO::CON: // for constrained optimization 
    {
      p_value = 0.5;
      return;
    }
  case SCHED_ALGO::PVAL:
    return PVAL (parent, state, p_value);
  default:
    {
      // code for default
      if (!strcmp(state, "2312"))
        p_value = 1;
      else
        p_value = 0;
    }
  };

  return;
}

void State_Space_Gen::LLF (State* parent, const char * state, float& p_value)
{
  // LLF 
  float task_1_arr = 60*(parent->task_1.arr_.size_ - parent->task_1.arr_.cur_ + 1)
                      /parent->task_1.arr_.rate_;

  float task_1_svc = 60*(parent->task_1.svc_.size_ - parent->task_1.svc_.cur_ + 1)
                      /parent->task_1.svc_.rate_;

  float task_1_lax = task_1_arr - task_1_svc;

  float task_2_arr = 60*(parent->task_2.arr_.size_ - parent->task_2.arr_.cur_ + 1)
                      /parent->task_2.arr_.rate_;

  float task_2_svc = 60*(parent->task_2.svc_.size_ - parent->task_2.svc_.cur_ + 1)
                      /parent->task_2.svc_.rate_;

  float task_2_lax = task_2_arr - task_2_svc;

  p_value = (task_1_lax <= task_2_lax);
  return;
}

void State_Space_Gen::MLF (State* parent, const char * state, float& p_value)
{
  // LLF 
  float task_1_arr = 60*(parent->task_1.arr_.size_ - parent->task_1.arr_.cur_ + 1)
                      /parent->task_1.arr_.rate_;

  float task_1_svc = 60*(parent->task_1.svc_.size_ - parent->task_1.svc_.cur_ + 1)
                      /parent->task_1.svc_.rate_;

  float task_1_lax = task_1_arr - task_1_svc;

  float task_2_arr = 60*(parent->task_2.arr_.size_ - parent->task_2.arr_.cur_ + 1)
                      /parent->task_2.arr_.rate_;

  float task_2_svc = 60*(parent->task_2.svc_.size_ - parent->task_2.svc_.cur_ + 1)
                      /parent->task_2.svc_.rate_;

  float task_2_lax = task_2_arr - task_2_svc;

  p_value = (task_1_lax >= task_2_lax);
  return;
}

// best for 9 stages ....
//void State_Space_Gen::OPT(State_List::iterator parent, const char * state, float& p_value)
//{
//	bool t1_death = false;
//	bool t2_death = false;
//
//	//// suicide .............begin 
//	if (parent->task_1.arr_.cur_ > parent->task_1.svc_.cur_)
//		if (parent->task_1.arr_.cur_ > 4) // 8 for 83%
//			if (parent->task_1.arr_.cur_ - parent->task_1.svc_.cur_ > 4) // 3 for 83%
//				t1_death = true;
//
//	if (parent->task_2.arr_.cur_ > parent->task_2.svc_.cur_)
//		if (parent->task_2.arr_.cur_ > 4) // 8 for 83%
//			if (parent->task_2.arr_.cur_ - parent->task_2.svc_.cur_ > 3)
//				t2_death = true;
//
//	if (t1_death && !t2_death)
//	{
//		p_value = 0;
//		return;
//	}
//
//	if (!t1_death && t2_death)
//	{
//		p_value = 1;
//		return;
//	}
//	//// suicide .............ends ....
//
//	/////      Allow other to catch up ///////////
//	bool allow_t1 = false;
//	bool allow_t2 = false;
//
//	if (parent->task_1.svc_.cur_ > parent->task_1.arr_.cur_)
////		if (parent->task_1.svc_.cur_ > 8)
//			if (parent->task_1.svc_.cur_ - parent->task_1.arr_.cur_ > 4) // 5 for 83%
//				allow_t2 = true;
//
//	if (parent->task_2.svc_.cur_ > parent->task_2.arr_.cur_)
////		if (parent->task_2.svc_.cur_ > 8)
//			if (parent->task_2.svc_.cur_ - parent->task_2.arr_.cur_ > 5)
//				allow_t1 = true;
//
//	if (allow_t1 && !allow_t2)
//	{
//		p_value = 1;
//		return;
//	}
//
//	if (!allow_t1 && allow_t2)
//	{
//		p_value = 0;
//		return;
//	}
//	/////      Allow other to catch up ///////////
//
//	////		.. finish the current close service
//	bool finish_t1 = false;
//	bool finish_t2 = false;
//
//	if (parent->task_1.svc_.cur_ > parent->task_1.arr_.cur_)
//		if (parent->task_1.svc_.cur_ > 8) // 6 for 83%
//			if (parent->task_1.svc_.cur_ - parent->task_1.arr_.cur_ < 2) //...dont remember .. could be 3 for 83% .. check it out 
//				finish_t1 = true;
//
//	if (parent->task_2.svc_.cur_ > parent->task_2.arr_.cur_)
//		if (parent->task_2.svc_.cur_ > 8)
//			if (parent->task_2.svc_.cur_ - parent->task_2.arr_.cur_ < 2)
//				finish_t2 = true;
//
//	if (finish_t1 && !finish_t2)
//	{
//		p_value = 1;
//		return;
//	}
//
//	if (!finish_t1 && finish_t2)
//	{
//		p_value = 0;
//		return;
//	}
//
//	return this->EDF (parent, state, p_value);
////		return this->LLF (parent, state, p_value);
//}

 // for 5 stages ....
//void State_Space_Gen::OPT(State_List::iterator parent, const char * state, float& p_value)
//{
//	bool t1_death = false;
//	bool t2_death = false;
//
//	//// suicide .............begin 
//	if (parent->task_1.arr_.cur_ > parent->task_1.svc_.cur_)
//		if (parent->task_1.arr_.cur_ > 3) // 8 for 83%
//			if (parent->task_1.arr_.cur_ - parent->task_1.svc_.cur_ > 2) // 3 for 83%
//				t1_death = true;
//
//	if (parent->task_2.arr_.cur_ > parent->task_2.svc_.cur_)
//		if (parent->task_2.arr_.cur_ > 3) // 8 for 83%
//			if (parent->task_2.arr_.cur_ - parent->task_2.svc_.cur_ > 2)
//				t2_death = true;
//
//	if (t1_death && !t2_death)
//	{
//		p_value = 0;
//		return;
//	}
//
//	if (!t1_death && t2_death)
//	{
//		p_value = 1;
//		return;
//	}
//	//// suicide .............ends ....
//
//	// This condition is not working for 5 stages ...
//
//	/////      Allow other to catch up ///////////
//	//bool allow_t1 = false;
//	//bool allow_t2 = false;
//
//	//if (parent->task_1.svc_.cur_ > parent->task_1.arr_.cur_)
//	//	if (parent->task_1.svc_.cur_ > 3)
//	//		if (parent->task_1.svc_.cur_ - parent->task_1.arr_.cur_ > 4) // 5 for 83%
//	//			allow_t2 = true;
//
//	//if (parent->task_2.svc_.cur_ > parent->task_2.arr_.cur_)
//	//	if (parent->task_2.svc_.cur_ > 3)
//	//		if (parent->task_2.svc_.cur_ - parent->task_2.arr_.cur_ > 4)
//	//			allow_t1 = true;
//
//	//if (allow_t1 && !allow_t2)
//	//{
//	//	p_value = 1;
//	//	return;
//	//}
//
//	//if (!allow_t1 && allow_t2)
//	//{
//	//	p_value = 0;
//	//	return;
//	//}
//	/////      Allow other to catch up ///////////
////
////	////		.. finish the current close service
//	bool finish_t1 = false;
//	bool finish_t2 = false;
//
//	if (parent->task_1.svc_.cur_ > parent->task_1.arr_.cur_)
//		if (parent->task_1.svc_.cur_ > 3) // 6 for 83%
//			if (parent->task_1.svc_.cur_ - parent->task_1.arr_.cur_ < 4) //...dont remember .. could be 3 for 83% .. check it out 
//				finish_t1 = true;
//
//	if (parent->task_2.svc_.cur_ > parent->task_2.arr_.cur_)
//		if (parent->task_2.svc_.cur_ > 3)
//			if (parent->task_2.svc_.cur_ - parent->task_2.arr_.cur_ < 2)
//				finish_t2 = true;
//
//	if (finish_t1 && !finish_t2)
//	{
//		p_value = 1;
//		return;
//	}
//
//	if (!finish_t1 && finish_t2)
//	{
//		p_value = 0;
//		return;
//	}
//
//	return this->EDF (parent, state, p_value);
////		return this->LLF (parent, state, p_value);
//}

 // for n stages .... trying a generic thing, only suicide ... and testing it with 
/// simulation ..
void State_Space_Gen::OPT(State* parent, const char * state, float& p_value)
{
	bool t1_death = false;
	bool t2_death = false;

	int suicide_arr_1 = parent->task_1.arr_.size_/2 + 1;
	int suicide_arr_2 = parent->task_2.arr_.size_/2 + 1;
	int safe_zone = parent->task_2.svc_.size_/2;

	//// suicide .............begin 
	if (parent->task_1.arr_.cur_ > parent->task_1.svc_.cur_)
		if (parent->task_1.arr_.cur_ > suicide_arr_1) // 8 for 83%
			if (parent->task_1.arr_.cur_ - parent->task_1.svc_.cur_ > safe_zone) // 3 for 83%
				t1_death = true;

	if (parent->task_2.arr_.cur_ > parent->task_2.svc_.cur_)
		if (parent->task_2.arr_.cur_ > suicide_arr_2) // 8 for 83%
			if (parent->task_2.arr_.cur_ - parent->task_2.svc_.cur_ > safe_zone)
				t2_death = true;

	if (t1_death && !t2_death)
	{
		p_value = 0;
		return;
	}

	if (!t1_death && t2_death)
	{
		p_value = 1;
		return;
	}
	//// suicide .............ends ....

	// This condition is not working for 5 stages ...

	/////      Allow other to catch up ///////////
	//bool allow_t1 = false;
	//bool allow_t2 = false;

	//if (parent->task_1.svc_.cur_ > parent->task_1.arr_.cur_)
	//	if (parent->task_1.svc_.cur_ > 3)
	//		if (parent->task_1.svc_.cur_ - parent->task_1.arr_.cur_ > 4) // 5 for 83%
	//			allow_t2 = true;

	//if (parent->task_2.svc_.cur_ > parent->task_2.arr_.cur_)
	//	if (parent->task_2.svc_.cur_ > 3)
	//		if (parent->task_2.svc_.cur_ - parent->task_2.arr_.cur_ > 4)
	//			allow_t1 = true;

	//if (allow_t1 && !allow_t2)
	//{
	//	p_value = 1;
	//	return;
	//}

	//if (!allow_t1 && allow_t2)
	//{
	//	p_value = 0;
	//	return;
	//}
	/////      Allow other to catch up ///////////
//
//	////		.. finish the current close service
	//bool finish_t1 = false;
	//bool finish_t2 = false;

	//if (parent->task_1.svc_.cur_ > parent->task_1.arr_.cur_)
	//	if (parent->task_1.svc_.cur_ > 3) // 6 for 83%
	//		if (parent->task_1.svc_.cur_ - parent->task_1.arr_.cur_ < 4) //...dont remember .. could be 3 for 83% .. check it out 
	//			finish_t1 = true;

	//if (parent->task_2.svc_.cur_ > parent->task_2.arr_.cur_)
	//	if (parent->task_2.svc_.cur_ > 3)
	//		if (parent->task_2.svc_.cur_ - parent->task_2.arr_.cur_ < 2)
	//			finish_t2 = true;

	//if (finish_t1 && !finish_t2)
	//{
	//	p_value = 1;
	//	return;
	//}

	//if (!finish_t1 && finish_t2)
	//{
	//	p_value = 0;
	//	return;
	//}

	return this->EDF (parent, state, p_value);
//		return this->LLF (parent, state, p_value);
}

void State_Space_Gen::SJF (State* parent, const char * state, float& p_value)
{
    /// SJF .....
  //check the state .. to be potentially deadline miss
  //if (state[0] - '0' == parent->task_1.arr_.size_ ||
  //  state[2] - '0' == parent->task_2.arr_.size_)
  //  p_value = (parent->task_1.svc_.rate_ > parent->task_2.svc_.rate_);

  p_value = (parent->task_1.svc_.rate_ > parent->task_2.svc_.rate_);
  return;
}

void State_Space_Gen::DLJ (State* parent, const char * state, float& p_value)
{
    /// SJF .....
  //check the state .. to be potentially deadline miss
  if (state[0] - '0' == parent->task_1.arr_.size_ ||
    state[2] - '0' == parent->task_2.arr_.size_)
    p_value = (parent->task_2.svc_.rate_ > parent->task_1.svc_.rate_);
  else
    p_value = (parent->task_1.svc_.rate_ > parent->task_2.svc_.rate_);

  return;
}

void State_Space_Gen::RM (State* parent, const char * state, float& p_value)
{
  // LLF 
  p_value = 0;
  return;
}

void State_Space_Gen::EDF (State* parent, const char * state, float& p_value)
{
  // LLF 
  float task_1_arr = 60*(parent->task_1.arr_.size_ - parent->task_1.arr_.cur_ + 1)
                      /parent->task_1.arr_.rate_;

  float task_2_arr = 60*(parent->task_2.arr_.size_ - parent->task_2.arr_.cur_ + 1)
                      /parent->task_2.arr_.rate_;

  p_value = (task_1_arr <= task_2_arr);
  return;
}

void State_Space_Gen::RND (State* parent, const char * state, float& p_value)
{
  // LLF 

  p_value = rand ()/((double)RAND_MAX + 1);

  return;
}


State_Space_Gen::~State_Space_Gen ()
{
  //this->stop_simulation ();

  if (this->algo_ == SCHED_ALGO::CON)
    delete [] state_probs;
}

void State_Space_Gen::init_states ()
{
  expt_num++;
  if (this->ddln_miss.size ())
  {
    this->ddln_miss[0].clear ();
    this->ddln_miss[1].clear ();
  }
  
  this->ddln_miss.clear ();

  if (this->ddln_met.size ())
  {
    this->ddln_met[0].clear ();
    this->ddln_met[1].clear ();
  }
  
  this->ddln_met.clear ();


  this->new_states.clear ();
  this->old_states.clear ();

  time_t now = time (NULL);
  now += expt_num;
	srand (now);	

  other_var_no = 0;
}

void State_Space_Gen::gen_ddln_met_eqn ()
{
  // parse through each state 

  WTS_ARRAY wts_1 (old_states.size (), 0);
  this->ddln_met.push_back (wts_1);

  WTS_ARRAY wts_2 (old_states.size (), 0);
  this->ddln_met.push_back (wts_2);

	int index = 0;
  for (State_List::iterator it = this->old_states.begin ();
        it != this->old_states.end ();it++, index++)
  {
    for (int i = 0; i < it->out_edges.size ();i++)
    {
      if (it->out_edges[i].ev.state == met)
      {
        if (it->out_edges[i].ev.task_id == 1)
        {
          //ddln_met[0][it->id - 1] = it->out_edges[i].rate;
					ddln_met[0][index] = it->out_edges[i].rate;
        }
        else
        {
          //ddln_met[1][it->id - 1] = it->out_edges[i].rate;
					ddln_met[1][index] = it->out_edges[i].rate;
        }
      }
    }
  }

//  return 1;
}

void State_Space_Gen::compute_ddln_met ()
{
  ddln_met_1 = 0;

  for (int i = 0;i < this->ddln_miss[0].size ();i++)
  {
    ddln_met_1 += ddln_met[0][i]*state_probs[i];
  }

  ddln_met_2 = 0;

  for (int i = 0;i < this->ddln_miss[0].size ();i++)
  {
    ddln_met_2 += ddln_met[1][i]*state_probs[i];
  }

  ofstream out ("ddln_miss_val.csv", ios::app);

//  out << "DM1 = " << ddln_miss_1 << endl;
//  out << "DM2 = " << ddln_miss_2 << endl;
  out //<< this->expt_num 
    << "," 
    << ddln_met_1 
    << "," 
    << ddln_met_2 
    << "," 
    << ddln_met_1 + ddln_met_2
    << "," 
    << (ddln_miss_1+ddln_miss_2)/(ddln_met_1 + ddln_met_2)
    << endl;

//  cout << "DM1 = " << ddln_miss_1 << endl;
//  cout << "DM2 = " << ddln_miss_2 << endl;

  out.close ();

//  delete [] state_probs;

//  return 1;  
}

void State_Space_Gen::read_p_values ()
{
  ifstream in;
  in.open ("p_vector.txt");

  float val;
  char buf[20];
  while (!in.eof ())
  {
    memset (buf, 0 , 20);
    in >> buf;
    in >> val;

    int state = atoi(buf);
//    p_vector.push_back(val);
    this->state_p_vec[state] = val;
  }

  in.close ();

  pval_index = 0;
}

void State_Space_Gen::PVAL (State* parent, const char * state, float& p_value)
{
  //p_value = p_vector[pval_index++];

  p_value = this->state_p_vec[parent->config];
}

void State_Space_Gen::pval_modify ()
{
//  this->read_p_values ();

  ifstream var_nos ("var_nos.txt");

  int var;
  int state;

	if (var_nos != 0)
	{
		while (!var_nos.eof ())
		{
			var_nos >> var;
			var_nos >> state;

			this->con_var_nos[var] = state;
		}

		ofstream out ("var_values.txt");

		for (map<int, int>::iterator it = this->con_var_nos.begin ();
			it != this->con_var_nos.end ();
			it++)
		{
			out << this->state_p_vec[it->second] << endl;
		}

	  out.close ();
	}

	var_nos.close ();
}

void State_Space_Gen::gen_cpu_util_eqn ()
{
  ofstream out ("cpu_util.txt");
  // parse through each state 

	for each (State s in old_states)
	{
		if (s.task_1.svc_.cur_ == 0 && s.task_2.svc_.cur_ == 0)
		{
			idle_state_ids_.push_back (s.id - 1); // state has idle processor

			out << (s.id - 1) << endl;
		}
	}
	
  out.close ();
}

void State_Space_Gen::compute_cpu_util ()
{

	float cpu_util = 1.0;

	for each (int i in idle_state_ids_)
	{
		cpu_util -= state_probs[i];
	}

	int index = 0;
	this->cpu_util_1_ = 0;
	this->cpu_util_2_ = 0;

	for each (State s in old_states)
	{
		if (s.task_1.svc_.cur_ != 0 && s.task_2.svc_.cur_ != 0)
		{
			this->cpu_util_1_ += s.p_val_*state_probs[index];
			this->cpu_util_2_ += (1 - s.p_val_)*state_probs[index];
		}
		if (s.task_1.svc_.cur_ != 0 && s.task_2.svc_.cur_ == 0)
		{
			this->cpu_util_1_ += state_probs[index];
		}
		if (s.task_1.svc_.cur_ == 0 && s.task_2.svc_.cur_ != 0)
		{
			this->cpu_util_2_ += state_probs[index];
		}
		index++;
	}

	ofstream out ("cpu_util.txt");
	out << "cpu util is " << cpu_util << endl;
	
	out << "cpu util for task 1 is " << this->cpu_util_1_ << endl;
	out << "cpu util for task 2 is " << this->cpu_util_2_ << endl;

	out.close ();

	cpu_util_ = cpu_util;

	ofstream ddln_out ("ddln_miss_val.csv", ios::app);

	ddln_out << "," << cpu_util;

	ddln_out.close ();
}

void State_Space_Gen::create_sparse_matrice ()
{
	// go through the rate matrix, find the non-zero elements and 
	// print them to the files ...

	//variables which will be output 
	stringstream i_str;
	stringstream j_str;
	stringstream s_str;

	i_str << "i = [ ";
	j_str << "j = [ ";
	s_str << "s = [ ";

	int total_size = old_states.size () * old_states.size ();

	int row_num = 1;
	int column_num = 1;

	bool first = true;

	for (int i = 0;i < total_size;i++)
	{
		if (rate_matrix[i] != 0)
		{
			if (first == true)
			{
				i_str << row_num;
				j_str << column_num;
				s_str << rate_matrix[i];
				first = false;
			}
			else
			{
				i_str << ", " << row_num;
				j_str << ", " << column_num;
				s_str << ", " << rate_matrix[i];
			}
		}

		row_num++;
		
		if (( (i+1) % old_states.size ()) == 0)
		{
			column_num++;
			row_num = 1;
		}
	}

	i_str << " ];";
	j_str << " ];";
	s_str << " ];";

	ofstream sp ("sparse_a.m");

	sp << i_str.str () << endl
			<< j_str.str () << endl
			<< s_str.str () << endl
			<< "m = " << old_states.size () << endl
			<< "n = " << old_states.size () << endl
			<< "a = sparse ( i, j, s, m, n);" << endl;

	sp.close ();
}

void State_Space_Gen::generate_sparse_map ()
{
	int r_index = 1;

	ofstream states ("p_states.txt");
	
  for (State_List::iterator it = old_states.begin ();
    it != old_states.end ();it++)
  {
		SP_ROW	row;

    double pos_wt = 0;
    
    for (int i = 0;i < it->out_edges.size ();i++)
    {
        pos_wt += it->out_edges[i].rate;
    }

		row[r_index] = pos_wt;

    for (int j = 0;j < it->in_edges.size ();j++)
    {
			row[it->in_edges[j].end] += -it->in_edges[j].rate;
    }

		this->sparse_map_[r_index] = row;

		states 
			<< it->config 
			<< "\t"
			<< it->p_val_
			<< endl;

		r_index++;
  }

	states.close ();

	// make the last row all 1's for the normalizing equation ...
	
	// bring down the index , 
	r_index--;

	for (int i = 1; i <= old_states.size ();i++)
		this->sparse_map_[r_index][i] = 1;
}

// creates the sparse matrice from the map
void State_Space_Gen::create_sparse_matrice_from_map ()
{
	// go through the rate matrix, find the non-zero elements and 
	// print them to the files ...

	//variables which will be output 
	stringstream i_str;
	stringstream j_str;
	stringstream s_str;

	i_str << "i = [ ";
	j_str << "j = [ ";
	s_str << "s = [ ";

	bool first = true;

	// taking each row 
	for each (pair<int, SP_ROW> spr in this->sparse_map_)
	{
		for each (pair<int, int> val in spr.second)
		{
			if (first == true)
			{
				i_str << spr.first; // row num
				j_str << val.first; // col num
				s_str << val.second; // value 
				first = false;
			}
			else
			{
				i_str << ", " << spr.first;
				j_str << ", " << val.first;
				s_str << ", " << val.second;
			}
		}
	}
	
	i_str << " ];";
	j_str << " ];";
	s_str << " ];";

	ofstream sp ("sparse_a.m");

	sp << i_str.str () << endl
			<< j_str.str () << endl
			<< s_str.str () << endl
			<< "m = " << old_states.size () << endl
			<< "n = " << old_states.size () << endl
			<< "a = sparse ( i, j, s, m, n);" << endl;

	sp.close ();
}