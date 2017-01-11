local scriptPath = _SCRIPT_PATH

solution "xgc"
    configurations { "Debug", "Release" }
    location "prj"
    characterset "MBCS"

    filter { "system:windows", "configurations:Relase" }
        flags { "NoIncrementalLink", "LinkTimeOptimization" }

project "common"
    kind "SharedLib"
    language "C++"
    location "prj/common"
    includedirs "inc/common"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{prj.name}/%{cfg.buildcfg}"

    flags { "C++11", "MultiProcessorCompile" }

    files {
        "inc/common/**.h",
        "inc/common/**.hpp",
        "src/common/**.cpp",
        "src/common/**.inl",
    }

    vpaths {
        ["Header Files/*"] = { "inc/common/**.h", "inc/common/**.hpp" },
        ["Source Files/*"] = { "src/common/**.cpp", "src/common/**.inl" }
    }

    filter "configurations:Debug"
        defines { "_DEBUG", "_DEBUG_OUTPUT", "_LIB_EXPORTS", "_DLL" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG", "_ASSERT_LOG", "_LIB_EXPORTS", "_DLL" }
        optimize "On"

    filter "system:windows"
        implibdir "lib/%{cfg.buildcfg}"
        architecture "x64"
        defines { "WIN64", "_IMAGEHLP64" }

    filter "system:linux"
        implibdir "bin/%{cfg.buildcfg}"
        links { "stdc++" }
        buildoptions { "-pthread" }
        architecture "x64"
        defines { "LINUX64" }

project "unittest"
    kind "ConsoleApp"
    language "C++"
    location "prj/unittest"
    includedirs "inc/common"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{prj.name}/%{cfg.buildcfg}"
    
    flags { "C++11", "MultiProcessorCompile" }

    files {
        "test/**.h",
        "test/**.hpp",
        "test/**.cpp",
        "test/**.inl",
    }

    filter "configurations:Debug"
        defines { "_DEBUG", "_DEBUG_OUTPUT" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG", "_ASSERT_LOG" }
        symbols "On"
        optimize "On"
    
    filter "system:windows"
    	libdirs { "lib/%{cfg.buildcfg}" }
    	links { "common.lib" }
        architecture "x64"
        defines { "WIN64" }

    filter "system:linux"
       	libdirs { "bin/%{cfg.buildcfg}" }
        links { "stdc++", "rt", "pthread", "common" }
        buildoptions { "-pthread" }
        architecture "x64"
        defines { "LINUX64" }
