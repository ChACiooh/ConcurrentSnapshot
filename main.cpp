#include "my_thread.h"
#include <time.h>

using namespace std;

bool check_arg(int argc)
{
	/* Need to check the command line message to check N. */
	if (argc < 2) {
		printf("Error: need more argument(s).\n");
		printf("run as \"./run [number of threads]\"\n");
		return false;
	} else if (argc > 2) {
		printf("Error: too many arguments.\n");
		printf("run as \"./run [number of threads]\"\n");
		return false;
	}
	return true;
}

// =================== main function ==================== //
int main(int argc, char* argv[])	// #2. parameterize the number of threads
{
	srand(time(NULL));		// for generating random variable to update
	if(!check_arg(argc))	return 0;
	
	// construct the container for thread values
	int n = atoi(argv[1]);
	set_thread_number(n);
	printf("The number of threads: %d\n", n);
	gen_threads();


	// initialize the counter
	initialize_counter();

	// use boost::thread_group to implement Wait-free
	//set_boost_asio_io_services();
	printf("All of initialization is done.\n");

	//threadpool_create();

	clock_t start_time, end_time;
	start_time = clock();
    while (1) {
		for(int tid = 0; tid < n; ++tid) {
			//io_post_update_thread_value(tid);
			update_thread_value(tid);
			end_time = clock();
			if(end_time - start_time >= 60 * CLOCKS_PER_SEC)	goto OUTSIDE;
		}
		if(end_time - start_time >= 60 * CLOCKS_PER_SEC)	break;
    }
OUTSIDE:

    //delete_threadpool();
	print_counter();
	goodbye_ollie();
	return 0;
}
