function tool_project( prj_name, src_path )
	project( prj_name )
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++11"
	location "prj/tools"
	includedirs { "src/common", }
	targetdir "bin/%{cfg.buildcfg}"
	objdir "obj/%{prj.name}/%{cfg.buildcfg}"
	links { "common" }
	flags { "MultiProcessorCompile" }

	files {
		src_path .. "/**",
	}

	filter "configurations:Debug"
		defines { "_DEBUG", "_DEBUG_OUTPUT" }

	filter "configurations:Release"
		defines { "NDEBUG", "_ASSERT_LOG" }
		optimize "On"
	
	filter "system:windows"
		libdirs { "lib/%{cfg.buildcfg}" }
		systemversion "10.0.16299.0"
		defines { "WIN64", "_CRT_SECURE_NO_WARNINGS" }

	filter "system:linux"
		libdirs { "bin/%{cfg.buildcfg}" }
		links { "stdc++", "rt", "pthread" }
		buildoptions { "-pthread" }
		defines { "LINUX64" }
end

solution "tools"
	configurations { "Debug", "Release" }
	location "prj"
	characterset "MBCS"
	architecture "x64"

	symbols "On"

	tool_project( "easebuffer", "tools/easebuffer" )

	project "lua_debuger_stub"
		kind "SharedLib"
		language "C++"
		cppdialect "C++11"
		location "prj/tools"
		includedirs { "src/common", "dep/luajit/src" }
		libdirs {"dep/luajit/src" }
		defines "LUA_DEBUGER_EXPORTS"

		targetdir "bin/%{cfg.buildcfg}"
		objdir "obj/%{prj.name}/%{cfg.buildcfg}"
		links { "common", "lua51" }

		flags { "MultiProcessorCompile" }

		files {
			"tools/" .. "lua_debuger_stub" .. "/**",
		}

		vpaths {
			["Header Files/*"] = { 
				"tools/" .. "lua_debuger_stub" .. "/*.h", 
			},

			["Source Files/*"] = { 
				"tools/" .. "lua_debuger_stub" .. "/*.cpp", 
			}
		}

		filter "configurations:Debug"
			defines { "_DEBUG", "_DEBUG_OUTPUT" }

		filter "configurations:Release"
			defines { "NDEBUG", "_ASSERT_LOG" }
			optimize "On"
		
		filter "system:windows"
			includedirs { "dep/vld/src/" }
			libdirs { 
				"lib/%{cfg.buildcfg}", 
				"dep/vld/lib/Win$(PlatformArchitecture)/%{cfg.buildcfg}-$(PlatformToolset)" 
			}

			systemversion "10.0.16299.0"
			defines { "WIN64", "_CRT_SECURE_NO_WARNINGS" }

		filter "system:linux"
			libdirs { "bin/%{cfg.buildcfg}" }
			links { "stdc++", "rt", "pthread", "ncurses", "panel" }
			buildoptions { "-pthread" }
			defines { "LINUX64" }

	project "lua_debuger"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++11"
		location "prj/tools"
		includedirs { "src/common" }
		libdirs {"dep/luajit/src" }

		targetdir "bin/%{cfg.buildcfg}"
		objdir "obj/%{prj.name}/%{cfg.buildcfg}"
		links { "common" }

		flags { "MultiProcessorCompile" }

		files {
			"tools/lua_debuger/**",
		}

		vpaths {
			["Header Files/*"] = { 
				"tools/lua_debuger/*.h", 
			},

			["Source Files/*"] = { 
				"tools/lua_debuger/*.cpp", 
			}
		}

		filter "configurations:Debug"
			defines { "_DEBUG", "_DEBUG_OUTPUT" }

		filter "configurations:Release"
			defines { "NDEBUG", "_ASSERT_LOG" }
			optimize "On"
		
		filter "system:windows"
			includedirs { "dep/vld/src/" }
			libdirs { 
				"lib/%{cfg.buildcfg}", 
				"dep/vld/lib/Win$(PlatformArchitecture)/%{cfg.buildcfg}-$(PlatformToolset)" 
			}

			systemversion "10.0.16299.0"
			defines { "WIN64", "_CRT_SECURE_NO_WARNINGS" }

		filter "system:linux"
			libdirs { "bin/%{cfg.buildcfg}" }
			links { "stdc++", "rt", "pthread", "ncurses", "panel" }
			buildoptions { "-pthread" }
			defines { "LINUX64" }