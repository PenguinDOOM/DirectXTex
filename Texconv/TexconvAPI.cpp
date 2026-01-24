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
#include <sal.h>
#include <objbase.h>
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

// Helper function to convert narrow string to wide string
static std::wstring NarrowToWide(const char* narrow)
{
    if (!narrow || !narrow[0])
        return std::wstring();

    // Get required buffer size
    int size = MultiByteToWideChar(CP_UTF8, 0, narrow, -1, nullptr, 0);
    if (size <= 0)
        return L"[Error converting string]";

    std::wstring wide(size - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, narrow, -1, &wide[0], size);
    return wide;
}

// Helper function to convert exception to wide string error message
static void SetLastErrorFromException(const std::exception& e)
{
    std::wstring error = L"Exception: ";
    error += NarrowToWide(e.what());
    SetLastError(error.c_str());
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

    // Note: COM initialization is now handled gracefully by wmain
    // wmain will accept COM already being initialized in a different threading model

    try
    {
        // Build argument list
        // CRITICAL: Build all strings in argStorage first, then create argv pointers
        // This prevents pointer invalidation when argStorage reallocates
        std::vector<std::wstring> argStorage;

        // Program name
        argStorage.push_back(L"texconv");

        // Output directory
        if (options->outputDir && options->outputDir[0])
        {
            argStorage.push_back(L"-o");
            argStorage.push_back(options->outputDir);
        }

        // Width
        if (options->width > 0)
        {
            argStorage.push_back(L"-w");
            argStorage.push_back(std::to_wstring(options->width));
        }

        // Height
        if (options->height > 0)
        {
            argStorage.push_back(L"-h");
            argStorage.push_back(std::to_wstring(options->height));
        }

        // Mip levels
        if (options->mipLevels > 0)
        {
            argStorage.push_back(L"-m");
            argStorage.push_back(std::to_wstring(options->mipLevels));
        }

        // Format
        if (options->format && options->format[0])
        {
            argStorage.push_back(L"-f");
            argStorage.push_back(options->format);
        }

        // File type
        if (options->fileType && options->fileType[0])
        {
            argStorage.push_back(L"-ft");
            argStorage.push_back(options->fileType);
        }

        // Filter
        if (options->filter && options->filter[0])
        {
            argStorage.push_back(L"-if");
            argStorage.push_back(options->filter);
        }

        // sRGB input
        if (options->srgbIn && options->srgbIn[0])
        {
            argStorage.push_back(L"-srgbi");
            argStorage.push_back(options->srgbIn);
        }

        // sRGB output
        if (options->srgbOut && options->srgbOut[0])
        {
            argStorage.push_back(L"-srgbo");
            argStorage.push_back(options->srgbOut);
        }

        // Prefix
        if (options->prefix && options->prefix[0])
        {
            argStorage.push_back(L"-px");
            argStorage.push_back(options->prefix);
        }

        // Suffix
        if (options->suffix && options->suffix[0])
        {
            argStorage.push_back(L"-sx");
            argStorage.push_back(options->suffix);
        }

        // Feature level
        if (options->featureLevel && options->featureLevel[0])
        {
            argStorage.push_back(L"-fl");
            argStorage.push_back(options->featureLevel);
        }

        // GPU adapter
        if (options->gpuAdapter >= 0)
        {
            argStorage.push_back(L"-gpu");
            argStorage.push_back(std::to_wstring(options->gpuAdapter));
        }

        // WIC quality
        if (options->wicQuality >= 0.0f)
        {
            argStorage.push_back(L"-wicq");
            argStorage.push_back(std::to_wstring(options->wicQuality));
        }

        // Alpha threshold
        if (options->alphaThreshold >= 0.0f && options->alphaThreshold != 0.5f)
        {
            argStorage.push_back(L"-at");
            argStorage.push_back(std::to_wstring(options->alphaThreshold));
        }

        // Compression mode
        if (options->compressionMode && options->compressionMode[0])
        {
            argStorage.push_back(L"-bc");
            argStorage.push_back(options->compressionMode);
        }

        // Option flags
        if (options->options & TEXCONV_OPT_PREMUL_ALPHA)
        {
            argStorage.push_back(L"-pmalpha");
        }

        if (options->options & TEXCONV_OPT_DEMUL_ALPHA)
        {
            argStorage.push_back(L"-alpha");
        }

        if (options->options & TEXCONV_OPT_SEPALPHA)
        {
            argStorage.push_back(L"-sepalpha");
        }

        if (options->options & TEXCONV_OPT_NO_ALPHA)
        {
            argStorage.push_back(L"-noalpha");
        }

        if (options->options & TEXCONV_OPT_HFLIP)
        {
            argStorage.push_back(L"-hflip");
        }

        if (options->options & TEXCONV_OPT_VFLIP)
        {
            argStorage.push_back(L"-vflip");
        }

        if (options->options & TEXCONV_OPT_FORCE_SRGB)
        {
            argStorage.push_back(L"-srgb");
        }

        if (options->options & TEXCONV_OPT_FORCE_LINEAR)
        {
            argStorage.push_back(L"-linear");
        }

        if (options->options & TEXCONV_OPT_USE_DX10)
        {
            argStorage.push_back(L"-dx10");
        }

        if (options->options & TEXCONV_OPT_USE_DX9)
        {
            argStorage.push_back(L"-dx9");
        }

        if (options->options & TEXCONV_OPT_FIT_POWEROF2)
        {
            argStorage.push_back(L"-pow2");
        }

        if (options->options & TEXCONV_OPT_INVERT_Y)
        {
            argStorage.push_back(L"-inverty");
        }

        if (options->options & TEXCONV_OPT_RECONSTRUCT_Z)
        {
            argStorage.push_back(L"-reconstructz");
        }

        if (options->options & TEXCONV_OPT_OVERWRITE)
        {
            argStorage.push_back(L"-y");
        }

        if (options->options & TEXCONV_OPT_NOLOGO)
        {
            argStorage.push_back(L"-nologo");
        }

        // Note: outputFile is not used as texconv doesn't support specifying
        // individual output file names via command line. Output directory (-o)
        // is used instead, and the output filename is derived from input filename.

        // Input file (required)
        argStorage.push_back(options->inputFile);

        // Now build argv pointer array after all strings are in argStorage
        // This ensures pointers remain valid
        std::vector<wchar_t*> argv;
        argv.reserve(argStorage.size());
        for (auto& arg : argStorage)
        {
            argv.push_back(const_cast<wchar_t*>(arg.c_str()));
        }

        // Build command line string for debugging
        std::wstring cmdLineDebug = L"texconv";
        for (size_t i = 1; i < argv.size(); ++i)
        {
            cmdLineDebug += L" ";
            // Add quotes if the argument contains spaces
            std::wstring arg = argv[i];
            if (arg.find(L' ') != std::wstring::npos)
            {
                cmdLineDebug += L"\"";
                cmdLineDebug += arg;
                cmdLineDebug += L"\"";
            }
            else
            {
                cmdLineDebug += arg;
            }
        }

        // Call wmain with the constructed arguments
        int result = wmain(static_cast<int>(argv.size()), argv.data());

        // Note: wmain initializes COM on first call and leaves it initialized
        // This is intentional to support repeated calls on the same thread

        if (result == 0)
        {
            SetLastError(L"");
            return TEXCONV_SUCCESS;
        }
        else
        {
            // Provide detailed error message with the exit code and actual command line
            std::wstring errorMsg = L"Texture conversion failed with exit code ";
            errorMsg += std::to_wstring(result);
            errorMsg += L".\nCommand line: ";
            errorMsg += cmdLineDebug;
            SetLastError(errorMsg.c_str());
            return TEXCONV_ERROR_PROCESS_FAILED;
        }
    }
    catch (const std::exception& e)
    {
        SetLastErrorFromException(e);
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

    // Note: COM initialization is now handled gracefully by wmain
    // wmain will accept COM already being initialized in a different threading model

    try
    {
        // Parse command line into arguments
        // CRITICAL: Build all strings in argStorage first, then create argv pointers
        // This prevents pointer invalidation when argStorage reallocates
        std::vector<std::wstring> argStorage;

        // Add program name
        argStorage.push_back(L"texconv");

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
        }

        // Now build argv pointer array after all strings are in argStorage
        // This ensures pointers remain valid
        std::vector<wchar_t*> argv;
        argv.reserve(argStorage.size());
        for (auto& arg : argStorage)
        {
            argv.push_back(const_cast<wchar_t*>(arg.c_str()));
        }

        // Build command line string for debugging
        std::wstring cmdLineDebug = L"texconv";
        for (size_t i = 1; i < argv.size(); ++i)
        {
            cmdLineDebug += L" ";
            // Add quotes if the argument contains spaces
            std::wstring arg = argv[i];
            if (arg.find(L' ') != std::wstring::npos)
            {
                cmdLineDebug += L"\"";
                cmdLineDebug += arg;
                cmdLineDebug += L"\"";
            }
            else
            {
                cmdLineDebug += arg;
            }
        }

        // Call wmain
        int result = wmain(static_cast<int>(argv.size()), argv.data());

        // Note: wmain initializes COM on first call and leaves it initialized
        // This is intentional to support repeated calls on the same thread

        if (result == 0)
        {
            SetLastError(L"");
            return TEXCONV_SUCCESS;
        }
        else
        {
            // Provide detailed error message with the exit code and actual command line
            std::wstring errorMsg = L"Texture conversion failed with exit code ";
            errorMsg += std::to_wstring(result);
            errorMsg += L".\nCommand line: ";
            errorMsg += cmdLineDebug;
            SetLastError(errorMsg.c_str());
            return TEXCONV_ERROR_PROCESS_FAILED;
        }
    }
    catch (const std::exception& e)
    {
        SetLastErrorFromException(e);
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
