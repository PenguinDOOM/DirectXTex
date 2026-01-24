//--------------------------------------------------------------------------------------
// File: TexconvAPI.h
//
// DirectX Texture Converter DLL API
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248926
//--------------------------------------------------------------------------------------

#pragma once

#ifdef TEXCONV_DLL_EXPORTS
#define TEXCONV_API __declspec(dllexport)
#else
#define TEXCONV_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
#define TEXCONV_SUCCESS 0
#define TEXCONV_ERROR_INVALID_ARGUMENTS 1
#define TEXCONV_ERROR_LOAD_FAILED 2
#define TEXCONV_ERROR_PROCESS_FAILED 3
#define TEXCONV_ERROR_SAVE_FAILED 4
#define TEXCONV_ERROR_MEMORY_ALLOCATION 5
#define TEXCONV_ERROR_INITIALIZATION 6

// Texture conversion options structure
typedef struct TexconvOptions
{
    // Input/Output
    const wchar_t* inputFile;
    const wchar_t* outputFile;  // Reserved for future use; currently not used
    const wchar_t* outputDir;

    // Size
    int width;
    int height;
    int mipLevels;

    // Format
    const wchar_t* format;
    const wchar_t* fileType;

    // Filters
    const wchar_t* filter;
    const wchar_t* srgbIn;
    const wchar_t* srgbOut;

    // Options flags (bitwise OR of option flags)
    unsigned long long options;

    // Compression
    const wchar_t* compressionMode;

    // Feature level
    const wchar_t* featureLevel;

    // Quality
    float wicQuality;
    float alphaThreshold;
    float alphaWeight;

    // GPU adapter
    int gpuAdapter;

    // Additional options
    const wchar_t* prefix;
    const wchar_t* suffix;

    // Reserved for future use
    void* reserved;
} TexconvOptions;

// Option flags
#define TEXCONV_OPT_NORMAL_MAP          (1ULL << 0)
#define TEXCONV_OPT_PREMUL_ALPHA        (1ULL << 1)
#define TEXCONV_OPT_DEMUL_ALPHA         (1ULL << 2)
#define TEXCONV_OPT_SEPALPHA            (1ULL << 3)
#define TEXCONV_OPT_NO_ALPHA            (1ULL << 4)
#define TEXCONV_OPT_HFLIP               (1ULL << 5)
#define TEXCONV_OPT_VFLIP               (1ULL << 6)
#define TEXCONV_OPT_FORCE_SRGB          (1ULL << 7)
#define TEXCONV_OPT_FORCE_LINEAR        (1ULL << 8)
#define TEXCONV_OPT_TYPELESS            (1ULL << 9)
#define TEXCONV_OPT_BC_DITHER           (1ULL << 10)
#define TEXCONV_OPT_BC_UNIFORM          (1ULL << 11)
#define TEXCONV_OPT_BC_QUICK            (1ULL << 12)
#define TEXCONV_OPT_TIMING              (1ULL << 13)
#define TEXCONV_OPT_OVERWRITE           (1ULL << 14)
#define TEXCONV_OPT_USE_DX10            (1ULL << 15)
#define TEXCONV_OPT_USE_DX9             (1ULL << 16)
#define TEXCONV_OPT_EXPAND_LUMINANCE    (1ULL << 17)
#define TEXCONV_OPT_NOLOGO              (1ULL << 18)
#define TEXCONV_OPT_FIT_POWEROF2        (1ULL << 19)
#define TEXCONV_OPT_COLORKEY            (1ULL << 20)
#define TEXCONV_OPT_INVERT_Y            (1ULL << 21)
#define TEXCONV_OPT_RECONSTRUCT_Z       (1ULL << 22)

// Initialize default options
TEXCONV_API void TexconvInitOptions(TexconvOptions* options);

// Convert a single texture file
// Returns TEXCONV_SUCCESS on success, or an error code on failure
TEXCONV_API int TexconvConvertFile(const TexconvOptions* options);

// Convert a texture with a command-line style argument string
// This provides compatibility with the command-line interface
// Returns TEXCONV_SUCCESS on success, or an error code on failure
TEXCONV_API int TexconvConvertCommandLine(const wchar_t* commandLine);

// Get the last error message
// Returns a null-terminated wide string describing the last error
// The string is valid until the next API call or the DLL is unloaded
TEXCONV_API const wchar_t* TexconvGetLastError();

#ifdef __cplusplus
}
#endif
