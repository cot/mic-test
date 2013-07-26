#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <sys/time.h>

#include <wchar.h>
#include <source/COIProcess_source.h>
#include <common/COITypes_common.h>
#include <common/COIResult_common.h>
#include <common/COIEngine_common.h>
#include <source/COIEngine_source.h>
#include <source/COIPipeline_source.h>
#include <source/COIEvent_source.h>
#include <source/COIBuffer_source.h>

#include "type.h"

int main(int argc,char *argv[]) {

	/*Program Variables */
	int i;
	int SIZE;
	int count;
	struct timeval beg, end;
//	__declspec(target(mic)) adble *p1off;

	/*COI Variables */
	static unsigned nb_devices = 0;
	COIENGINE engine;
	COIPROCESS *coi_process;
	COIPROCESS proc;
	COIRESULT res;
	COIPIPELINE pipeline;
	COIEVENT function_event;
	COIFUNCTION alloc_func;
	COIFUNCTION init_func;
	COIFUNCTION todo_func;
	COIFUNCTION clean_func;
	COIFUNCTION lookup_funcs[4];
	adble* input_data = NULL;
	adble* output_data = NULL;
	void* buffer_ptr = NULL;
	uint64_t buffer_size;
	const char*  SINK_NAME = "offload_on_mic";

	/* Defining program size */
	if(argc>1)
		SIZE = atoi(argv[1]);
	else
		SIZE = 1000;
	printf("Pb SIZE = %i\n",SIZE);
	buffer_size = 2 * SIZE;

	/* Define input data */
	input_data  = (adble*)malloc(SIZE*sizeof(adble));
	output_data = (adble*)malloc(SIZE*sizeof(adble));
	for(i=0; i<SIZE; i++)
		input_data[i] = i;
	
	/* Time evaluation */
	gettimeofday(&beg, NULL);
	/* Return the number of MIC devices in the system */
	res = COIEngineGetCount(COI_ISA_MIC, &nb_devices);
	if (res != COI_SUCCESS)	nb_devices = 0;
	printf("number of device(s) = %i\n",nb_devices);

	/* Let's get the handle which let us manage the remote MIC device (ISA : Industry Standard Architecture) */
	res = COIEngineGetHandle(COI_ISA_MIC, nb_devices-1, &engine);
	assert(res==COI_SUCCESS);

	/* The following call creates a process on the sink */
	res = COIProcessCreateFromFile(
			engine,         // The engine to create the process on.
			SINK_NAME,      // The local path to the sink side binary to launch.
			0, NULL,        // argc and argv for the sink process.
			false, NULL,    // Environment variables to set for the sink process.
			true, NULL,     // Enable the proxy but don't specify a proxy root path.
			0,              // The amount of memory to pre-allocate and register for use with COIBUFFERs.
			NULL,           // Path to search for dependencies
			&proc           // The resulting process handle.
			);
	assert(res==COI_SUCCESS);

	/* Create a pipeline to execute the run functions  */
	res = COIPipelineCreate(proc, NULL, 0, &pipeline);
	assert(res==COI_SUCCESS);

	/* Define the function to execute on the mic and retrieve handles to them */
	const char* funcs[] = {"alloc","init","todo","clean"};
	res = COIProcessGetFunctionHandles(
			proc,		// Process to query for the function
			4,		// The number of functions to query
			funcs,
			lookup_funcs	// Handles to the functions
			);
	assert(res == COI_SUCCESS);
	alloc_func      = lookup_funcs[0];
	init_func       = lookup_funcs[1];
	todo_func	= lookup_funcs[2];
	clean_func      = lookup_funcs[3];

	/* Launch the run function */
	res = COIPipelineRunFunction(
			pipeline, todo_func,	// Pipeline handle and function handle
			0, NULL, NULL,		// Buffers and access flags to pass to the function
			0, NULL,		// Input dependencies
			NULL,			// Misc data to pass to the function
			0,			//
			NULL,			// Return values that will be passed back
			0,
			&function_event);
	assert(res==COI_SUCCESS);

	res = COIEventWait(
			1,			// Number of event to wait
			&function_event,	// Event handle
			-1,			// Wait indefinitely
			true,			// Wait for all events
			NULL,NULL);		// Number of event signaled and their indices
	assert(res==COI_SUCCESS);

	/* Destroy the pipeline */
	res = COIPipelineDestroy(pipeline);
	assert(res==COI_SUCCESS);

	/* Destroy the process */
	res = COIProcessDestroy(
			proc,			// Process handle to be destroyed
			-1,			// Wait indefinitely
			false,			// Don't force to exit
			NULL,			// Don't care about the exit result
			NULL);			// Don't care what the exit reason was
	assert(res==COI_SUCCESS);

	gettimeofday(&end, NULL);
	/* Time evaluation */
	printf("up and down offload and work = \t");
	printf(" %g\n", ((end.tv_sec + end.tv_usec * 1e-6) - (beg.tv_sec + beg.tv_usec * 1e-6)));
	printf("_________________________________________________________________________\n\n");

	free(input_data);
	free(output_data);

	return EXIT_SUCCESS;

}

