# Texconv DLL for Unity and C# Applications

This directory contains a DLL version of the texconv command-line tool that can be called from Unity C# scripts or any other C# application using P/Invoke.

## Overview

The texconv DLL provides the same texture conversion functionality as the command-line tool but in a library form that can be integrated directly into your applications.

## Building the DLL

### Using Visual Studio

1. Open the solution file `DirectXTex_Desktop_2022.sln`
2. Add the `Texconv_Desktop_2022_DLL.vcxproj` project to the solution
3. Build the project in your desired configuration (Debug or Release)
4. The DLL will be output to `Bin\Desktop_2022_DLL\x64\[Configuration]\texconv.dll`

### Using CMake

```bash
cmake -B build -DBUILD_TEXCONV_DLL=ON
cmake --build build --config Release
```

The DLL will be located in the build output directory.

### Build Requirements

**To build the DLL, you need:**
- Visual Studio 2022 with C++ Desktop Development workload
- Windows 10 SDK (for headers and import libraries during compilation)
- CMake 3.21+ (if using CMake build)

**Note:** While the Windows 10 SDK is required to **build** the DLL, end users do **not** need the SDK installed to **run** the DLL. The DLL only depends on standard Windows system libraries that ship with Windows 10/11.

## Using the DLL from C#

### 1. Copy the DLL to your project

For Unity:
- Copy `texconv.dll` to `Assets/Plugins/x86_64/` for 64-bit builds
- The DLL is self-contained; no additional DirectXTex DLL dependencies are needed

For other C# projects:
- Copy the DLL to your project's output directory
- Ensure it's in the same directory as your executable or in a directory on your PATH

### 2. Use the C# wrapper

Copy `TexconvWrapper.cs` to your C# project. This file provides a P/Invoke wrapper for the DLL.

### 3. Example usage

```csharp
using DirectXTex;

// Initialize options with defaults
TexconvWrapper.TexconvOptions options = new TexconvWrapper.TexconvOptions();
TexconvWrapper.TexconvInitOptions(ref options);

// Configure conversion
options.inputFile = "Assets/Textures/MyTexture.png";
options.outputDir = "Assets/StreamingAssets/Textures";
options.format = "BC7_UNORM_SRGB";
options.fileType = "DDS";
options.mipLevels = 0; // Generate full mipmap chain
options.options = TexconvWrapper.TEXCONV_OPT_OVERWRITE;

// For BC compression options, use the compressionMode field:
// "d" for dither, "u" for uniform, "q" for quick (BC7), "x" for 3 subsets (BC7)
options.compressionMode = "d"; // Enable dithering for BC compression

// Perform conversion
int result = TexconvWrapper.TexconvConvertFile(ref options);

if (result == TexconvWrapper.TEXCONV_SUCCESS)
{
    Debug.Log("Conversion successful!");
}
else
{
    Debug.LogError($"Conversion failed: {TexconvWrapper.GetLastErrorString()}");
}
```

### 4. Alternative: Command-line style

You can also use the command-line style API:

```csharp
string commandLine = "-f BC3_UNORM -ft DDS -o output -y input.png";
int result = TexconvWrapper.TexconvConvertCommandLine(commandLine);
```

## API Reference

### Functions

#### `TexconvInitOptions`
Initializes a `TexconvOptions` structure with default values.

```csharp
void TexconvInitOptions(ref TexconvOptions options);
```

#### `TexconvConvertFile`
Converts a texture file using the specified options.

```csharp
int TexconvConvertFile(ref TexconvOptions options);
```

**Returns:** 
- `TEXCONV_SUCCESS` (0) on success
- Error code on failure

#### `TexconvConvertCommandLine`
Converts a texture using a command-line style argument string.

```csharp
int TexconvConvertCommandLine(string commandLine);
```

**Returns:** 
- `TEXCONV_SUCCESS` (0) on success
- Error code on failure

#### `TexconvGetLastError`
Returns a description of the last error that occurred.

```csharp
string GetLastErrorString();
```

### Error Codes

- `TEXCONV_SUCCESS` (0) - Operation succeeded
- `TEXCONV_ERROR_INVALID_ARGUMENTS` (1) - Invalid arguments provided
- `TEXCONV_ERROR_LOAD_FAILED` (2) - Failed to load input file
- `TEXCONV_ERROR_PROCESS_FAILED` (3) - Failed to process texture
- `TEXCONV_ERROR_SAVE_FAILED` (4) - Failed to save output file
- `TEXCONV_ERROR_MEMORY_ALLOCATION` (5) - Memory allocation failed
- `TEXCONV_ERROR_INITIALIZATION` (6) - Initialization failed

### Option Flags

Combine flags using bitwise OR:

```csharp
options.options = TexconvWrapper.TEXCONV_OPT_OVERWRITE | 
                  TexconvWrapper.TEXCONV_OPT_FORCE_SRGB;
```

Available flags:
- `TEXCONV_OPT_NORMAL_MAP` - Generate normal map
- `TEXCONV_OPT_PREMUL_ALPHA` - Premultiply alpha
- `TEXCONV_OPT_DEMUL_ALPHA` - Demultiply (remove premultiplication from) alpha
- `TEXCONV_OPT_SEPALPHA` - Separate alpha channel
- `TEXCONV_OPT_NO_ALPHA` - Remove alpha channel
- `TEXCONV_OPT_HFLIP` - Flip horizontally
- `TEXCONV_OPT_VFLIP` - Flip vertically
- `TEXCONV_OPT_FORCE_SRGB` - Force sRGB color space
- `TEXCONV_OPT_FORCE_LINEAR` - Force linear color space
- `TEXCONV_OPT_OVERWRITE` - Overwrite existing files
- `TEXCONV_OPT_USE_DX10` - Use DX10 header
- `TEXCONV_OPT_FIT_POWEROF2` - Fit to power of 2 dimensions
- `TEXCONV_OPT_INVERT_Y` - Invert Y channel (useful for normal maps)
- `TEXCONV_OPT_RECONSTRUCT_Z` - Reconstruct Z channel from X and Y (normal maps)

### Compression Modes

BC compression behavior is controlled via the `CompressionMode` field. Combine characters for multiple options:
- `"d"` - Enable dithering
- `"u"` - Use uniform weighting instead of perceptual
- `"q"` - Use BC7 quick mode (faster, lower quality)
- `"x"` - Use BC7 3-subset mode

Example:
```csharp
options.compressionMode = "du"; // Dithering with uniform weighting
```

## Unity Editor Integration Example

```csharp
using UnityEngine;
using UnityEditor;
using DirectXTex;
using System.IO;

public class TextureConverter : EditorWindow
{
    [MenuItem("Tools/Convert Texture to DDS")]
    static void ConvertSelectedTexture()
    {
        // Get selected texture
        Texture2D texture = Selection.activeObject as Texture2D;
        if (texture == null)
        {
            EditorUtility.DisplayDialog("Error", "Please select a texture", "OK");
            return;
        }

        string inputPath = AssetDatabase.GetAssetPath(texture);
        string absoluteInputPath = Path.GetFullPath(inputPath);
        string outputDir = Path.Combine(Application.streamingAssetsPath, "DDSTextures");

        // Ensure output directory exists
        if (!Directory.Exists(outputDir))
        {
            Directory.CreateDirectory(outputDir);
        }

        // Configure conversion
        TexconvWrapper.TexconvOptions options = new TexconvWrapper.TexconvOptions();
        TexconvWrapper.TexconvInitOptions(ref options);

        options.inputFile = absoluteInputPath;
        options.outputDir = outputDir;
        options.format = "BC7_UNORM_SRGB";
        options.fileType = "DDS";
        options.mipLevels = 0;
        options.options = TexconvWrapper.TEXCONV_OPT_OVERWRITE;

        // Convert
        int result = TexconvWrapper.TexconvConvertFile(ref options);

        if (result == TexconvWrapper.TEXCONV_SUCCESS)
        {
            AssetDatabase.Refresh();
            EditorUtility.DisplayDialog("Success", 
                $"Texture converted successfully to {outputDir}", "OK");
        }
        else
        {
            EditorUtility.DisplayDialog("Error", 
                $"Conversion failed: {TexconvWrapper.GetLastErrorString()}", "OK");
        }
    }
}
```

## Supported Formats

The DLL supports all the same formats as the command-line tool, including:
- Input: PNG, JPEG, BMP, TIFF, TGA, HDR, DDS, and more (via WIC)
- Output: DDS, TGA, HDR, and WIC-supported formats

Common compression formats:
- BC1_UNORM (DXT1)
- BC2_UNORM (DXT3)
- BC3_UNORM (DXT5)
- BC4_UNORM
- BC5_UNORM
- BC6H_UF16
- BC7_UNORM
- BC7_UNORM_SRGB

## Threading Considerations

The DLL uses COM internally (for WIC support), so ensure COM is initialized on the thread calling the DLL:
- The DLL automatically initializes COM with `COINIT_MULTITHREADED`
- In Unity, calls should typically be made from the main thread or from threads where COM has been initialized

## Runtime Dependencies

The texconv.dll has the following runtime dependencies:

### Windows System DLLs (included with Windows 10+)
These DLLs are part of the Windows operating system and do **not** require the Windows 10 SDK to be installed:
- **ole32.dll** - COM support (Component Object Model)
- **windowscodecs.dll** - WIC (Windows Imaging Component) 
- **version.dll** - Version information APIs
- **uuid.lib** - Statically linked into the DLL (no runtime dependency)

All of these are standard Windows system libraries that ship with Windows 10 and Windows 11. End users do **not** need to install the Windows 10 SDK.

### Visual C++ Runtime
- **MSVC Runtime** (e.g., vcruntime140.dll, msvcp140.dll)
- Usually already present on Windows systems
- If not, can be installed via [Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist)

### DirectXTex Library
- The DirectXTex library is **statically linked** into the DLL
- No separate DirectXTex.dll is required at runtime

## Limitations

- The DLL must be compiled for the same architecture (x64/x86) as your application
- Windows 10 or later (uses WIC and DirectX APIs)
- Requires Visual C++ Runtime (typically already installed on Windows systems)

## Troubleshooting

### DLL Not Found
Ensure the DLL is in the correct location for your application:
- Unity: `Assets/Plugins/x86_64/` for 64-bit
- Other: Same directory as executable or in PATH

### COM Initialization Errors
If you see COM-related errors, ensure you're calling the DLL from the main thread in Unity.

### Error Code 3 (TEXCONV_ERROR_PROCESS_FAILED)
If you receive error code 3, the conversion process failed. To diagnose:

1. **Check the error message**: Call `TexconvGetLastError()` or `TexconvWrapper.GetLastErrorString()` to see the detailed error message with exit code
2. **Verify file paths**: Ensure input file path is absolute or relative to Unity's working directory (typically the project root)
3. **Check file exists**: Verify the input file exists and is accessible
4. **Verify output directory**: Ensure the output directory exists or can be created
5. **Check file format**: Verify the input file format is supported (PNG, JPG, TGA, BMP, DDS, etc.)
6. **Test with absolute paths**: Use `System.IO.Path.GetFullPath()` to convert relative paths to absolute paths

Example with error handling:
```csharp
var options = new TexconvWrapper.TexconvOptions();
TexconvWrapper.TexconvInitOptions(ref options);

// Use absolute paths
options.inputFile = System.IO.Path.GetFullPath("Assets/Textures/MyTexture.png");
options.outputDir = System.IO.Path.GetFullPath("Assets/StreamingAssets/DDS");

// Ensure output directory exists
System.IO.Directory.CreateDirectory(options.outputDir);

options.format = "BC7_UNORM_SRGB";
options.fileType = "DDS";

int result = TexconvWrapper.TexconvConvertFile(ref options);
if (result != TexconvWrapper.TEXCONV_SUCCESS)
{
    Debug.LogError($"Conversion failed with code {result}: {TexconvWrapper.GetLastErrorString()}");
}
```

### Missing DLL Dependencies
If you get errors about missing DLLs:

1. **Visual C++ Runtime not found**: Install the [Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist) for the version used to compile the DLL

2. **System DLL errors**: This typically indicates the target system is missing Windows updates. The DLL requires Windows 10 or later with all system components:
   - ole32.dll (COM support)
   - windowscodecs.dll (Windows Imaging Component)
   - version.dll (Version APIs)
   
   These are standard Windows components that should be present on all Windows 10+ systems.

To verify dependencies of the compiled DLL, use the [Dependencies](https://github.com/lucasg/Dependencies) tool or Visual Studio's dumpbin utility:
```
dumpbin /DEPENDENTS texconv.dll
```

## License

Copyright (c) Microsoft Corporation. Licensed under the MIT License.

See the LICENSE file in the root of the DirectXTex repository for details.
