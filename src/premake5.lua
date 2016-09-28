local scriptPath = _SCRIPT_PATH

solution "xgc"
    configurations { "Debug", "Release" }
    location "../prj"
    characterset "MBCS"

    filter { "system:windows", "configurations:Relase" }
        flags { "NoIncrementalLink", "LinkTimeOptimization" }

project "Common"
    kind "StaticLib"
    language "C++"
    location "../prj/Common"
    objdir "../obj"

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

    filter "system:windows"
        system "Windows"
        architecture "x64"
        defines { "WIN64", "_IMAGEHLP64" }
	    targetdir "../lib/$(Configuration)"

    filter { "system:linux", "configurations:Debug" }
        system "Linux"
        architecture "x64"
        targetdir "../lib/debug"

    filter { "system:linux", "configurations:Release" }
        system "Linux"
        architecture "x64"
        targetdir "../lib/release"