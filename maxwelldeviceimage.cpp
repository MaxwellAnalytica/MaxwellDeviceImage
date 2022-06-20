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
#include <chrono>
#include <filesystem>
#include <Windows.h>

using namespace std;
using namespace chrono;
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

int32_t MaxwellDeviceImage::explore()
{
	int32_t size = 8192;
	char szBuffer[8192] = { 0x00 };
	bool success = true;
	explore_->explore(szBuffer, &size);
	try
	{
		device_object_ = wdjson::parse(std::string(szBuffer));
		if (!device_object_.is_null())
			std::cout << device_object_.dump(4) << std::endl;
	}
	catch (const std::exception&)
	{
		return -1;
	}
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

int32_t MaxwellDeviceImage::searchTargetDevice(int32_t number, int32_t index)
{
	if (isNull())
		return -1;
	//
	auto disk_array = device_object_.at("DeviceArray").get<wdjson::array_t>();
	for (const auto& disk : disk_array)
	{
		auto disk_number = disk.at("Index").get<int32_t>();
		if (disk_number == number)
		{
			if (index == -1)
			{
				clone_info_.source.offset = 0;
				clone_info_.source.size = disk.at("Size").get<int64_t>();
				clone_info_.source.bytesOfSector = disk.at("BytesPerSector").get<int32_t>();
				break;
			}
			else
			{
				auto volume_array = disk.at("VolumeArray").get<wdjson::array_t>();
				for (const auto& volume : volume_array)
				{
					auto volume_index = volume.at("Index").get<int32_t>();
					if (volume_index == index)
					{
						clone_info_.source.size = volume.at("Size").get<int64_t>();
						clone_info_.source.offset = volume.at("StartingOffset").get<int64_t>(); // 扇区
						clone_info_.source.bytesOfSector = disk.at("BytesPerSector").get<int32_t>();
						break;
					}
				}
			}
		}
	}
	//
	return 0;
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
		int32_t number = -1, index = -1;
		int32_t count = sscanf(device_name.c_str(), "disk%ds%d", &number, &index);
		if (count == 1)
		{
			clone_info_.source.device_name = "\\\\.\\PHYSICALDRIVE" + std::to_string(number);
			clone_info_.type = ImageType::IMG_Disk;
		}
		else if (count == 2)
		{
			clone_info_.source.device_name = "\\\\.\\PHYSICALDRIVE" + std::to_string(number);
			clone_info_.type = ImageType::IMG_Partition;
		}
		//
		if (searchTargetDevice(number, index) < 0)
		{
			std::cout << "error: " << device_name << ": searchTargetDevice" << std::endl;
			return;
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
	std::string target_device = clone_info_.target.dest_dir + "/" + WdUtil::getTimeStamp(1) + ".mdk";
	int32_t target_fd = OPEN(target_device.c_str(), IOFlag::WD_WRONLY | IOFlag::WD_CREAT | IOFlag::WD_BINARY, 1);
	if (target_fd < 0)
	{
		return -1;
	}
	std::string source_device = clone_info_.source.device_name;
	int32_t source_fd = OPEN(source_device.c_str(), IOFlag::WD_RDONLY | IOFlag::WD_BINARY, 1);
	if (source_fd < 0)
	{
		return -1;
	}
	const int32_t ConstBufferSize = 4 * 1024 * 1024;
	char* lpBuffer = new char[ConstBufferSize];
	int32_t ConstBytesOfSector = clone_info_.source.bytesOfSector;
	int64_t offset = clone_info_.source.offset / ConstBytesOfSector;
	int64_t total = offset + clone_info_.source.size / ConstBytesOfSector;
	//
	int64_t last_offset = offset;
	auto start = system_clock::now().time_since_epoch();
	while (offset < total)
	{
		SEEK(source_fd, offset * ConstBytesOfSector, SEEK_SET);
		auto count = READ(source_fd, lpBuffer, ConstBufferSize);
		if (count < 0)
		{
			offset += (ConstBufferSize / ConstBytesOfSector);
			continue;
		}
		//
		WRITE(target_fd, lpBuffer, count);
		offset += (count / ConstBytesOfSector);
		auto end = system_clock::now().time_since_epoch();
		auto elapsed = duration_cast<milliseconds>(end - start).count();
		if (elapsed > 1000)
		{
			double rate = double((offset - last_offset) * ConstBytesOfSector * 1000.0 / (1024.0 * 1024.0 * elapsed));
			std::cout << "[-] rate:" << rate << std::endl;
			last_offset = offset;
			start = end;
		}
	}
	//
	delete[] lpBuffer;
	//
	CLOSE(target_fd);
	CLOSE(source_fd);
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