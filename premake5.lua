workspace "Trace"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Public"
	}

OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

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
		"trace/src"
	}

	filter "system:windows"
		staticruntime "off"

	defines
	{
		"TRC_WINDOWS",
		"TRC_CORE",
		"TRC_ASSERT_ENABLED",
		"TRC_DEBUG_BUILD"
	}

	

	filter "configurations:Debug"
		symbols "On"
		runtime "Debug"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"

	filter "configurations:Public"
		runtime "Release"
		optimize "On"


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
		"trace"
	}

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"