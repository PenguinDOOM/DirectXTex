//--------------------------------------------------------------------------------------
// File: TexconvWrapper.cs
//
// C# P/Invoke wrapper for TexconvAPI DLL
// Example usage from Unity or any C# application
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//--------------------------------------------------------------------------------------

using System;
using System.Runtime.InteropServices;

namespace DirectXTex
{
    /// <summary>
    /// P/Invoke wrapper for the TexconvAPI DLL
    /// </summary>
    public static class TexconvWrapper
    {
        private const string DllName = "texconv.dll";

        // Error codes
        public const int TEXCONV_SUCCESS = 0;
        public const int TEXCONV_ERROR_INVALID_ARGUMENTS = 1;
        public const int TEXCONV_ERROR_LOAD_FAILED = 2;
        public const int TEXCONV_ERROR_PROCESS_FAILED = 3;
        public const int TEXCONV_ERROR_SAVE_FAILED = 4;
        public const int TEXCONV_ERROR_MEMORY_ALLOCATION = 5;
        public const int TEXCONV_ERROR_INITIALIZATION = 6;

        // Option flags
        public const ulong TEXCONV_OPT_NORMAL_MAP = (1UL << 0);
        public const ulong TEXCONV_OPT_PREMUL_ALPHA = (1UL << 1);
        public const ulong TEXCONV_OPT_DEMUL_ALPHA = (1UL << 2);
        public const ulong TEXCONV_OPT_SEPALPHA = (1UL << 3);
        public const ulong TEXCONV_OPT_NO_ALPHA = (1UL << 4);
        public const ulong TEXCONV_OPT_HFLIP = (1UL << 5);
        public const ulong TEXCONV_OPT_VFLIP = (1UL << 6);
        public const ulong TEXCONV_OPT_FORCE_SRGB = (1UL << 7);
        public const ulong TEXCONV_OPT_FORCE_LINEAR = (1UL << 8);
        public const ulong TEXCONV_OPT_TYPELESS = (1UL << 9);
        public const ulong TEXCONV_OPT_BC_DITHER = (1UL << 10);
        public const ulong TEXCONV_OPT_BC_UNIFORM = (1UL << 11);
        public const ulong TEXCONV_OPT_BC_QUICK = (1UL << 12);
        public const ulong TEXCONV_OPT_TIMING = (1UL << 13);
        public const ulong TEXCONV_OPT_OVERWRITE = (1UL << 14);
        public const ulong TEXCONV_OPT_USE_DX10 = (1UL << 15);
        public const ulong TEXCONV_OPT_USE_DX9 = (1UL << 16);
        public const ulong TEXCONV_OPT_EXPAND_LUMINANCE = (1UL << 17);
        public const ulong TEXCONV_OPT_NOLOGO = (1UL << 18);
        public const ulong TEXCONV_OPT_FIT_POWEROF2 = (1UL << 19);
        public const ulong TEXCONV_OPT_COLORKEY = (1UL << 20);
        public const ulong TEXCONV_OPT_INVERT_Y = (1UL << 21);
        public const ulong TEXCONV_OPT_RECONSTRUCT_Z = (1UL << 22);

        /// <summary>
        /// Texture conversion options
        /// </summary>
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct TexconvOptions
        {
            // Input/Output
            [MarshalAs(UnmanagedType.LPWStr)]
            public string inputFile;

            [MarshalAs(UnmanagedType.LPWStr)]
            public string outputFile;

            [MarshalAs(UnmanagedType.LPWStr)]
            public string outputDir;

            // Size
            public int width;
            public int height;
            public int mipLevels;

            // Format
            [MarshalAs(UnmanagedType.LPWStr)]
            public string format;

            [MarshalAs(UnmanagedType.LPWStr)]
            public string fileType;

            // Filters
            [MarshalAs(UnmanagedType.LPWStr)]
            public string filter;

            [MarshalAs(UnmanagedType.LPWStr)]
            public string srgbIn;

            [MarshalAs(UnmanagedType.LPWStr)]
            public string srgbOut;

            // Options flags
            public ulong options;

            // Compression
            [MarshalAs(UnmanagedType.LPWStr)]
            public string compressionMode;

            // Feature level
            [MarshalAs(UnmanagedType.LPWStr)]
            public string featureLevel;

            // Quality
            public float wicQuality;
            public float alphaThreshold;
            public float alphaWeight;

            // GPU adapter
            public int gpuAdapter;

            // Additional options
            [MarshalAs(UnmanagedType.LPWStr)]
            public string prefix;

            [MarshalAs(UnmanagedType.LPWStr)]
            public string suffix;

            // Reserved
            public IntPtr reserved;
        }

        /// <summary>
        /// Initialize default options
        /// </summary>
        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void TexconvInitOptions(ref TexconvOptions options);

        /// <summary>
        /// Convert a single texture file
        /// </summary>
        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TexconvConvertFile(ref TexconvOptions options);

        /// <summary>
        /// Convert a texture with a command-line style argument string
        /// </summary>
        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int TexconvConvertCommandLine([MarshalAs(UnmanagedType.LPWStr)] string commandLine);

        /// <summary>
        /// Get the last error message
        /// </summary>
        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr TexconvGetLastError();

        /// <summary>
        /// Helper method to get the last error as a managed string
        /// </summary>
        public static string GetLastErrorString()
        {
            IntPtr ptr = TexconvGetLastError();
            if (ptr == IntPtr.Zero)
                return string.Empty;
            return Marshal.PtrToStringUni(ptr);
        }
    }

    /// <summary>
    /// Example usage class
    /// </summary>
    public class TexconvExample
    {
        public static void Example1_BasicConversion()
        {
            // Initialize options with defaults
            TexconvWrapper.TexconvOptions options = new TexconvWrapper.TexconvOptions();
            TexconvWrapper.TexconvInitOptions(ref options);

            // Set input and output
            options.inputFile = "input.png";
            options.outputDir = "output";
            options.format = "BC3_UNORM";
            options.fileType = "DDS";
            options.options = TexconvWrapper.TEXCONV_OPT_OVERWRITE;

            // Convert the file
            int result = TexconvWrapper.TexconvConvertFile(ref options);

            if (result == TexconvWrapper.TEXCONV_SUCCESS)
            {
                Console.WriteLine("Conversion successful!");
            }
            else
            {
                Console.WriteLine($"Conversion failed with error code: {result}");
                Console.WriteLine($"Error message: {TexconvWrapper.GetLastErrorString()}");
            }
        }

        public static void Example2_ResizeAndGenerateMips()
        {
            // Initialize options
            TexconvWrapper.TexconvOptions options = new TexconvWrapper.TexconvOptions();
            TexconvWrapper.TexconvInitOptions(ref options);

            // Configure conversion
            options.inputFile = "texture.tga";
            options.outputDir = "processed";
            options.width = 512;
            options.height = 512;
            options.mipLevels = 0; // Generate full mipmap chain
            options.format = "BC7_UNORM";
            options.fileType = "DDS";
            options.options = TexconvWrapper.TEXCONV_OPT_OVERWRITE | 
                              TexconvWrapper.TEXCONV_OPT_FORCE_SRGB;

            // Convert
            int result = TexconvWrapper.TexconvConvertFile(ref options);

            if (result != TexconvWrapper.TEXCONV_SUCCESS)
            {
                Console.WriteLine($"Error: {TexconvWrapper.GetLastErrorString()}");
            }
        }

        public static void Example3_CommandLineStyle()
        {
            // Use command-line style conversion
            string commandLine = "-f BC3_UNORM -ft DDS -o output -y input.png";
            int result = TexconvWrapper.TexconvConvertCommandLine(commandLine);

            if (result != TexconvWrapper.TEXCONV_SUCCESS)
            {
                Console.WriteLine($"Error: {TexconvWrapper.GetLastErrorString()}");
            }
        }

        public static void Example4_Unity()
        {
            // Example for Unity - convert imported texture to DDS
            string unityTextureAssetPath = "Assets/Textures/MyTexture.png";
            string outputPath = "Assets/StreamingAssets/Textures";

            TexconvWrapper.TexconvOptions options = new TexconvWrapper.TexconvOptions();
            TexconvWrapper.TexconvInitOptions(ref options);

            options.inputFile = unityTextureAssetPath;
            options.outputDir = outputPath;
            options.format = "BC7_UNORM_SRGB";
            options.fileType = "DDS";
            options.mipLevels = 0; // Full mipmap chain
            options.options = TexconvWrapper.TEXCONV_OPT_OVERWRITE;

            int result = TexconvWrapper.TexconvConvertFile(ref options);

            if (result == TexconvWrapper.TEXCONV_SUCCESS)
            {
                // In Unity, you would refresh the AssetDatabase here
                // UnityEditor.AssetDatabase.Refresh();
                Console.WriteLine($"Successfully converted texture to {outputPath}");
            }
            else
            {
                Console.WriteLine($"Failed to convert texture: {TexconvWrapper.GetLastErrorString()}");
            }
        }
    }
}
