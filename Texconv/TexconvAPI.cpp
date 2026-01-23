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

// Track COM initialization state per thread
static thread_local bool g_comInitializedByWrapper = false;

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

// Helper function to ensure COM is initialized in MULTITHREADED mode for wmain
// Returns true if ready to call wmain (COM initialized or will succeed in wmain)
static bool EnsureComForWmain()
{
    if (g_comInitializedByWrapper)
    {
        // Already initialized by us on this thread
        return true;
    }

    // Try to initialize COM in MULTITHREADED mode (same as wmain needs)
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    
    if (SUCCEEDED(hr) || hr == S_FALSE)
    {
        // Successfully initialized or already initialized in compatible mode
        g_comInitializedByWrapper = true;
        return true;
    }
    else if (hr == RPC_E_CHANGED_MODE)
    {
        // COM already initialized in a different threading model (likely APARTMENTTHREADED from Unity)
        // This will cause wmain's CoInitializeEx to fail
        // We can't fix this without modifying wmain, so we'll let wmain fail and report the error
        wchar_t errorMsg[256];
        swprintf_s(errorMsg, 256, 
            L"COM already initialized in incompatible mode (0x%08X). "
            L"Unity may have initialized COM as APARTMENTTHREADED. "
            L"Try calling from a worker thread or restart Unity editor.", 
            static_cast<unsigned int>(hr));
        SetLastError(errorMsg);
        return false;
    }
    else
    {
        // Other COM initialization failure
        wchar_t errorMsg[256];
        swprintf_s(errorMsg, 256, L"Failed to initialize COM (0x%08X)", static_cast<unsigned int>(hr));
        SetLastError(errorMsg);
        return false;
    }
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

    // Ensure COM is initialized in the correct mode before calling wmain
    if (!EnsureComForWmain())
    {
        return TEXCONV_ERROR_INITIALIZATION;
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

        // Note: outputFile is not used as texconv doesn't support specifying
        // individual output file names via command line. Output directory (-o)
        // is used instead, and the output filename is derived from input filename.

        // Input file (required)
        argStorage.push_back(options->inputFile);
        argv.push_back(const_cast<wchar_t*>(argStorage.back().c_str()));

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
            // Provide more detailed error message with the exit code
            wchar_t errorMsg[256];
            swprintf_s(errorMsg, 256, L"Texture conversion failed with exit code %d. Check input file path and format.", result);
            SetLastError(errorMsg);
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

    // Ensure COM is initialized in the correct mode before calling wmain
    if (!EnsureComForWmain())
    {
        return TEXCONV_ERROR_INITIALIZATION;
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

        // Note: wmain initializes COM on first call and leaves it initialized
        // This is intentional to support repeated calls on the same thread

        if (result == 0)
        {
            SetLastError(L"");
            return TEXCONV_SUCCESS;
        }
        else
        {
            // Provide more detailed error message with the exit code
            wchar_t errorMsg[256];
            swprintf_s(errorMsg, 256, L"Texture conversion failed with exit code %d. Check command-line arguments and file paths.", result);
            SetLastError(errorMsg);
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
