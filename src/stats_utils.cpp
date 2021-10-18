#include "stats_utils.h"
#include "main.h"
#include "player/video_texture.h"

uint64_t idletick0 = 19200000;
uint64_t idletick1 = 19200000;
uint64_t idletick2 = 19200000;

uint32_t CPU_Hz = 0;

uint32_t GPU_Hz = 0;

uint32_t RAM_Hz = 0;


uint64_t RAM_Total_all_u = 0;
uint64_t RAM_Total_application_u = 0;
uint64_t RAM_Total_applet_u = 0;
uint64_t RAM_Total_system_u = 0;
uint64_t RAM_Total_systemunsafe_u = 0;
uint64_t RAM_Used_all_u = 0;
uint64_t RAM_Used_application_u = 0;
uint64_t RAM_Used_applet_u = 0;
uint64_t RAM_Used_system_u = 0;
uint64_t RAM_Used_systemunsafe_u = 0;

FanController g_ICon;
float Rotation_SpeedLevel_f = 0;

uint32_t fd = 0;
uint32_t GPU_Load_u = 0;

int32_t SoC_temperaturemiliC = 0;
int32_t PCB_temperaturemiliC = 0;
int32_t skin_temperaturemiliC = 0;

Result clkrstCheck = 1;
Result pcvCheck = 1;
	
Result nvCheck = 1;
Result tsCheck = 1;
Result fanCheck = 1;
Result tcCheck = 1;
Result Hinted = 1;



bool threadexit2 = false;



void CheckCore0(void* arg) {
	while (threadexit2 == false) {
		static uint64_t idletick_a0 = 0;
		static uint64_t idletick_b0 = 0;
		svcGetInfo(&idletick_b0, InfoType_IdleTickCount, INVALID_HANDLE, 0);
		svcSleepThread(1'000'000'000);
		svcGetInfo(&idletick_a0, InfoType_IdleTickCount, INVALID_HANDLE, 0);
		idletick0 = idletick_a0 - idletick_b0;
	}
}

void CheckCore1(void* arg) {
	while (threadexit2 == false) {
		static uint64_t idletick_a1 = 0;
		static uint64_t idletick_b1 = 0;
		svcGetInfo(&idletick_b1, InfoType_IdleTickCount, INVALID_HANDLE, 1);
		svcSleepThread(1'000'000'000);
		svcGetInfo(&idletick_a1, InfoType_IdleTickCount, INVALID_HANDLE, 1);
		idletick1 = idletick_a1 - idletick_b1;
	}
}

void CheckCore2(void* arg) {
	while (threadexit2 == false) {
		static uint64_t idletick_a2 = 0;
		static uint64_t idletick_b2 = 0;
		svcGetInfo(&idletick_b2, InfoType_IdleTickCount, INVALID_HANDLE, 2);
		svcSleepThread(1'000'000'000);
		svcGetInfo(&idletick_a2, InfoType_IdleTickCount, INVALID_HANDLE, 2);
		idletick2 = idletick_a2 - idletick_b2;
	}
}

void Misc(void* arg) {
	while (threadexit2 == false) {
		
		// CPU, GPU and RAM Frequency
		if (R_SUCCEEDED(clkrstCheck)) {
			ClkrstSession clkSession;
			if (R_SUCCEEDED(clkrstOpenSession(&clkSession, PcvModuleId_CpuBus, 3))) {
				clkrstGetClockRate(&clkSession, &CPU_Hz);
				clkrstCloseSession(&clkSession);
			}
			if (R_SUCCEEDED(clkrstOpenSession(&clkSession, PcvModuleId_GPU, 3))) {
				clkrstGetClockRate(&clkSession, &GPU_Hz);
				clkrstCloseSession(&clkSession);
			}
			if (R_SUCCEEDED(clkrstOpenSession(&clkSession, PcvModuleId_EMC, 3))) {
				clkrstGetClockRate(&clkSession, &RAM_Hz);
				clkrstCloseSession(&clkSession);
			}
		}
		else if (R_SUCCEEDED(pcvCheck)) {
			pcvGetClockRate(PcvModule_CpuBus, &CPU_Hz);
			pcvGetClockRate(PcvModule_GPU, &GPU_Hz);
			pcvGetClockRate(PcvModule_EMC, &RAM_Hz);
		}
		
		//Temperatures
		if (R_SUCCEEDED(tsCheck)) {
			tsGetTemperatureMilliC(TsLocation_External, &SoC_temperaturemiliC);
			tsGetTemperatureMilliC(TsLocation_Internal, &PCB_temperaturemiliC);
		}
		if (R_SUCCEEDED(tcCheck)) tcGetSkinTemperatureMilliC(&skin_temperaturemiliC);
		
		//RAM Memory Used
		
			svcGetSystemInfo(&RAM_Total_application_u, 0, INVALID_HANDLE, 0);
			svcGetSystemInfo(&RAM_Total_applet_u, 0, INVALID_HANDLE, 1);
			svcGetSystemInfo(&RAM_Total_system_u, 0, INVALID_HANDLE, 2);
			svcGetSystemInfo(&RAM_Total_systemunsafe_u, 0, INVALID_HANDLE, 3);
			svcGetSystemInfo(&RAM_Used_application_u, 1, INVALID_HANDLE, 0);
			svcGetSystemInfo(&RAM_Used_applet_u, 1, INVALID_HANDLE, 1);
			//svcGetSystemInfo(&cpu_meter->RAM_Used_system_u, 1, INVALID_HANDLE, 2);
			//svcGetSystemInfo(&cpu_meter->RAM_Used_systemunsafe_u, 1, INVALID_HANDLE, 3);
		
		
		//Fan
		if (R_SUCCEEDED(fanCheck)) fanControllerGetRotationSpeedLevel(&g_ICon, &Rotation_SpeedLevel_f);
		
		//GPU Load
		if (R_SUCCEEDED(nvCheck)) nvIoctl(fd, NVGPU_GPU_IOCTL_PMU_GET_GPU_LOAD, &GPU_Load_u);
		
		// Interval
		svcSleepThread(1'000'000'000);
	}
}

CPU_Meter::CPU_Meter(Main *_main){


		main = _main;
		initServices();
	
		threadCreate(&t0, CheckCore0, NULL, NULL, 0x200, 0x2C, 0);
		threadCreate(&t1, CheckCore1, NULL, NULL, 0x200, 0x2C, 1);
		threadCreate(&t2, CheckCore2, NULL, NULL, 0x200, 0x2C, 2);
		threadCreate(&t4, Misc, NULL, NULL, 0x200, 0x3B, -2);
		threadStart(&t0);
		threadStart(&t1);
		threadStart(&t2);
		threadStart(&t4);
		printf("CPU_Meter Init");
}

void CPU_Meter::GetCores_Perc(){
	
	if (idletick0 > systemtickfrequency) idletick0 = systemtickfrequency;
	if (idletick1 > systemtickfrequency) idletick1 = systemtickfrequency;
	if (idletick2 > systemtickfrequency) idletick2 = systemtickfrequency;
	
	
	snprintf(CPU_Hz_c, sizeof CPU_Hz_c, "CPU: Frequency: %.1f MHz", (float)CPU_Hz / 1000000);
	
	snprintf(CPU_Usage0, sizeof CPU_Usage0, "Core #0: %.2f%s", ((double)systemtickfrequency - (double)idletick0) / (double)systemtickfrequency * 100, "%");
	snprintf(CPU_Usage1, sizeof CPU_Usage1, "Core #1: %.2f%s", ((double)systemtickfrequency - (double)idletick1) / (double)systemtickfrequency * 100, "%");
	snprintf(CPU_Usage2, sizeof CPU_Usage2, "Core #2: %.2f%s", ((double)systemtickfrequency - (double)idletick2) / (double)systemtickfrequency * 100, "%");
	
	snprintf(GPU_Hz_c, sizeof GPU_Hz_c, "GPU: Frequency: %.1f MHz", (float)GPU_Hz / 1000000);
	snprintf(GPU_Load_c, sizeof GPU_Load_c, "GPU: Load: %.1f%s", (float)GPU_Load_u / 10, "%");
	
	float RAM_Total_application_f = (float)RAM_Total_application_u / 1024 / 1024;
	float RAM_Total_applet_f = (float)RAM_Total_applet_u / 1024 / 1024;
	float RAM_Total_system_f = (float)RAM_Total_system_u / 1024 / 1024;
	float RAM_Total_systemunsafe_f = (float)RAM_Total_systemunsafe_u / 1024 / 1024;
	float RAM_Total_all_f = RAM_Total_application_f + RAM_Total_applet_f + RAM_Total_system_f + RAM_Total_systemunsafe_f;
	float RAM_Used_application_f = (float)RAM_Used_application_u / 1024 / 1024;
	float RAM_Used_applet_f = (float)RAM_Used_applet_u / 1024 / 1024;
	float RAM_Used_system_f = (float)RAM_Used_system_u / 1024 / 1024;
	float RAM_Used_systemunsafe_f = (float)RAM_Used_systemunsafe_u / 1024 / 1024;
	float RAM_Used_all_f = RAM_Used_application_f + RAM_Used_applet_f + RAM_Used_system_f + RAM_Used_systemunsafe_f;
	snprintf(RAM_Hz_c, sizeof GPU_Hz_c, "RAM: Frequency: %.1f MHz", (float)RAM_Hz / 1000000);
	snprintf(RAM_all_c, sizeof RAM_all_c, "RAM: %.0f/%.0fMB", RAM_Used_all_f, RAM_Total_all_f);
	
	
	
	snprintf(SoCPCB_temperature_c, sizeof SoCPCB_temperature_c, "SoC: %2.2f C PCB: %2.2f C", (float)SoC_temperaturemiliC / 1000, (float)PCB_temperaturemiliC / 1000);
	snprintf(skin_temperature_c, sizeof skin_temperature_c, "Skin: %2.2f \u00B0C", (float)skin_temperaturemiliC / 1000);
	snprintf(Rotation_SpeedLevel_c, sizeof Rotation_SpeedLevel_c, "Fan: %2.2f%s", Rotation_SpeedLevel_f * 100, "%");

}

void CPU_Meter::initServices() {
	if (R_SUCCEEDED(smInitialize())) {
		
		if (hosversionAtLeast(8,0,0)) clkrstCheck = clkrstInitialize();
		else pcvCheck = pcvInitialize();
		
		tsCheck = tsInitialize();
		if (hosversionAtLeast(5,0,0)) tcCheck = tcInitialize();
		if (R_SUCCEEDED(fanInitialize())) {
			if (hosversionAtLeast(7,0,0)) fanCheck = fanOpenController(&g_ICon, 0x3D000001);
			else fanCheck = fanOpenController(&g_ICon, 1);
		}
		if (R_SUCCEEDED(nvInitialize())) nvCheck = nvOpen(&fd, "/dev/nvhost-ctrl-gpu");

		smExit();
	}
	Hinted = envIsSyscallHinted(0x6F);
}

void ExitCpuMeterServices() {
	if (R_SUCCEEDED(clkrstCheck)) {
		clkrstExit();
	}else if(R_SUCCEEDED(pcvCheck)) {
		pcvExit();
	}
	if (R_SUCCEEDED(tsCheck)) {
		tsExit();
	}
	if (R_SUCCEEDED(tcCheck)) {
		tcExit();
	}
	if (R_SUCCEEDED(fanCheck)) {
		fanControllerClose(&g_ICon);
		fanExit();
	}

	if (R_SUCCEEDED(nvCheck)) {
		nvClose(fd);
		nvExit();
	}
}

CPU_Meter::~CPU_Meter(){
	threadexit2 = true;
	threadWaitForExit(&t0);
	threadWaitForExit(&t1);
	threadWaitForExit(&t2);
	threadWaitForExit(&t4);
	threadClose(&t0);
	threadClose(&t1);
	threadClose(&t2);
	threadClose(&t4);
	
	printf("CPU Meter Deinit\n");
	
}

