#include "stdafx.h"
#include "HelloWorld.h"

#define __CRT_SECURE_NO_WARNINGS
#define __CL_ENABLE_EXCEPTIONS

#include <fstream>
#include <iostream>
#include <iterator>

#ifdef MAC
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

#define ELE_TYPE unsigned int
#define  NUM_ELEMENTS 128000

ELE_TYPE w_hash(ELE_TYPE seed)
{
	seed = (seed ^ 61) ^ (seed >> 16);
	seed *= 9;
	seed = seed ^ (seed >> 4);
	seed *= 0x27d4eb2d;
	seed = seed ^ (seed >> 15);
	return seed;
}

int main() {

	std::vector<cl::Platform> platforms;
	std::vector<cl::Device> allDevices, platformDevices, ctxDevices;
	cl::Event profileEvent;
	std::string platform_name, device_name;
	std::string kernelName;

	try {// Place the devices of the chosen platform into a context
		cl::Platform::get(&platforms);
		for (cl_uint i = 0; i < platforms.size(); i++) {
			platform_name = platforms[i].getInfo<CL_PLATFORM_NAME>();
			std::cout << "Platform[" << i << "]: " << platform_name << std::endl;
		}
		// Chose one of the platforms
		cl_int p_index = 1;
		std::cout << "Chosen Platform:" << p_index << std::endl;
		platforms[p_index].getDevices(CL_DEVICE_TYPE_GPU, &platformDevices);

		// Create context and access device names, chose one of the devices
		cl::Context context(platformDevices);
		ctxDevices = context.getInfo<CL_CONTEXT_DEVICES>();
		for (cl_uint i = 0; i < ctxDevices.size(); i++) {
			device_name = ctxDevices[i].getInfo<CL_DEVICE_NAME>();
			std::cout << "Device[" << i << "]: " << device_name << std::endl;
		}
		cl_int d_index = 0;
		std::cout << "Chosen Device:" << d_index << std::endl;

		// Create and build programs
		std::ifstream programFile("HelloWorld_Kernel.cl");
		std::string programString(std::istreambuf_iterator<char>(programFile),
			(std::istreambuf_iterator<char>()));
		cl::Program::Sources source(1, std::make_pair(programString.c_str(),
			programString.length() + 1));
		cl::Program program(context, source);
		try {

			std::cout << "Building Program ..." << std::endl;
			cl_int err = program.build(ctxDevices, "-cl-std=CL1.2");

			std::cout << "Program Build Code = " << err << std::endl;
		}
		catch (cl::Error e) {
			std::cout << e.what() << ": Error code " << e.err() << std::endl;
			std::cout << "Build log: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(ctxDevices[d_index])
				<< std::endl;
		}


		std::cout << "Program Build Complete..." << std::endl;
		//getchar();
		// Create individual kernels
		cl_int err = CL_SUCCESS;
		cl::Kernel kernel(program, "testkernel", &err);
		std::cout << "Kernel Build Code = " << err << std::endl;
		kernelName = kernel.getInfo<CL_KERNEL_FUNCTION_NAME>();
		std::cout << "Kernel: " << kernelName << std::endl;


		int bufferSize = NUM_ELEMENTS * sizeof(ELE_TYPE);
		ELE_TYPE* hostBuff = (ELE_TYPE*)malloc(bufferSize);
		for (int i = 0; i < NUM_ELEMENTS; ++i) {
			hostBuff[i] = i; //rand();
		}

		cl::Buffer outbuff(context,(cl_mem_flags) CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, bufferSize, hostBuff);
		kernel.setArg(0, sizeof(cl::Buffer), &outbuff);

	
		// Enqueue kernel-execution command with profiling event
		cl::CommandQueue queue(context, ctxDevices[d_index], CL_QUEUE_PROFILING_ENABLE);
		queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(NUM_ELEMENTS),
			cl::NullRange, NULL, NULL);
		queue.finish();

		ELE_TYPE* devBuff = (ELE_TYPE*)queue.enqueueMapBuffer(outbuff, CL_TRUE, CL_MAP_READ, 0, bufferSize);

		bool isMatched = true;
		unsigned int resultVal = 0;
		unsigned int maxint = 0; maxint--;
		for (int i = 0; i < NUM_ELEMENTS; ++i) {
			resultVal = w_hash(hostBuff[i]);
			if (resultVal < maxint / 2)
			{
				resultVal = w_hash(w_hash(w_hash(hostBuff[i])));
			}

			if (resultVal != devBuff[i]) {
				isMatched = false;
				break;
			}
		}

		queue.enqueueUnmapMemObject(outbuff, devBuff);

		//std::cout << "\nDone: " << std::endl;
		std::cout << "\n" << (isMatched ? "Matched" : "Not Matched") << std::endl;

		free(hostBuff);
	}
	catch (cl::Error e) {
		std::cout << e.what() << ": Error code " << e.err() << std::endl;
	}

	getchar();
	return 0;
}

HelloWorld::HelloWorld()
{
}


HelloWorld::~HelloWorld()
{
}
