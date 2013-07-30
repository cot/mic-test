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
void alloc(uint32_t         in_BufferCount,
           void**           in_ppBufferPointers,
           uint64_t*        in_pBufferLengths,
           void*            in_pMiscData,
           uint16_t         in_MiscDataLength,
           void*            in_pReturnValue,
           uint16_t         in_ReturnValueLength) {

	UNREFERENCED_PARAM(in_BufferCount);
	UNREFERENCED_PARAM(in_ppBufferPointers);
	UNREFERENCED_PARAM(in_pBufferLengths);
	if (in_MiscDataLength != sizeof(uint64_t) || !in_pMiscData ||
			in_ReturnValueLength != sizeof(void*) || !in_pReturnValue)
	{
		return;
	}


	// Allocate memory that will be used later as a Intel® Coprocessor Offload Infrastructure (Intel® COI)  buffer.

	uint64_t size = *(uint64_t*)in_pMiscData;
	posix_memalign(&g_buffer, 4096, size);

	if (!g_buffer)
	{
		printf("Error allocating memory for the buffer\n");fflush(0);
		goto end;
	}
	// The single allocation will be used as two halves, the first half to
	// hold input data and the second half to hold the output data.
	//
	g_buffer_length = size;
	g_input_buffer = (adble*)g_buffer;
	g_output_buffer = &((adble*)g_buffer)[size/2];

	// Need to return the address back to the source so that it can be
	// passed to COIBufferCreateFromMemory.
	//
end:
	*(void**)in_pReturnValue = g_buffer;


	printf("alloc on the mic\n");

}

COINATIVELIBEXPORT
void init(
          uint32_t         in_BufferCount,
          void**           in_ppBufferPointers,
          uint64_t*        in_pBufferLengths,
          void*            in_pMiscData,
          uint16_t         in_MiscDataLength,
          void*            in_pReturnValue,
          uint16_t         in_ReturnValueLength) {

	UNREFERENCED_PARAM(in_BufferCount);
	UNREFERENCED_PARAM(in_ppBufferPointers);
	UNREFERENCED_PARAM(in_pBufferLengths);
	UNREFERENCED_PARAM(in_pMiscData);
	UNREFERENCED_PARAM(in_MiscDataLength);
	UNREFERENCED_PARAM(in_pReturnValue);
	UNREFERENCED_PARAM(in_ReturnValueLength);

	assert(in_BufferCount == 1);

	COIRESULT res = COIBufferAddRef(g_buffer);

	assert(res==COI_SUCCESS);
}
COINATIVELIBEXPORT
void todo(
		uint32_t         in_BufferCount,
		void**           in_ppBufferPointers,
		uint64_t*        in_pBufferLengths,
		void*            in_pMiscData,
		uint16_t         in_MiscDataLength,
		void*            in_pReturnValue,
		uint16_t         in_ReturnValueLength) {
	UNREFERENCED_PARAM(in_BufferCount);
	UNREFERENCED_PARAM(in_ppBufferPointers);
	UNREFERENCED_PARAM(in_pBufferLengths);
	UNREFERENCED_PARAM(in_pMiscData);
	UNREFERENCED_PARAM(in_MiscDataLength);
	UNREFERENCED_PARAM(in_pReturnValue);
	UNREFERENCED_PARAM(in_ReturnValueLength);

	uint64_t i;
	
	for(i=0;i<g_buffer_length/2;i++) {
		g_output_buffer[i] =  g_input_buffer[i] * 2;
		printf("g_input_buffer[%i] = %g \n",(int)i,g_input_buffer[i]);
		printf("g_output_buffer[%i] = %g \n",(int)i,g_output_buffer[i]);
	}

	char command[50];
	strcpy(command,"cat /proc/cpuinfo |grep 'cpu family'|wc -l");
	system(command);

	printf("I'm on the mic\n");

}

COINATIVELIBEXPORT
void clean(
          uint32_t         in_BufferCount,
          void**           in_ppBufferPointers,
	  uint64_t*        in_pBufferLengths,
	  void*            in_pMiscData,
	  uint16_t         in_MiscDataLength,
	  void*            in_pReturnValue,
	  uint16_t         in_ReturnValueLength) {
	UNREFERENCED_PARAM(in_BufferCount);
	UNREFERENCED_PARAM(in_ppBufferPointers);
	UNREFERENCED_PARAM(in_pBufferLengths);
	UNREFERENCED_PARAM(in_pMiscData);
	UNREFERENCED_PARAM(in_MiscDataLength);
	UNREFERENCED_PARAM(in_pReturnValue);
	UNREFERENCED_PARAM(in_ReturnValueLength);

	COIBufferReleaseRef(g_buffer);

}

