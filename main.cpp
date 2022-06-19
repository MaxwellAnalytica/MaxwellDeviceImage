/**********************************************************************
*
* Copyright (C)2015-2022 Maxwell Analytica. All rights reserved.
*
* @file main.h
* @brief
* @details
* @author wd
* @version 1.0.0
* @date 2021-10-09 22:41:27.492
*
**********************************************************************/
#include <iostream>
#include "maxwelldeviceimage.h"

const int32_t ConstSize = 256;

const std::string ConstWdCliTitle = "**********************************************************************\n" \
                                    "***  Maxwell Command Line Prompt v3.1.8\n" \
                                    "***  Copyright (c)2015-2021 Maxwell Analytica, All rights reserved\n" \
                                    "**********************************************************************\n";

static void usage()
{
    std::cout << "Usage:\n";
    //
    std::cout << "\t1, whole disk: clone <disk_name> <dest_dir>, such as: `clone disk0 G:/images`\n";
    std::cout << "\t2, whole partition: clone <partition_name> <dest_dir>, such as: `clone disk0s2 G:/images`\n";
    std::cout << "\t3, whole volume: clone <drive_name> <dest_dir>, such as: `clone D: G:/images`\n";
    //
    std::cout << "\n";
}

int main()
{
    std::cout << ConstWdCliTitle << std::endl;
    //
    MaxwellDeviceImage app;
    app.start();
    //
    char lpBuffer[ConstSize] = { 0x00 };
    while (true)
    {
        // prompt
        if (!app.isPrompt())
            std::cout << ConstPrefix;
        //
        std::string line;
        std::getline(std::cin, line, '\n');
        if (line.size() == 0)
            continue;
        //
        auto options = MaxwellDeviceImage::split(line, ' ');
        if (options[0] == "quit" || options[0] == "q" || options[0] == "exit")
        {
            std::cout << "bye" << std::endl;
            break;
        }
        else if (options[0] == "clear" || options[0] == "cls")
        {
            system("cls");
            std::cout << ConstWdCliTitle << std::endl;
            continue;
        }
        else if (options[0] == "version" || options[0] == "v")
        {
            std::cout << "v3.1.8" << std::endl;
            continue;
        }
        else if (options[0] == "help" || options[0] == "h" || options[0] == "?")
        {
            usage();
            continue;
        }
        else if (options[0] == "ls")
        {
            int32_t size = 8192;
            char szBuffer[8192] = { 0x00 };
            bool success = true;
            app.explore_->explore(szBuffer, &size);
            try
            {
                app.device_object_ = wdjson::parse(std::string(szBuffer));
                if (!app.device_object_.is_null())
                    std::cout << app.device_object_.dump(4) << std::endl;
            }
            catch (const std::exception&)
            {
                success = false;
            }
            if (!success)
                std::cout << "error: failed to explore devices" << std::endl;
        }
        else if (options[0] == "clone")
        {
            app.device_image(options);
        }
        else
        {
            std::cout << "error: " << options[0] << ": command not found" << std::endl;
        }
        //
        fflush(stdout);
    }
    //
    return 0;
}
