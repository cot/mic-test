//
//
// Author:  Reza  Rahman
//
//
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define  MICVEC_DEFINE_OUTPUT_OPERATORS
#include <iostream>
#include <micvec.h>

#include <sink/COIPipeline_sink.h>
#include <sink/COIProcess_sink.h>
#include <sink/COIBuffer_sink.h>
#include <common/COIMacros_common.h>

#include "type.h"

/* Global Variables used for I/O */
void* g_buffer 			= NULL;
adble* g_input_buffer		= NULL;
adble* g_output_buffer		= NULL;
uint64_t g_buffer_length	= 0;

int main(int argc, char*argv[]){

	UNUSED_ATTR COIRESULT result;
	UNREFERENCED_PARAM(argc);
	UNREFERENCED_PARAM(argv);

	/* Call to enqueued functions on the sink : done to synchronize any initialisation required on the sink side */
	result = COIPipelineStartExecutingRunFunctions();
	assert(result == COI_SUCCESS);

	/* Wait the COIProcessDestroy call on the source side */
	COIProcessWaitForShutdown();

	free(g_buffer);
	g_buffer = NULL;
	g_buffer_length = 0;
	return 0;
}

COINATIVELIBEXPORT
void alloc() {

}

COINATIVELIBEXPORT
void init() {

}
COINATIVELIBEXPORT
void todo() {
	char command[50];
	
	strcpy(command,"cat /proc/cpuinfo |grep 'cpu family'|wc -l");
	system(command);

	printf("I'm on the mic\n");

}

COINATIVELIBEXPORT
void clean() {

}

/*

	__declspec(align(64)) Is32vec16 inputData(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
	__declspec(align(64)) Is32vec16 outputData; std::cout << "input = " << inputData;

	// Swizzle input data and print
	std::cout << "\n nswizzle data for pattern 'cdab' \n" <<  inputData.cdab();

	// Swizzle input data and print
	// --> shuffle intra lane data
	std::cout << "\n Intra lane shuffle data for pattern 'aaaa' \n";
	outputData = Is32vec16(_mm512_shuffle_epi32(__m512i(inputData), _MM_PERM_AAAA)); std::cout << outputData << "\n";

	// --> shuffle inter lane data
	std::cout << " Inter lane shuffle data for pattern 'aabc' \n";
	outputData = Is32vec16(_mm512_permute4f128_epi32(__m512i(inputData), _MM_PERM_AABC)); std::cout << outputData << "\n";
*/


