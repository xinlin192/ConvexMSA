
class Param{
	public:
	char* trainFname;
	char* modelFname;
	char* heldoutFname;
	float_type lambda; //for L1-norm (default 1/N)
	float_type C; //weight of loss
	int speed_up_rate; // speed up rate for sampling
	int split_up_rate; // split up [K] into a number of subsets	
	Problem* train;
	HeldoutEval* heldoutEval = NULL;
	//solver-specific param
	int solver;
	int max_iter;
	int max_select;
	bool using_importance_sampling;
	int post_solve_iter;

	Param(){
		solver = 0;
		lambda = 1.0;
		C = 1.0;
		max_iter = 20;
		max_select = 1;
		speed_up_rate = 1;
		split_up_rate = 1;
		using_importance_sampling = false;
		post_solve_iter = 0;

		heldoutFname == NULL;
		train = NULL;
	}
};

void parse_cmd_line(int argc, char** argv, Param* param){

	int i;
	for(i=1;i<argc;i++){
		if( argv[i][0] != '-' )
			break;
		if( ++i >= argc )
			exit_with_help();

		switch(argv[i-1][1]){
			
			case 's': param->solver = atoi(argv[i]);
				  break;
			case 'l': param->lambda = atof(argv[i]);
				  break;
			case 'c': param->C = atof(argv[i]);
				  break;
			case 'm': param->max_iter = atoi(argv[i]);
				  break;
			case 'g': param->max_select = atoi(argv[i]);
				  break;
			case 'r': param->speed_up_rate = atoi(argv[i]);
				  break;
			case 'i': param->using_importance_sampling = true; --i;
				  break;
			case 'q': param->split_up_rate = atoi(argv[i]);
				  break;
			case 'p': param->post_solve_iter = atoi(argv[i]);
				  break;
			case 'h': param->heldoutFname = argv[i];
				  break;
			default:
				  cerr << "unknown option: -" << argv[i-1][1] << endl;
				  exit(0);
		}
	}

	if(i>=argc)
		exit_with_help();

	param->trainFname = argv[i];
	i++;

	if( i<argc )
		param->modelFname = argv[i];
	else{
		param->modelFname = new char[FNAME_LEN];
		strcpy(param->modelFname,"model");
	}
}
