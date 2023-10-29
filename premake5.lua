workspace "ecs_boids"
	configurations { "debug", "release" }

project "boids"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"

	location "boids"

	targetdir "bin/%{prj.name}/%{cfg.buildcfg}"
	objdir "obj/%{prj.name}/%{cfg.buildcfg}"
	targetname "boids"

	includedirs { "%{prj.location}/include" }

	includedirs { "%{wks.location}/raylib-4.5.0_linux_amd64/include" }
	libdirs { "%{wks.location}/raylib-4.5.0_linux_amd64/lib" }

	links { "raylib" }

	files { "%{prj.location}/**.h", "%{prj.location}/**.hpp", "%{prj.location}/**.cpp" }

	filter "configurations:debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:release"
		defines { "NDEBUG" }
		optimize "On"

	filter {}
