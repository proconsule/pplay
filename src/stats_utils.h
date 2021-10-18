//
// Created by proconsule on 13/10/21.
//

#ifndef STATS_UTILS_H
#define STATS_UTILS_H

#include <stdio.h>


#include <switch.h>

#define NVGPU_GPU_IOCTL_PMU_GET_GPU_LOAD 0x80044715

class Main;

void ExitCpuMeterServices();

class CPU_Meter{
public:
	CPU_Meter(Main *_main);
	
	~CPU_Meter();
	
	void initServices();
	void GetCores_Perc();
	
	Main *main = nullptr;
	
	uint64_t systemtickfrequency = 19200000;
	
	char CPU_Usage0[32];
	char CPU_Usage1[32];
	char CPU_Usage2[32];
	char CPU_Usage3[32];
	
	//Temperatures
	
	char SoCPCB_temperature_c[64];
	char skin_temperature_c[32];
	
	
	//Frequency
	///CPU
	char CPU_Hz_c[32];
	///GPU
	char GPU_Hz_c[32];
	///RAM
	char RAM_Hz_c[32];

	//RAM Size
	char RAM_all_c[64];
	char RAM_application_c[64];
	char RAM_applet_c[64];
	char RAM_system_c[64];
	char RAM_systemunsafe_c[64];
	char RAM_compressed_c[320];
	char RAM_var_compressed_c[320];
	

	//Fan
	
	char Rotation_SpeedLevel_c[64];

	//GPU Usage
	
	char GPU_Load_c[32];
	
	Thread t0;
	Thread t1;
	Thread t2;
	Thread t3;
	Thread t4;
	
	Thread t5;
	
	
};


#endif