// Fill out your copyright notice in the Description page of Project Settings.


#include "HardwareDetails.h"
#include "HardwareInfo.h"

FString UHardwareDetails::GetCPUBrand()
{
	if (PLATFORM_WINDOWS) {
		return FWindowsPlatformMisc::GetCPUBrand();
	}
}

FString UHardwareDetails::GetGPUBrand()
{
	if (PLATFORM_WINDOWS) {
		return FWindowsPlatformMisc::GetPrimaryGPUBrand();
	}
}

FString UHardwareDetails::GetRHI()
{
	return FHardwareInfo::GetHardwareInfo(NAME_RHI);
}

int32 UHardwareDetails::GetCPUCores()
{
	if (PLATFORM_WINDOWS) {
		return FWindowsPlatformMisc::NumberOfCores();
	}
}

int32 UHardwareDetails::GetCPUThreads()
{
	if (PLATFORM_WINDOWS) {
		return FWindowsPlatformMisc::NumberOfCoresIncludingHyperthreads();
	}
}

int32 UHardwareDetails::GetRAMCapacity()
{
	return FGenericPlatformMemory::GetPhysicalGBRam();
}
