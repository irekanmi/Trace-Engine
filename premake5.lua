workspace "Trace"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Public"
	}

OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "externals/GLFW/include"
IncludeDir["GLEW"] = "externals/GLEW/include"

project "trace"
	location "trace"
	kind "StaticLib"
	language "C++"
	staticruntime "On"

	targetdir ("bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("bin-int/" .. OutputDir .. "/%{prj.name}")

	pchheader ( "pch.h" )
	pchsource ( "trace/src/core/pch.cpp" )

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"trace/src/core",
		"trace/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLEW}"
	}


	libdirs
	{
		"externals/libs"
	}


	links
	{
		"opengl32.lib",
		"glew32s.lib",
		"glfw3.lib"
	}	

	filter "configurations:Debug"
		symbols "On"
		runtime "Debug"

	defines
	{
		"TRC_WINDOWS",
		"TRC_CORE",
		"TRC_ASSERT_ENABLED",
		"TRC_DEBUG_BUILD",
		"GLEW_STATIC"
	}

	filter "configurations:Release"
		runtime "Release"
		optimize "On"

		defines
		{
			"TRC_WINDOWS",
			"TRC_CORE",
			"TRC_RELEASE_BUILD",
			"GLEW_STATIC"
		}

	
	filter "configurations:Public"
		runtime "Release"
		optimize "On"

	filter "system:windows"
		staticruntime "off"

	

project "TestApp"
	kind "ConsoleApp"
	language "C++"
	location "TestApp"
	
	
	targetdir ( "bin/" .. OutputDir .. "/%{prj.name}" )		
	objdir ( "bin-int/" .. OutputDir .. "/%{prj.name}" )

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"trace/src"
	}

	filter "system:windows"
	
	defines
	{
		"TRC_WINDOWS",
		"TRC_APP",
		"TRC_ASSERT_ENABLED",
		"TRC_DEBUG_BUILD"
	}

	links
	{
		"trace",
	}

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"