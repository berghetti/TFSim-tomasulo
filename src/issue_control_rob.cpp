#include "issue_control_rob.hpp"
#include "general.hpp"

issue_control_rob::issue_control_rob(sc_module_name name): sc_module(name)
{
	res_type = {{"DADD",1},{"DADDI",1},{"DADDU",1},
				{"DADDIU",1},{"DSUB",1},{"DSUBU",1},
				{"DMUL",1},{"DMULU",1},{"DDIV",1},
				{"DDIVU",1},{"LD",2},{"SD",3},
				{"BEQ",4},{"BNE",4},{"BGTZ",4},
				{"BLTZ",4},{"BGEZ",4},{"BLEZ",4}};
	SC_THREAD(issue_select);
	sensitive << in;
	dont_initialize();
}

void issue_control_rob::issue_select()
{
	while(true)
	{
		in->nb_read(p);
		out_rob->write(p);
		in_rob->read(rob_pos);
		ord = instruction_split(p);
		switch(res_type[ord[0]])
		{
			case 1:
				out_rsv->write(p + ' ' + rob_pos);
				break;
			case 2:
				out_slbuff->write(p + ' ' + rob_pos);
				out_adu->write(p + ' ' + rob_pos);
				break;
			case 3:
				out_adu->write(p + ' ' + rob_pos);
				break;
			case 4:
				break;
			default:
				cerr << "Instruçao nao suportada!" << endl << flush;
				sc_stop();
				exit(1);
		}
		in->notify();
		wait();
	}
}

