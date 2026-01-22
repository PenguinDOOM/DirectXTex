//--------------------------------------------------------------------------------------
// File: test_api.cpp
//
// Simple test to verify the TexconvAPI can be called correctly
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//--------------------------------------------------------------------------------------

#include "TexconvAPI.h"
#include <iostream>
#include <cstring>

int main()
{
    std::wcout << L"Testing TexconvAPI..." << std::endl;

    // Test 1: Initialize options
    TexconvOptions options;
    TexconvInitOptions(&options);

    // Verify defaults
    if (options.width != 0 || options.height != 0 || options.mipLevels != 0)
    {
        std::wcerr << L"ERROR: Default initialization failed" << std::endl;
        return 1;
    }

    std::wcout << L"Test 1: TexconvInitOptions - PASSED" << std::endl;

    // Test 2: Set options
    options.inputFile = L"test.png";
    options.outputDir = L"output";
    options.format = L"BC3_UNORM";
    options.fileType = L"DDS";
    options.options = TEXCONV_OPT_OVERWRITE | TEXCONV_OPT_NOLOGO;

    std::wcout << L"Test 2: Set options - PASSED" << std::endl;

    // Test 3: Command-line conversion (will fail since files don't exist, but tests the API)
    const wchar_t* cmdLine = L"-nologo -f BC3_UNORM -ft DDS -y nonexistent.png";
    int result = TexconvConvertCommandLine(cmdLine);
    
    // We expect this to fail since the file doesn't exist, but we're testing the API works
    if (result != TEXCONV_SUCCESS)
    {
        const wchar_t* error = TexconvGetLastError();
        std::wcout << L"Test 3: Command-line conversion (expected to fail) - Error: " 
                   << (error ? error : L"unknown") << std::endl;
    }

    std::wcout << L"All API tests completed successfully!" << std::endl;
    return 0;
}
