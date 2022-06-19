/**********************************************************************
*
* Copyright (C)2015-2022 Maxwell Analytica. All rights reserved.
*
* @file iexplore.h
* @brief
* @details
* @author wd
* @version 1.0.0
* @date 2021-10-09 22:41:27.492
*
**********************************************************************/
#pragma once

#ifdef _DLL_EXPORTS
#define EXPORTL_API _declspec(dllexport)
#else
#define EXPORTL_API _declspec(dllimport)
#endif

#include <stdint.h>

enum ExploreResult
{
	ER_Timeout		= -3,
	ER_NotEnough	= -2,
	ER_Failure		= -1,
	ER_Success
};

class IExplore
{
public:
	virtual int32_t explore(void* deviceBuffer, int32_t* size) = 0;

	virtual int32_t verifyProtectionStatus(char* driverLetter) = 0;
};

extern "C" EXPORTL_API IExplore *CreateExplorer();