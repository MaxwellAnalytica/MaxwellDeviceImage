/**********************************************************************
*
* Copyright (C)2015-2022 Maxwell Analytica. All rights reserved.
*
* @file maxwelldeviceimage.cpp
* @brief
* @details
* @author wd
* @version 1.0.0
* @date 2021-10-09 22:41:27.492
*
**********************************************************************/
#include "maxwelldeviceimage.h"
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <Windows.h>

namespace fs = std::filesystem;

MaxwellDeviceImage::MaxwellDeviceImage()
{
	stop_ = true;
	prompt_ = false;
}

MaxwellDeviceImage::~MaxwellDeviceImage()
{
	stop_ = true;
}

int32_t MaxwellDeviceImage::start()
{
	explore_ = CreateExplorer();
	if (explore_ == nullptr)
		return -1;
	//
	stop_ = false;
	//
	//
	return 0;
}

bool MaxwellDeviceImage::isPrompt() const
{
	return prompt_;
}

bool MaxwellDeviceImage::isNull() const
{
	return device_object_.is_null();
}

void MaxwellDeviceImage::device_image(std::vector<std::string>& options)
{
	clone_info_.target.dest_dir = options[2];
	fs::path dest_path(clone_info_.target.dest_dir);
	try
	{
		if (!fs::exists(dest_path))
		{
			if (!fs::create_directories(dest_path))
			{
				std::cout << "error: " << clone_info_.target.dest_dir << " create directories failure" << std::endl;
				return;
			}
		}
		auto tokens = MaxwellDeviceImage::split(clone_info_.target.dest_dir, '/');
		std::wstring drive = WdUtil::s2ws(tokens[0]);
		_ULARGE_INTEGER lpFreeBytesAvailableToCaller, lpTotalNumberOfBytes, lpTotalNumberOfFreeBytes;
		if (!GetDiskFreeSpaceEx(drive.c_str(), &lpFreeBytesAvailableToCaller, &lpTotalNumberOfBytes, &lpTotalNumberOfFreeBytes))
		{
			std::cout << "error: " << clone_info_.target.dest_dir << " open failed" << std::endl;
			return;
		}
		//
		clone_info_.target.availabled = (int64_t)lpFreeBytesAvailableToCaller.QuadPart;
	}
	catch (const std::exception& e)
	{
		std::cout << "error: " << clone_info_.target.dest_dir << ": directory not exists" << std::endl;
		return;
	}
	//
	prompt_ = true;
	std::string device_name = options[1];
	std::transform(device_name.begin(), device_name.end(), device_name.begin(), ::tolower);
	if (memcmp(device_name.c_str(), "disk", 4) == 0)
	{
		int32_t number = 0, index = 0;
		int32_t count = sscanf(device_name.c_str(), "disk%ds%d", &number, &index);
		if (count == 1)
		{
			clone_info_.source.device_name = "\\\\.\\PHYSICALDRIVE" + std::to_string(number);
			clone_info_.source.offset = 0;
			clone_info_.type = ImageType::IMG_Disk;
			std::cout << std::endl;
		}
		else if (count == 2)
		{
			clone_info_.source.device_name = "\\\\.\\PHYSICALDRIVE" + std::to_string(number);
			clone_info_.source.offset = 0;
			clone_info_.type = ImageType::IMG_Partition;
			//
			std::cout << std::endl;
		}
	}
	else
	{
		clone_info_.type = ImageType::IMG_Volume;
		std::wstring drive = WdUtil::s2ws(device_name);
		_ULARGE_INTEGER lpFreeBytesAvailableToCaller, lpTotalNumberOfBytes, lpTotalNumberOfFreeBytes;
		if (GetDiskFreeSpaceEx(drive.c_str(), &lpFreeBytesAvailableToCaller, &lpTotalNumberOfBytes, &lpTotalNumberOfFreeBytes))
		{
			std::cout << "error: " << device_name << ": open failed" << std::endl;
			return;
		}
		clone_info_.source.offset = 0;
		clone_info_.source.device_name = device_name;
		clone_info_.source.size = (int64_t)lpTotalNumberOfBytes.QuadPart;
	}
	// 判断空间是否足够
	if (clone_info_.source.size > clone_info_.target.availabled)
	{
		std::cout << "error: " << clone_info_.target.dest_dir << " size not enough, at least " << clone_info_.source.size/(1024*1024*1024) << "(GB)" << std::endl;
		return;
	}
	//
	future_result_ = std::async([this] { return this->imageCallback(); });
	//
	future_result_.wait();
}

int32_t MaxwellDeviceImage::imageCallback()
{
	while (!stop_)
	{
		std::string cmd;
		prompt_ = false;
		std::cout << ConstPrefix;
	}
	//
	return 0;
}

std::vector<std::string> MaxwellDeviceImage::split(const std::string& str, char tag)
{
	std::vector<std::string> strlist;
	std::string subStr;
	for (size_t i = 0; i < str.length(); i++)
	{
		if (tag == str[i]) //完成一次切割
		{
			if (!subStr.empty())
			{
				strlist.push_back(subStr);
				subStr.clear();
			}
		}
		else //将i位置的字符放入子串
		{
			subStr.push_back(str[i]);
		}
	}

	if (!subStr.empty()) //剩余的子串作为最后的子字符串
	{
		strlist.push_back(subStr);
	}

	return strlist;
}