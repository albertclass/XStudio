solution "xgc"
    configurations { "Debug", "Release" }
    location "prj"
    characterset "MBCS"

    filter { 
    	"system:windows", 
    	"configurations:Relase" 
    }
    
    flags { 
    	"NoIncrementalLink", 
    	"LinkTimeOptimization" 
   	}

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

project "net"
    kind "SharedLib"
    language "C++"
    location "prj/net"
    includedirs { "inc/net", "inc/common", "dep/asio/asio/include" }
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{prj.name}/%{cfg.buildcfg}"

    flags { "C++11", "MultiProcessorCompile" }

    files {
        "inc/net/**.h",
        "inc/net/**.hpp",
        "src/net/**.cpp",
        "src/net/**.inl",
    }

    vpaths {
        ["Header Files/*"] = { "inc/net/**.h", "inc/net/**.hpp" },
        ["Source Files/*"] = { "src/net/**.cpp", "src/net/**.inl" }
    }

    filter "configurations:Debug"
        defines { "_DEBUG", "_DEBUG_OUTPUT", "_LIB_EXPORTS", "_DLL" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG", "_ASSERT_LOG", "_LIB_EXPORTS", "_DLL" }
        optimize "On"
        libdirs { "lib/%{cfg.buildcfg}" }
        links { "common.lib" }

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

project "net_module"
    kind "SharedLib"
    language "C++"
    location "prj/net"
    includedirs { "inc/net_module", "inc/net", "inc/common" }
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{prj.name}/%{cfg.buildcfg}"

    flags { "C++11", "MultiProcessorCompile" }

    files {
        "inc/net_module/**.h",
        "src/net_module/**.cpp",
    }

    vpaths {
        ["Header Files/*"] = { "inc/net_module/**.h" },
        ["Source Files/*"] = { "src/net_module/**.cpp" }
    }

    filter "configurations:Debug"
        defines { "_DEBUG", "_DEBUG_OUTPUT", "_LIB_EXPORTS", "_DLL" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG", "_ASSERT_LOG", "_LIB_EXPORTS", "_DLL" }
        optimize "On"

    filter "system:windows"
        implibdir "lib/%{cfg.buildcfg}"
        libdirs { "lib/%{cfg.buildcfg}" }
        links { "common.lib", "net.lib" }

        architecture "x64"
        defines { "WIN64", "_IMAGEHLP64" }

    filter "system:linux"
        implibdir "bin/%{cfg.buildcfg}"
        links { "stdc++", "common", "net" }
        buildoptions { "-pthread" }
        architecture "x64"
        defines { "LINUX64" }

project "database"
    kind "SharedLib"
    language "C++"
    location "prj/database"
    includedirs { "inc/database", "inc/common"}
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{prj.name}/%{cfg.buildcfg}"

    flags { "C++11", "MultiProcessorCompile" }

    files {
        "inc/database/**.h",
        "inc/database/**.hpp",
        "src/database/**.h",
        "src/database/**.hpp",
        "src/database/**.cpp",
        "src/database/**.inl"
    }

    vpaths {
        ["Header Files/*"] = { "inc/database/**.h", "inc/database/**.hpp", "src/database/**.h", "src/database/**.hpp" },
        ["Source Files/*"] = { "src/database/**.cpp", "src/database/**.inl" }
    }

    filter "configurations:Debug"
        defines { "_DEBUG", "_DEBUG_OUTPUT", "_DB_EXPORTS", "_DLL" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG", "_ASSERT_LOG", "_DB_EXPORTS", "_DLL" }
        optimize "On"

    filter "system:windows"
    	includedirs "$(MYSQL56)/include"
        implibdir "lib/%{cfg.buildcfg}"
        libdirs { "lib/%{cfg.buildcfg}", "$(MYSQL56)/lib" }
        links { "common.lib", "libmysql.lib" }
        architecture "x64"
        defines { "WIN64", "_IMAGEHLP64" }

    filter "system:linux"
        implibdir "bin/%{cfg.buildcfg}"
        libdirs { "/usr/lib64/mysql" }
        links { "stdc++", "common", "mysqlclient" }
        buildoptions { "-pthread" }
        architecture "x64"
        defines { "LINUX64" }

project "framework"
    kind "SharedLib"
    language "C++"
    location "prj/framework"
    includedirs { "inc/framework", "inc/common", "inc/net", "inc/database"}
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{prj.name}/%{cfg.buildcfg}"
    
    flags { "C++11", "MultiProcessorCompile" }

    files {
        "inc/framework/**.h",
        "src/framework/**.cpp"
    }

    vpaths {
    	["Header Files/*"] = { "inc/framework/**.h" },
        ["Source Files/*"] = { "src/framework/**.cpp" },
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
    	links { "common.lib", "net.lib", "database.lib" }
        architecture "x64"
        defines { "WIN64" }

    filter "system:linux"
       	libdirs { "bin/%{cfg.buildcfg}" }
        links { "stdc++", "rt", "pthread", "common", "net", "database" }
        buildoptions { "-pthread" }
        architecture "x64"
        defines { "LINUX64" }


project "unittest"
    kind "ConsoleApp"
    language "C++"
    location "prj/unittest"
    includedirs {"inc/common", "inc/net", "inc/database" }
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
    	links { "common.lib", "net.lib", "database.lib" }
        architecture "x64"
        defines { "WIN64" }

    filter "system:linux"
       	libdirs { "bin/%{cfg.buildcfg}" }
        links { "stdc++", "rt", "pthread", "common", "net", "database" }
        buildoptions { "-pthread" }
        architecture "x64"
        defines { "LINUX64" }

project "net_client"
    kind "ConsoleApp"
    language "C++"
    location "prj/sample/net_client"
    includedirs { "sample/network", "inc/common", "inc/net", "inc/net_module" }
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{prj.name}/%{cfg.buildcfg}"
    
    flags { "C++11", "MultiProcessorCompile" }

    files {
	    "sample/network/header.h",
	    "sample/network/protocol.h",
	    "sample/network/session.h",
	    "sample/network/session.cpp",
	    "sample/network/client/**",
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
    	links { "common.lib", "net.lib", "net_module.lib" }
        architecture "x64"
        defines { "WIN64" }

    filter "system:linux"
       	libdirs { "bin/%{cfg.buildcfg}" }
        links { "stdc++", "rt", "pthread", "common", "net", "net_module" }
        buildoptions { "-pthread" }
        architecture "x64"
        defines { "LINUX64" }

project "net_server"
    kind "ConsoleApp"
    language "C++"
    location "prj/sample/net_server"
    includedirs { "sample/network", "inc/common", "inc/net", "inc/net_module", "dep/PDCurses" }
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{prj.name}/%{cfg.buildcfg}"
    
    flags { "C++11", "MultiProcessorCompile" }

    files {
        "sample/network/header.h",
        "sample/network/protocol.h",
        "sample/network/session.h",
        "sample/network/session.cpp",
        "sample/network/server/**",
    }

    filter "configurations:Debug"
     	libdirs { "dep/PDCurses/win32/Debug" }
        defines { "_DEBUG", "_DEBUG_OUTPUT" }
        symbols "On"

    filter "configurations:Release"
    	libdirs { "dep/PDCurses/win32/Release" }
        defines { "NDEBUG", "_ASSERT_LOG" }
        symbols "On"
        optimize "On"
    
    filter "system:windows"
    	libdirs { "lib/%{cfg.buildcfg}" }
    	links { "common.lib", "net.lib", "pdcurses.lib", "panel.lib" }
        architecture "x64"
        defines { "WIN64" }

    filter "system:linux"
       	libdirs { "bin/%{cfg.buildcfg}" }
        links { "stdc++", "rt", "pthread", "common", "net" }
        buildoptions { "-pthread" }
        architecture "x64"
        defines { "LINUX64" }

