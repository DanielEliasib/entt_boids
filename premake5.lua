workspace "ecs_boids"
	configurations { "debug", "release" }

local raylib_dir = 'raylib/src'

project "boids"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"

	location "boids/"

	targetdir "bin/%{prj.name}/%{cfg.buildcfg}"
	objdir "obj/%{prj.name}/%{cfg.buildcfg}"
	targetname "boids"

	includedirs { "%{prj.location}/include" }

	includedirs { "%{wks.location}/libs/raylib/include/" }
	libdirs { "%{wks.location}/libs/raylib/" }

	links { "raylib" }

	filter "system:windows"
		links { "OpenGL32", "GDI32", "WinMM"}
	filter {}

	files { "%{prj.location}/**.h", "%{prj.location}/**.hpp", "%{prj.location}/**.cpp" }

	filter "configurations:debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:release"
		defines { "NDEBUG" }
		optimize "On"

	filter {}
