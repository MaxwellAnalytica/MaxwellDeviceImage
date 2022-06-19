/**********************************************************************
*
* Copyright (C)2015-2022 Maxwell Analytica. All rights reserved.
*
* @file maxwelldeviceimage.h
* @brief
* @details
* @author wd
* @version 1.0.0
* @date 2021-10-09 22:41:27.492
*
**********************************************************************/
#pragma once

#include <stdint.h>
#include <string>
#include <future>
#include <atomic>
#include <iostream>
#include "json.hpp"
#include "wdio.h"
#include "wdutil.h"
#include "safequeue.h"
#include "iexplore.h"

using namespace wd;
using namespace std;

using wdjson = nlohmann::json;

const std::string ConstPrefix = "[maxwell@device-image ~]# ";

enum class ImageType
{
	IMG_Disk,
	IMG_Partition,
	IMG_Volume
};

struct CloneInfo
{
	ImageType		type;
	struct {
		int64_t		size;
		int64_t		offset;
		std::string device_name;
	} source;
	struct {
		std::string dest_dir;
		int64_t		availabled;
	} target;
};

class MaxwellDeviceImage
{
public:
	MaxwellDeviceImage();
	~MaxwellDeviceImage();

	int32_t start();

	bool isPrompt() const;

	bool isNull() const;

	void device_image(std::vector<std::string>& options);

	static std::vector<std::string> split(const std::string& str, char tag);

protected:
	int32_t imageCallback();

public:
	IExplore* explore_;
	wdjson device_object_;

	CloneInfo clone_info_;
	//
	std::atomic_bool stop_;
	std::atomic_bool prompt_;
	//
	std::future<int32_t> future_result_;
};

