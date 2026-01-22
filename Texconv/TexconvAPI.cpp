//--------------------------------------------------------------------------------------
// File: TexconvAPI.cpp
//
// DirectX Texture Converter DLL API Implementation
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248926
//--------------------------------------------------------------------------------------

#define TEXCONV_DLL_EXPORTS
#include "TexconvAPI.h"

#include <windows.h>
#include <string>
#include <vector>
#include <mutex>

// Thread-local storage for error messages
static std::mutex g_errorMutex;
static std::wstring g_lastError;

extern int __cdecl wmain(_In_ int argc, _In_z_count_(argc) wchar_t* argv[]);

// Helper function to set the last error message
static void SetLastError(const wchar_t* error)
{
    std::lock_guard<std::mutex> lock(g_errorMutex);
    g_lastError = error ? error : L"";
}

// Initialize default options
TEXCONV_API void TexconvInitOptions(TexconvOptions* options)
{
    if (!options)
        return;

    memset(options, 0, sizeof(TexconvOptions));
    
    // Set defaults
    options->width = 0;
    options->height = 0;
    options->mipLevels = 0;
    options->wicQuality = -1.0f;
    options->alphaThreshold = 0.5f;
    options->alphaWeight = 1.0f;
    options->gpuAdapter = -1;
    options->options = 0;
}

// Convert a single texture file
TEXCONV_API int TexconvConvertFile(const TexconvOptions* options)
{
    if (!options || !options->inputFile)
    {
        SetLastError(L"Invalid arguments: options or inputFile is NULL");
        return TEXCONV_ERROR_INVALID_ARGUMENTS;
    }

    try
    {
        // Build argument list
        std::vector<std::wstring> argStorage;
        std::vector<wchar_t*> argv;

        // Program name
        argStorage.push_back(L"texconv");
        argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));

        // Output directory
        if (options->outputDir && options->outputDir[0])
        {
            argStorage.push_back(L"-o");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(options->outputDir);
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // Width
        if (options->width > 0)
        {
            argStorage.push_back(L"-w");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(std::to_wstring(options->width));
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // Height
        if (options->height > 0)
        {
            argStorage.push_back(L"-h");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(std::to_wstring(options->height));
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // Mip levels
        if (options->mipLevels > 0)
        {
            argStorage.push_back(L"-m");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(std::to_wstring(options->mipLevels));
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // Format
        if (options->format && options->format[0])
        {
            argStorage.push_back(L"-f");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(options->format);
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // File type
        if (options->fileType && options->fileType[0])
        {
            argStorage.push_back(L"-ft");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(options->fileType);
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // Filter
        if (options->filter && options->filter[0])
        {
            argStorage.push_back(L"-if");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(options->filter);
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // sRGB input
        if (options->srgbIn && options->srgbIn[0])
        {
            argStorage.push_back(L"-srgbi");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(options->srgbIn);
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // sRGB output
        if (options->srgbOut && options->srgbOut[0])
        {
            argStorage.push_back(L"-srgbo");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(options->srgbOut);
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // Prefix
        if (options->prefix && options->prefix[0])
        {
            argStorage.push_back(L"-px");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(options->prefix);
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // Suffix
        if (options->suffix && options->suffix[0])
        {
            argStorage.push_back(L"-sx");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(options->suffix);
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // Feature level
        if (options->featureLevel && options->featureLevel[0])
        {
            argStorage.push_back(L"-fl");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(options->featureLevel);
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // GPU adapter
        if (options->gpuAdapter >= 0)
        {
            argStorage.push_back(L"-gpu");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(std::to_wstring(options->gpuAdapter));
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // WIC quality
        if (options->wicQuality >= 0.0f)
        {
            argStorage.push_back(L"-wicq");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(std::to_wstring(options->wicQuality));
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // Alpha threshold
        if (options->alphaThreshold >= 0.0f && options->alphaThreshold != 0.5f)
        {
            argStorage.push_back(L"-at");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(std::to_wstring(options->alphaThreshold));
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // Compression mode
        if (options->compressionMode && options->compressionMode[0])
        {
            argStorage.push_back(L"-bc");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(options->compressionMode);
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // Option flags
        if (options->options & TEXCONV_OPT_PREMUL_ALPHA)
        {
            argStorage.push_back(L"-pmalpha");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        if (options->options & TEXCONV_OPT_DEMUL_ALPHA)
        {
            argStorage.push_back(L"-alpha");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        if (options->options & TEXCONV_OPT_SEPALPHA)
        {
            argStorage.push_back(L"-sepalpha");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        if (options->options & TEXCONV_OPT_NO_ALPHA)
        {
            argStorage.push_back(L"-noalpha");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        if (options->options & TEXCONV_OPT_HFLIP)
        {
            argStorage.push_back(L"-hflip");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        if (options->options & TEXCONV_OPT_VFLIP)
        {
            argStorage.push_back(L"-vflip");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        if (options->options & TEXCONV_OPT_FORCE_SRGB)
        {
            argStorage.push_back(L"-srgb");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        if (options->options & TEXCONV_OPT_FORCE_LINEAR)
        {
            argStorage.push_back(L"-linear");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        if (options->options & TEXCONV_OPT_BC_DITHER)
        {
            argStorage.push_back(L"-dx10");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        if (options->options & TEXCONV_OPT_USE_DX10)
        {
            argStorage.push_back(L"-dx10");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        if (options->options & TEXCONV_OPT_USE_DX9)
        {
            argStorage.push_back(L"-dx9");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        if (options->options & TEXCONV_OPT_FIT_POWEROF2)
        {
            argStorage.push_back(L"-pow2");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        if (options->options & TEXCONV_OPT_INVERT_Y)
        {
            argStorage.push_back(L"-inverty");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        if (options->options & TEXCONV_OPT_RECONSTRUCT_Z)
        {
            argStorage.push_back(L"-reconstructz");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        if (options->options & TEXCONV_OPT_OVERWRITE)
        {
            argStorage.push_back(L"-y");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        if (options->options & TEXCONV_OPT_NOLOGO)
        {
            argStorage.push_back(L"-nologo");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // Output file if specified (as suffix)
        if (options->outputFile && options->outputFile[0])
        {
            argStorage.push_back(L"-o");
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
            argStorage.push_back(options->outputFile);
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // Input file (required)
        argStorage.push_back(options->inputFile);
        argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));

        // Call wmain with the constructed arguments
        int result = wmain(static_cast<int>(argv.size()), argv.data());

        if (result == 0)
        {
            SetLastError(L"");
            return TEXCONV_SUCCESS;
        }
        else
        {
            SetLastError(L"Texture conversion failed");
            return TEXCONV_ERROR_PROCESS_FAILED;
        }
    }
    catch (const std::exception& e)
    {
        std::wstring error = L"Exception: ";
        // Convert narrow string to wide string
        const char* msg = e.what();
        for (size_t i = 0; msg[i] != '\0'; ++i)
        {
            error += static_cast<wchar_t>(msg[i]);
        }
        SetLastError(error.c_str());
        return TEXCONV_ERROR_PROCESS_FAILED;
    }
    catch (...)
    {
        SetLastError(L"Unknown exception occurred");
        return TEXCONV_ERROR_PROCESS_FAILED;
    }
}

// Convert a texture with a command-line style argument string
TEXCONV_API int TexconvConvertCommandLine(const wchar_t* commandLine)
{
    if (!commandLine || !commandLine[0])
    {
        SetLastError(L"Invalid command line");
        return TEXCONV_ERROR_INVALID_ARGUMENTS;
    }

    try
    {
        // Parse command line into arguments
        std::vector<std::wstring> argStorage;
        std::vector<wchar_t*> argv;

        // Add program name
        argStorage.push_back(L"texconv");
        argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));

        // Simple command-line parser
        std::wstring currentArg;
        bool inQuotes = false;

        for (size_t i = 0; commandLine[i] != L'\0'; ++i)
        {
            wchar_t c = commandLine[i];

            if (c == L'"')
            {
                inQuotes = !inQuotes;
            }
            else if (c == L' ' && !inQuotes)
            {
                if (!currentArg.empty())
                {
                    argStorage.push_back(currentArg);
                    argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
                    currentArg.clear();
                }
            }
            else
            {
                currentArg += c;
            }
        }

        if (!currentArg.empty())
        {
            argStorage.push_back(currentArg);
            argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));
        }

        // Call wmain
        int result = wmain(static_cast<int>(argv.size()), argv.data());

        if (result == 0)
        {
            SetLastError(L"");
            return TEXCONV_SUCCESS;
        }
        else
        {
            SetLastError(L"Texture conversion failed");
            return TEXCONV_ERROR_PROCESS_FAILED;
        }
    }
    catch (const std::exception& e)
    {
        std::wstring error = L"Exception: ";
        const char* msg = e.what();
        for (size_t i = 0; msg[i] != '\0'; ++i)
        {
            error += static_cast<wchar_t>(msg[i]);
        }
        SetLastError(error.c_str());
        return TEXCONV_ERROR_PROCESS_FAILED;
    }
    catch (...)
    {
        SetLastError(L"Unknown exception occurred");
        return TEXCONV_ERROR_PROCESS_FAILED;
    }
}

// Get the last error message
TEXCONV_API const wchar_t* TexconvGetLastError()
{
    std::lock_guard<std::mutex> lock(g_errorMutex);
    return g_lastError.c_str();
}
