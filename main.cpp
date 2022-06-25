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
                                    "***  Maxwell Command Line Prompt v3.2.8\n" \
                                    "***  Copyright (c)2015-2021 Maxwell Analytica, All rights reserved\n" \
                                    "**********************************************************************\n";

static void usage()
{
    std::cout << "Usage:\n";
    //
    std::cout << "\thelp: help or h\n";
    std::cout << "\tversion: version or v\n";
    std::cout << "\tclear screen: clear or cls\n";
    std::cout << "\texplore devices: ls\n";
    std::cout << "\tclone disk: clone <disk_name> <target_dir>, such as: `clone disk0 G:/images`\n";
    std::cout << "\tclone partition: clone <partition_name> <target_dir>, such as: `clone disk0s2 G:/images`\n";
    std::cout << "\tclone volume: clone <drive_name> <target_dir>, such as: `clone D: G:/images`\n";
    std::cout << "\tquit: quit or q\n";
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
        if (options[0] == "quit" || options[0] == "q")
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
        else if (options[0] == "help" || options[0] == "h")
        {
            usage();
            continue;
        }
        else if (options[0] == "ls")
        {
            if (app.explore() < 0)
                std::cout << "error: failed to explore devices" << std::endl;
        }
        else if (options[0] == "clone")
        {
            if (app.isNull())
            {
                if (app.explore() < 0)
                {
                    std::cout << "error: failed to explore devices" << std::endl;
                    return -1;
                }  
            }
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
