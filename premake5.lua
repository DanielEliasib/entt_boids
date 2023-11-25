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

	files { "%{prj.location}/**.h", "%{prj.location}/**.hpp", "%{prj.location}/**.cpp" }

	filter "configurations:debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:release"
		defines { "NDEBUG" }
		optimize "On"

	filter {}

	prebuildcommands {
	'{MKDIR} %{wks.location}/libs/raylib/include/',
	'{COPYDIR} %{wks.location}/raylib/src/raylib.h %{wks.location}/raylib/src/raymath.h %{wks.location}/raylib/src/rlgl.h %{wks.location}/libs/raylib/include/',
	'{CHDIR} %{wks.location}/raylib/src/ && make PLATFORM=PLATFORM_DESKTOP && {CHDIR} %{prj.location}',
	'{COPYDIR} %{wks.location}/raylib/src/libraylib.a %{wks.location}/raylib/src/rcore.o %{wks.location}/raylib/src/rshapes.o %{wks.location}/raylib/src/rtextures.o %{wks.location}/raylib/src/rtext.o %{wks.location}/raylib/src/utils.o %{wks.location}/raylib/src/rglfw.o %{wks.location}/raylib/src/rmodels.o %{wks.location}/libs/raylib/' }
