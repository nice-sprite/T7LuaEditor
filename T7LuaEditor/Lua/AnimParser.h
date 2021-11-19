#pragma once
#include <simdjson.h>
#include <regex>
#include <iostream>
#include <string>
#include <fmt/format.h>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;
namespace json = simdjson;
/// <summary>
/// porting this is gonna be insane
/// </summary>
class LuiAnimParser
{
    fs::path directory; // the root directory to process lua files from
public: 
    LuiAnimParser(std::wstring _dir) : directory{_dir}
    {
        AllocConsole();
        freopen( "CONOUT$", "w", stdout );
        std::ios::sync_with_stdio( false );
        // walk the directory and find lua files
        for (auto& p : fs::recursive_directory_iterator( _dir ))
        {
            if (p.path().extension() == ".lua")
                std::cout << p.path() << '\n';
        }
        std::cout << std::endl;
    }
};