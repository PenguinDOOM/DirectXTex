# Texconv DLL Implementation Summary

## Overview
This implementation adds DLL support to the DirectXTex Texconv command-line tool, enabling Unity developers and C# applications to perform texture conversions programmatically via P/Invoke.

## What Was Done

### 1. API Design
Created a clean C API that can be easily consumed from C#:
- **TexconvAPI.h**: Header file defining the DLL interface
- **TexconvAPI.cpp**: Implementation that wraps the existing texconv wmain function
- Simple error handling with descriptive error codes and messages
- Two API styles: structured options and command-line string

### 2. Build System
Configured multiple build systems to support the DLL:
- **CMakeLists.txt**: Added `BUILD_TEXCONV_DLL` option for CMake builds
- **Texconv_Desktop_2022_DLL.vcxproj**: Visual Studio 2022 project file
- **DirectXTex_Desktop_2022.sln**: Added DLL project to the solution
- Supports x64 Debug and Release configurations

### 3. C# Integration
Provided complete C# integration support:
- **TexconvWrapper.cs**: P/Invoke wrapper for .NET/Unity
- Complete type mappings for all structures
- Helper methods for error handling
- Four detailed usage examples including Unity editor integration

### 4. Documentation
Comprehensive documentation for developers:
- **README_DLL.md**: Complete guide for building and using the DLL
  - Build instructions for VS and CMake
  - API reference with all functions
  - Usage examples for C# and Unity
  - Troubleshooting guide
- **SECURITY_SUMMARY.md**: Security analysis and recommendations
- **README.md**: Updated main README with DLL information

### 5. Testing & Quality
- **test_api.cpp**: Basic API validation test
- Code review completed with issues addressed:
  - Fixed string conversion to use proper UTF-8/UTF-16 handling
  - Removed duplicate code via helper functions
  - Corrected flag mappings
- Security analysis completed with no critical issues found

## Key Features

### API Flexibility
Two ways to call the DLL:
1. **Structured API**: Type-safe with TexconvOptions struct
2. **Command-line API**: String-based for easy migration

### Error Handling
- Well-defined error codes (6 types)
- Thread-safe error message storage
- Detailed error descriptions via TexconvGetLastError

### Thread Safety
- All exported functions are thread-safe
- Protected shared state with mutex
- Safe for concurrent calls from multiple threads

### Full Feature Support
The DLL supports all texconv features:
- Format conversion (BC1-7, RGBA, etc.)
- Resizing and filtering
- Mipmap generation
- Alpha channel processing
- Normal map operations
- And more...

## Usage Example

### C# (Unity)
```csharp
using DirectXTex;

// Initialize options
var options = new TexconvWrapper.TexconvOptions();
TexconvWrapper.TexconvInitOptions(ref options);

// Configure conversion
options.InputFile = "texture.png";
options.OutputDir = "output";
options.Format = "BC7_UNORM_SRGB";
options.FileType = "DDS";
options.MipLevels = 0;
options.Options = TexconvWrapper.TEXCONV_OPT_OVERWRITE;

// Convert
int result = TexconvWrapper.TexconvConvertFile(ref options);
if (result != TexconvWrapper.TEXCONV_SUCCESS)
{
    Debug.LogError(TexconvWrapper.GetLastErrorString());
}
```

## Building the DLL

### Using CMake
```bash
cmake -B build -DBUILD_TEXCONV_DLL=ON
cmake --build build --config Release
```

### Using Visual Studio
1. Open `DirectXTex_Desktop_2022.sln`
2. Select the `texconv_dll` project
3. Build in Release configuration
4. DLL will be in `Bin\Desktop_2022_DLL\x64\Release\texconv.dll`

## Files Changed/Added

### New Files (7)
- Texconv/TexconvAPI.h
- Texconv/TexconvAPI.cpp
- Texconv/TexconvWrapper.cs
- Texconv/Texconv_Desktop_2022_DLL.vcxproj
- Texconv/README_DLL.md
- Texconv/test_api.cpp
- Texconv/SECURITY_SUMMARY.md

### Modified Files (3)
- CMakeLists.txt
- DirectXTex_Desktop_2022.sln
- README.md

## Design Decisions

### Minimal Code Changes
- No modifications to existing texconv.cpp
- Wraps the existing wmain function
- Preserves all existing functionality

### Safe API Design
- Uses C linkage for compatibility
- No memory management across DLL boundary
- All error handling via return codes
- Thread-safe implementation

### Platform Support
- Windows x64 (primary target)
- Can be extended to x86 if needed
- Requires Windows 10 SDK to **build** (not to run)

### Dependencies
- **DirectXTex library**: Statically linked into the DLL
- **Windows system DLLs**: ole32.dll, windowscodecs.dll, version.dll (included with Windows 10/11)
- **Visual C++ Runtime**: May need redistribution on target machines
- **No SDK required at runtime**: End users do not need Windows 10 SDK installed

## Next Steps for Users

1. **Build the DLL**: Use CMake or Visual Studio
2. **Copy to Unity**: Place in `Assets/Plugins/x86_64/`
3. **Copy C# wrapper**: Add TexconvWrapper.cs to your project
4. **Start converting**: Use the API examples as a starting point

## Support & Issues

For questions or issues:
- See README_DLL.md for detailed documentation
- Check SECURITY_SUMMARY.md for security considerations
- Refer to the main DirectXTex wiki for texconv features

## License

Copyright (c) Microsoft Corporation. Licensed under the MIT License.
