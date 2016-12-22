local scriptPath = _SCRIPT_PATH

solution "xgc"
    configurations { "Debug", "Release" }
    location "prj"
    characterset "MBCS"

    filter { "system:windows", "configurations:Relase" }
        flags { "NoIncrementalLink", "LinkTimeOptimization" }

project "Common"
    kind "SharedLib"
    language "C++"
    location "prj/Common"
    includedirs "inc/Common"
    objdir "obj"
    links { "stdc++" }

    flags { "C++11", "MultiProcessorCompile" }

    files {
        "inc/Common/**.h",
        "inc/Common/**.hpp",
        "src/Common/**.cpp",
        "src/Common/**.inl",
    }

    vpaths {
        ["Header Files/*"] = { "inc/Common/**.h", "inc/Common/**.hpp" },
        ["Source Files/*"] = { "src/Common/**.cpp", "src/Common/**.inl" }
    }

    filter "configurations:Debug"
        defines { "_DEBUG", "_DEBUG_OUTPUT", "_LIB_EXPORTS", "_DLL" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG", "_ASSERT_LOG", "_LIB_EXPORTS", "_DLL" }
        optimize "On"

    filter "system:windows"
        architecture "x64"
        defines { "WIN64", "_IMAGEHLP64" }
	    targetdir "lib/$(Configuration)"

    filter "system:linux"
        architecture "x64"
        defines { "LINUX64" }

    filter { "system:linux", "configurations:Debug" }
        targetdir "lib/debug"

    filter { "system:linux", "configurations:Release" }
        targetdir "lib/release"