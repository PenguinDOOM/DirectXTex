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

## Using the DLL from C#

### 1. Copy the DLL to your project

For Unity:
- Copy `texconv.dll` to `Assets/Plugins/x86_64/` for 64-bit builds
- Copy any required DirectXTex DLL dependencies to the same directory

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
options.InputFile = "Assets/Textures/MyTexture.png";
options.OutputDir = "Assets/StreamingAssets/Textures";
options.Format = "BC7_UNORM_SRGB";
options.FileType = "DDS";
options.MipLevels = 0; // Generate full mipmap chain
options.Options = TexconvWrapper.TEXCONV_OPT_OVERWRITE;

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
options.Options = TexconvWrapper.TEXCONV_OPT_OVERWRITE | 
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

        options.InputFile = absoluteInputPath;
        options.OutputDir = outputDir;
        options.Format = "BC7_UNORM_SRGB";
        options.FileType = "DDS";
        options.MipLevels = 0;
        options.Options = TexconvWrapper.TEXCONV_OPT_OVERWRITE;

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

## Limitations

- The DLL must be compiled for the same architecture (x64/x86) as your application
- Windows only (uses DirectX and WIC)
- Requires Visual C++ Runtime to be installed on target machines

## Troubleshooting

### DLL Not Found
Ensure the DLL is in the correct location for your application:
- Unity: `Assets/Plugins/x86_64/` for 64-bit
- Other: Same directory as executable or in PATH

### COM Initialization Errors
If you see COM-related errors, ensure you're calling the DLL from the main thread in Unity.

### Missing Dependencies
The DLL requires:
- DirectXTex.dll (if built as shared library)
- Visual C++ Runtime (typically already installed)
- Windows 10 SDK libraries

## License

Copyright (c) Microsoft Corporation. Licensed under the MIT License.

See the LICENSE file in the root of the DirectXTex repository for details.
