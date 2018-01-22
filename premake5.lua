solution "xgc"
	configurations { "Debug", "Release" }
	location "prj"
	characterset "MBCS"
	architecture "x64"

	symbols "On"

group "library"
	project "common"
		kind "SharedLib"
		language "C++"
		cppdialect "C++11"
		location "prj/library"
		includedirs "src/common"
		targetdir "bin/%{cfg.buildcfg}"
		objdir "obj/%{prj.name}/%{cfg.buildcfg}"

		flags { "MultiProcessorCompile" }

		files {
			"src/common/**.h",
			"src/common/**.hpp",
			"src/common/**.c",
			"src/common/**.cpp",
			"src/common/**.inl",
		}
		vpaths {
			["Header Files/*"] = { "src/common/**.h", "src/common/**.hpp" },
			["Source Files/*"] = { "src/common/**.c", "src/common/**.cpp", "src/common/**.inl" }
		}

		filter "configurations:Debug"
			defines { "_DEBUG", "_DEBUG_OUTPUT", "_LIB_EXPORTS", "_DLL" }

		filter "configurations:Release"
			defines { "NDEBUG", "_ASSERT_LOG", "_LIB_EXPORTS", "_DLL" }
			optimize "On"

		filter "system:windows"
			implibdir "lib/%{cfg.buildcfg}"
			systemversion "10.0.14393.0"
			defines { "WIN64", "_CRT_SECURE_NO_WARNINGS", "_IMAGEHLP64" }

		filter "system:linux"
			implibdir "bin/%{cfg.buildcfg}"
			buildoptions { "-pthread" }
			defines { "LINUX64" }

	project "net"
		kind "SharedLib"
		language "C++"
		cppdialect "C++11"
		location "prj/library"
		includedirs { "src/net", "src/common", "dep/asio/asio/include" }
		targetdir "bin/%{cfg.buildcfg}"
		objdir "obj/%{prj.name}/%{cfg.buildcfg}"
		links { "common" }

		flags { "MultiProcessorCompile" }

		files {
			"src/net/**.h",
			"src/net/**.hpp",
			"src/net/**.cpp",
			"src/net/**.inl",
		}

		vpaths {
			["Header Files/*"] = { "src/net/**.h", "src/net/**.hpp" },
			["Source Files/*"] = { "src/net/**.cpp", "src/net/**.inl" }
		}

		filter "configurations:Debug"
			defines { "_DEBUG", "_DEBUG_OUTPUT", "_LIB_EXPORTS", "_DLL" }

		filter "configurations:Release"
			defines { "NDEBUG", "_ASSERT_LOG", "_LIB_EXPORTS", "_DLL" }
			optimize "On"

		filter "system:windows"
			libdirs { "lib/%{cfg.buildcfg}" }
			implibdir "lib/%{cfg.buildcfg}"
			systemversion "10.0.14393.0"
			defines { "WIN64", "_CRT_SECURE_NO_WARNINGS", "_IMAGEHLP64" }

		filter "system:linux"
			libdirs { "bin/%{cfg.buildcfg}" }
			implibdir "bin/%{cfg.buildcfg}"
			links { "stdc++" }
			buildoptions { "-pthread" }
			defines { "LINUX64" }

	project "net_module"
		kind "SharedLib"
		language "C++"
		cppdialect "C++11"
		location "prj/library"
		includedirs { "src/net_module", "src/net", "src/common" }
		targetdir "bin/%{cfg.buildcfg}"
		objdir "obj/%{prj.name}/%{cfg.buildcfg}"
		links { "common", "net" }

		flags { "MultiProcessorCompile" }

		files {
			"src/net_module/**.h",
			"src/net_module/**.cpp",
		}

		vpaths {
			["Header Files/*"] = { "src/net_module/**.h" },
			["Source Files/*"] = { "src/net_module/**.cpp" }
		}

		filter "configurations:Debug"
			defines { "_DEBUG", "_DEBUG_OUTPUT", "_LIB_EXPORTS", "_DLL" }

		filter "configurations:Release"
			defines { "NDEBUG", "_ASSERT_LOG", "_LIB_EXPORTS", "_DLL" }
			optimize "On"

		filter "system:windows"
			implibdir "lib/%{cfg.buildcfg}"
			systemversion "10.0.14393.0"
			defines { "WIN64", "_CRT_SECURE_NO_WARNINGS", "_IMAGEHLP64" }
			libdirs "lib/%{cfg.buildcfg}"

		filter "system:linux"
			libdirs "bin/%{cfg.buildcfg}"
			implibdir "bin/%{cfg.buildcfg}"
			links { "stdc++" }
			buildoptions { "-pthread" }
			defines { "LINUX64" }

	project "database"
		kind "SharedLib"
		language "C++"
		cppdialect "C++11"
		location "prj/library"
		includedirs { "src/database", "src/common"}
		targetdir "bin/%{cfg.buildcfg}"
		objdir "obj/%{prj.name}/%{cfg.buildcfg}"

		flags { "MultiProcessorCompile" }

		files {
			"src/database/**.h",
			"src/database/**.hpp",
			"src/database/**.h",
			"src/database/**.hpp",
			"src/database/**.cpp",
			"src/database/**.inl"
		}

		vpaths {
			["Header Files/*"] = { "src/database/**.h", "src/database/**.hpp", "src/database/**.h", "src/database/**.hpp" },
			["Source Files/*"] = { "src/database/**.cpp", "src/database/**.inl" }
		}

		filter "configurations:Debug"
			defines { "_DEBUG", "_DEBUG_OUTPUT", "_DB_EXPORTS", "_DLL" }

		filter "configurations:Release"
			defines { "NDEBUG", "_ASSERT_LOG", "_DB_EXPORTS", "_DLL" }
			optimize "On"

		filter "system:windows"
			systemversion "10.0.14393.0"
			includedirs "$(MYSQL56)/include"
			implibdir "lib/%{cfg.buildcfg}"
			libdirs { "lib/%{cfg.buildcfg}", "$(MYSQL56)/lib" }
			links { "common", "libmysql.lib" }
			defines { "WIN64", "_CRT_SECURE_NO_WARNINGS", "_IMAGEHLP64" }

		filter "system:linux"
			implibdir "bin/%{cfg.buildcfg}"
			libdirs { "/usr/lib64/mysql" }
			links { "stdc++", "common", "mysqlclient" }
			buildoptions { "-pthread" }
			defines { "LINUX64" }

	project "nosql"
		kind "StaticLib"
		language "C++"
		cppdialect "C++11"
		location "prj/library"
		includedirs { "src/nosql", "src/common"}
		targetdir "lib/%{cfg.buildcfg}"
		objdir "obj/%{prj.name}/%{cfg.buildcfg}"
		links { "common", "hiredis" }

		flags { "MultiProcessorCompile" }

		files {
			"src/nosql/**.h",
			"src/nosql/**.hpp",
			"src/nosql/**.h",
			"src/nosql/**.hpp",
			"src/nosql/**.cpp",
			"src/nosql/**.inl"
		}

		vpaths {
			["Header Files/*"] = { "src/nosql/**.h", "src/nosql/**.hpp", "src/nosql/**.h", "src/nosql/**.hpp" },
			["Source Files/*"] = { "src/nosql/**.cpp", "src/nosql/**.inl" }
		}

		filter "configurations:Debug"
			defines { "_DEBUG", "_DEBUG_OUTPUT", "_NOSQL_EXPORTS", "_DLL" }

		filter "configurations:Release"
			defines { "NDEBUG", "_ASSERT_LOG", "_NOSQL_EXPORTS", "_DLL" }
			optimize "On"

		filter "system:windows"
			systemversion "10.0.14393.0"
			includedirs { "dep/redis-win" }
			implibdir "lib/%{cfg.buildcfg}"
			libdirs { "lib/%{cfg.buildcfg}", "dep/redis-win/%{cfg.buildcfg}" }
			links { "Win32_interop" }
			defines { "WIN64", "_CRT_SECURE_NO_WARNINGS" }

		filter "system:linux"
			includedirs { "dep/redis" }
			implibdir "lib/%{cfg.buildcfg}"
			libdirs { "lib/%{cfg.buildcfg}", "dep/redis/%{cfg.buildcfg}" }
			links { "stdc++" }
			buildoptions { "-pthread" }
			defines { "LINUX64" }

	project "framework"
		kind "StaticLib"
		language "C++"
		cppdialect "C++11"
		location "prj/library"
		includedirs { 
			"src/framework", 
			"src/common", 
			"src/database", 
			"src/net", 
			"src/net_module",
			"dep/luajit/src",
			"dep/LuaBridge/Source"
		}

		targetdir "lib/%{cfg.buildcfg}"

		objdir "obj/%{prj.name}/%{cfg.buildcfg}"

		libdirs { "dep/luajit/src" }

		links { 
			"common", 
			"database", 
			"net", 
			"net_module",
			"lua51",
		}

		flags { "MultiProcessorCompile" }

		files {
			"src/framework/**.h",
			"src/framework/**.cpp"
		}

		vpaths {
			["Header Files/*"] = { "src/framework/**.h" },
			["Source Files/*"] = { "src/framework/**.cpp" },
		}

		filter "configurations:Debug"
			defines { "_DEBUG", "_DEBUG_OUTPUT" }

		filter "configurations:Release"
			defines { "NDEBUG", "_ASSERT_LOG" }
			optimize "On"
		
		filter "system:windows"
			libdirs { "lib/%{cfg.buildcfg}" }
			systemversion "10.0.14393.0"
			defines { "WIN64", "_CRT_SECURE_NO_WARNINGS" }

		filter "system:linux"
			libdirs { "bin/%{cfg.buildcfg}" }
			links { "stdc++", "rt", "pthread" }
			buildoptions { "-pthread" }
			defines { "LINUX64" }

	project "core"
		kind "SharedLib"
		language "C++"
		cppdialect "C++11"
		location "prj/library"
		includedirs { "src/core", "src/common" }
		links { "common" }
		targetdir "bin/%{cfg.buildcfg}"
		objdir "obj/%{prj.name}/%{cfg.buildcfg}"

		flags { "MultiProcessorCompile" }

		files {
			"src/core/**.h",
			"src/core/**.hpp",
			"src/core/**.c",
			"src/core/**.cpp",
			"src/core/**.inl",
		}

		vpaths {
			["Header Files/*"] = { "src/core/**.h", "src/core/**.hpp" },
			["Source Files/*"] = { "src/core/**.c", "src/core/**.cpp", "src/core/**.inl" }
		}

		filter "configurations:Debug"
			defines { "_DEBUG", "_DEBUG_OUTPUT", "CORE_EXPORTS", "_DLL" }

		filter "configurations:Release"
			defines { "NDEBUG", "_ASSERT_LOG", "CORE_EXPORTS", "_DLL" }
			optimize "On"

		filter "system:windows"
			implibdir "lib/%{cfg.buildcfg}"
			systemversion "10.0.14393.0"
			defines { "WIN64" }

		filter "system:linux"
			implibdir "bin/%{cfg.buildcfg}"
			buildoptions { "-pthread" }
			defines { "LINUX64" }

group "sample"
	function sample_project( prj_name, src_path )
		project( prj_name )
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++11"
		location "prj/sample"
		includedirs { "sample/network", "src/common", "src/net", "src/net_module" }
		targetdir "bin/%{cfg.buildcfg}"
		objdir "obj/%{prj.name}/%{cfg.buildcfg}"
		links { "common", "net", "net_module" }
		defines { "PDC_WIDE" }

		flags { "MultiProcessorCompile" }

		files {
			"sample/network/header.h",
			"sample/network/protocol.h",
			"sample/network/session.h",
			"sample/network/session.cpp",
			"sample/network/" .. src_path .. "/**",
		}

		filter "configurations:Debug"
			defines { "_DEBUG", "_DEBUG_OUTPUT" }

		filter "configurations:Release"
			defines { "NDEBUG", "_ASSERT_LOG" }
			optimize "On"
		
		filter "system:windows"
			libdirs { "lib/%{cfg.buildcfg}" }
			systemversion "10.0.14393.0"
			defines { "WIN64", "_CRT_SECURE_NO_WARNINGS" }

		filter "system:linux"
			libdirs { "bin/%{cfg.buildcfg}" }
			links { "stdc++", "rt", "pthread" }
			buildoptions { "-pthread" }
			defines { "LINUX64" }
	end

	sample_project( "file_server", "server" )
	sample_project( "file_client", "client" )

group "chat"
	function chat_project( prj_name )
		local prj = project( prj_name )
			kind "ConsoleApp"
			language "C++"
			cppdialect "C++11"
			location "prj/chat"
			includedirs { 
				"sample/chat", 
				"sample/chat/chat_protocol",
				"src/common", 
				"src/net"
			}
			
			targetdir "bin/%{cfg.buildcfg}"
			objdir "obj/%{prj.name}/%{cfg.buildcfg}"
			links { "common", "net" }

			flags { "MultiProcessorCompile" }

			files {
				"sample/chat/*.h",
				"sample/chat/chat_protocol/*.h",
				"sample/chat/chat_protocol/*.cc",
				"sample/chat/" .. prj_name .. "/*.h",
				"sample/chat/" .. prj_name .. "/*.cpp",
			}

			vpaths {
				["Protocol/*"] = {
					"sample/chat/chat_protocol/*.h",
					"sample/chat/chat_protocol/*.cc",
				},

				["Header Files/*"] = { 
					"sample/chat/*.h",
					"sample/chat/" .. prj_name .. "/**.h",
				},

				["Source Files/*"] = { 
					"sample/chat/" .. prj_name .. "/**.cpp", 
				},
			}

			filter "configurations:Debug"
				defines { "_DEBUG", "_DEBUG_OUTPUT" }
				links { "libprotobufd" }


			filter "configurations:Release"
				defines { "NDEBUG", "_ASSERT_LOG" }
				links { "libprotobuf" }
				optimize "On"
			
			filter "system:windows"
				includedirs { 
					"dep/protobuf/include", 
					"dep/vld/inc"
				}

				libdirs { 
					"lib/%{cfg.buildcfg}", 
					"dep/protobuf/lib",
					"dep/vld/lib/Win$(PlatformArchitecture)/%{cfg.buildcfg}-$(PlatformToolset)" 
				}

				systemversion "10.0.14393.0"
				defines { "WIN64", "_CRT_SECURE_NO_WARNINGS" }

			filter "system:linux"
				libdirs { "bin/%{cfg.buildcfg}" }
				links { "stdc++", "rt", "pthread", "protobuf" }
				buildoptions { "-pthread" }
				defines { "LINUX64" }
		return prj
	end

	chat_project( "chat_gate" )
	chat_project( "chat_server" )
	chat_project( "chat_client" )

group "servers"
	function server_project( prj_name )
		local prj = project( prj_name )
			kind "ConsoleApp"
			language "C++"
			cppdialect "C++11"
			location "prj/servers"
			includedirs { "sample/servers", "src/common", "src/database", "src/net", "src/net_module", "src/framework" }
			targetdir "bin/%{cfg.buildcfg}"
			objdir "obj/%{prj.name}/%{cfg.buildcfg}"
			links { "common", "database", "net", "net_module", "framework" }
			-- pchheader "sample/servers/header.h"
			-- pchsource "sample/servers/header.cpp"

			flags { "MultiProcessorCompile" }

			files {
				"sample/servers/header.h",
				"sample/servers/header.cpp",
				"sample/servers/protocol.h",
				"sample/servers/".. prj_name .. "/**",
			}

			vpaths {
				["Header Files/*"] = { 
					"sample/servers/*.h", 
					"sample/servers/" .. prj_name .. "/**.h" 
				},

				["Source Files/*"] = { 
					"sample/servers/*.cpp", 
					"sample/servers/" .. prj_name .. "/**.cpp" 
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

				systemversion "10.0.14393.0"
				defines { "WIN64", "_CRT_SECURE_NO_WARNINGS" }

			filter "system:linux"
				libdirs { "bin/%{cfg.buildcfg}" }
				links { "stdc++", "rt", "pthread", "ncurses", "panel" }
				buildoptions { "-pthread" }
				defines { "LINUX64" }
		return prj
	end

	server_project( "gate_server" )
	server_project( "game_server" )
	server_project( "game_client" )

group "tools"
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

			systemversion "10.0.14393.0"
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

			systemversion "10.0.14393.0"
			defines { "WIN64", "_CRT_SECURE_NO_WARNINGS" }

		filter "system:linux"
			libdirs { "bin/%{cfg.buildcfg}" }
			links { "stdc++", "rt", "pthread", "ncurses", "panel" }
			buildoptions { "-pthread" }
			defines { "LINUX64" }

group ""
	project "unittest"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++11"
		location "prj/sample"
		includedirs {"src/common", "src/net", "src/database" }
		targetdir "bin/%{cfg.buildcfg}"
		objdir "obj/%{prj.name}/%{cfg.buildcfg}"
		links { "common", "net", "database" }

		flags { "MultiProcessorCompile" }

		files {
			"test/**.h",
			"test/**.hpp",
			"test/**.cpp",
			"test/**.inl",
		}

		filter "configurations:Debug"
			defines { "_DEBUG", "_DEBUG_OUTPUT" }

		filter "configurations:Release"
			defines { "NDEBUG", "_ASSERT_LOG" }
			optimize "On"
		
		filter "system:windows"
			libdirs { "lib/%{cfg.buildcfg}" }
			systemversion "10.0.14393.0"
			defines { "WIN64", "_CRT_SECURE_NO_WARNINGS" }

		filter "system:linux"
			libdirs { "bin/%{cfg.buildcfg}", "lib/%{cfg.buildcfg}" }
			links { "stdc++", "rt", "pthread" }
			buildoptions { "-pthread" }
			defines { "LINUX64" }
