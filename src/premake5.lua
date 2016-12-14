local scriptPath = _SCRIPT_PATH

solution "xgc"
    configurations { "Debug", "Release" }
    location "../prj"
    characterset "MBCS"

    filter { "system:windows", "configurations:Relase" }
        flags { "NoIncrementalLink", "LinkTimeOptimization" }

project "Common"
    kind "SharedLib"
    language "C++"
    location "../prj/Common"
    objdir "../obj"

    flags { "C++1y", "MultiProcessorCompile" }

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
        defines { "_DEBUG", "_DEBUG_OUTPUT", "_LIB_EXPORTS", "_DLL" }
        flags {"Symbols"}

    filter "configurations:Release"
        defines { "NDEBUG", "_ASSERT_LOG", "_LIB_EXPORTS", "_DLL" }
        optimize "On"

    filter "system:windows"
        architecture "x64"
        defines { "WIN64", "_IMAGEHLP64" }
	    targetdir "../lib/$(Configuration)"

    filter "system:linux"
        architecture "x64"
        defines { "LINUX64" }

    filter { "system:linux", "configurations:Debug" }
        targetdir "../lib/debug"

    filter { "system:linux", "configurations:Release" }
        targetdir "../lib/release"