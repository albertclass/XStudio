local scriptPath = _SCRIPT_PATH

solution "xgcbase"
    configurations { "Debug", "Release" }
    platforms {"Win32", "x64"}
    location "../prj"
    characterset "MBCS"

    filter { "system:windows", "configurations:Relase" }
        flags { "NoIncrementalLink", "LinkTimeOptimization" }

project "Common"
    kind "StaticLib"
    language "C++"
    location "../prj/Common"
    objdir "../obj"
    targetdir "../lib/$(Platform)/$(Configuration)"

    flags { "C++14", "MultiProcessorCompile" }

    files {
        "Common/**.h",
        "Common/**.hpp",
        "Common/**.cpp",
        "Common/**.inl",
    }

    vpaths {
        ["Header Files/*"] = { "Common/**.h", "Common/**.hpp" },
        ["Source Files/*"] = { "Common/**.cpp", "Common/**.inl" }
    }

    filter "configurations:Debug"
        kind "SharedLib"
        defines { "_DEBUG", "_DEBUG_OUTPUT", "_LIB_EXPORTS", "_DLL" }
        flags {"Symbols"}

    filter "configurations:Release"
        kind "SharedLib"
        defines { "NDEBUG", "_ASSERT_LOG", "_LIB_EXPORTS", "_DLL" }
        optimize "On"

    filter "platforms:Win32"
        system "Windows"
        architecture "x86"
        defines { "WIN32", "_IMAGEHLP32", "_WINDOWS" }

    filter "platforms:x64"
        system "Windows"
        architecture "x64"
        defines { "WIN64", "_IMAGEHLP64", "_WINDOWS" }

    filter "platforms:Linux"
        system "Linux"
        architecture "x64"
        defines { "_Linux" }