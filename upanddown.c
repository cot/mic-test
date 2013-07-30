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
	uint64_t SIZE;
	uint64_t amount;
	int count;
	struct timeval beg, end;
//	__declspec(target(mic)) adble *p1off;

	/*COI Variables */
	static unsigned nb_devices = 0;
	COIENGINE engine;
	COIPROCESS *coi_process;
	COIPROCESS proc;
	COIRESULT res;
	COIBUFFER buffer;
	COIPIPELINE pipeline;
	COIEVENT function_event;
	COIEVENT write_event;
	COIFUNCTION alloc_func;
	COIFUNCTION init_func;
	COIFUNCTION todo_func;
	COIFUNCTION clean_func;
	COIFUNCTION lookup_funcs[4];
	COI_ACCESS_FLAGS flags = COI_SINK_WRITE;
	adble* input_data = NULL;
	adble* output_data = NULL;
	void* buffer_ptr ;
	uint64_t buffer_size;
	const char*  SINK_NAME = "offload_on_mic";

	/* Defining program size */
	if(argc>1)
		SIZE = atoi(argv[1]);
	else
		SIZE = 100;

	amount = SIZE * sizeof(adble);
	buffer_size = 2 * SIZE;
	printf("Pb size = %i and buffer_size = %i\n",(int)SIZE,(int)buffer_size);

	/* Define input data */
	input_data  = (adble*)malloc(SIZE*sizeof(adble));
	output_data = (adble*)calloc(SIZE,sizeof(adble));
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
			SIZE,           // The amount of memory to pre-allocate and register for use with COIBUFFERs.
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
	assert(res==COI_SUCCESS);
	alloc_func      = lookup_funcs[0];
	init_func       = lookup_funcs[1];
	todo_func	= lookup_funcs[2];
	clean_func      = lookup_funcs[3];
	printf("Got function handles\n");

/* Call alloc function */
        /* Launch the run functions */
        res = COIPipelineRunFunction(
                        pipeline, alloc_func,   // Pipeline handle and function handle
                        0, NULL, NULL,          // Buffers and access flags to pass to the function
                        0, NULL,                // Input dependencies
                        &buffer_size,           // Misc data to pass to the function
                        sizeof(buffer_size),    //
                        &buffer_ptr,            // Return values that will be passed back
                        sizeof(buffer_ptr),
                        &function_event);
        assert(res==COI_SUCCESS);

        res = COIEventWait(
                        1,                      // Number of event to wait
                        &function_event,        // Event handle
                        -1,                     // Wait indefinitely
                        true,                   // Wait for all events
                        NULL,NULL);             // Number of event signaled and their indices
        assert(res==COI_SUCCESS);

	printf("Got buffer address %p\n",buffer_ptr);
	if(!buffer_ptr) {
		printf("Failed to allocate buffer memory on the sink\n");
		return -1;
	}

	res = COIBufferCreateFromMemory(
			buffer_size,		// The size of the buffer being created
			COI_BUFFER_NORMAL,	// Allocate a normal buffer type
			COI_SINK_MEMORY,	// Flag indicates the memory is on the sink
			buffer_ptr,		// Virtual address from the sink
			1,			// Number of process where buffer will be used
			&proc,			// Array of process handles
			&buffer);		// Output handle for the buffer
        assert(res==COI_SUCCESS);


/* Return alloc function */

/* Call init function */
	/* Launch the run functions */
	res = COIPipelineRunFunction(
			pipeline, init_func,
			1, &buffer, &flags,
			0, NULL,
			NULL, 0,
			NULL, 0,
			&function_event);
        assert(res==COI_SUCCESS);

	res = COIBufferWrite(
			buffer,             // Destination buffer to write to
			0,                  // Starting offset to write to in the buffer
			input_data,         // Address of the memory with the source data
			SIZE,               // Number of bytes to write
			COI_COPY_USE_DMA,   // How to transfer the data, force DMA here
			0, NULL,            // Input dependencies
			&write_event        // Completion event signaled when DMA finishes
		      );
        assert(res==COI_SUCCESS);
/* Return init function */

/* Call todo function */
	/* Launch the run functions */
	res = COIPipelineRunFunction(
			pipeline, todo_func,	// Pipeline handle and function handle
			0, NULL, NULL,		// Buffers and access flags to pass to the function
			1, &write_event,	// Input dependencies
			NULL,0,			// Misc data to pass to the function
			NULL,0, 		// Return values that will be passed back
			&function_event);
	assert(res==COI_SUCCESS);

	res = COIEventWait(
			1,			// Number of event to wait
			&function_event,	// Event handle
			-1,			// Wait indefinitely
			true,			// Wait for all events
			NULL,NULL);		// Number of event signaled and their indices
	assert(res==COI_SUCCESS);

	res = COIBufferRead(
			buffer,             	// Source buffer to read from
			SIZE,        	    	// Started offset to read from in the buffer
			output_data,        	// Local memory to use as destination
			SIZE,          	    	// Number of bytes to read
			COI_COPY_USE_DMA,   	// How to transfer the data, force DMA here
			1, &function_event, 	// Input dependencies
			COI_EVENT_SYNC      	// Force this read to be synchronous
			);
	assert(res == COI_SUCCESS);
/* Return todo function */
	for(i=0;i<SIZE;i++){
		printf("%g \n",output_data[i] / input_data[i]);
//		assert( (output_data[i] / input_data[i]) == 2);
		}	
/* Call clean function */
        /* Launch the run functions */
        res = COIPipelineRunFunction(
                        pipeline, clean_func,   // Pipeline handle and function handle
                        0, NULL, NULL,          // Buffers and access flags to pass to the function
                        0, NULL,	        // Input dependencies
                        NULL,0,                 // Misc data to pass to the function
                        NULL,0,                 // Return values that will be passed back
                        &function_event);
        assert(res==COI_SUCCESS);

        res = COIEventWait(
                        1,                      // Number of event to wait
                        &function_event,        // Event handle
                        -1,                     // Wait indefinitely
                        true,                   // Wait for all events
                        NULL,NULL);             // Number of event signaled and their indices
        assert(res==COI_SUCCESS);
/* Return clean function */

/* Destroy the buffer */
	res = COIBufferDestroy(buffer);
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
	printf("Time to do everything on the mic = \t");
	printf(" %g\n", ((end.tv_sec + end.tv_usec * 1e-6) - (beg.tv_sec + beg.tv_usec * 1e-6)));
	printf("_________________________________________________________________________\n\n");

	free(input_data);
	free(output_data);

	return EXIT_SUCCESS;

}

