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
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. OutputDir .. "/%{prj.name}");
	objdir ("bin-int/" .. OutputDir .. "/%{prj.name}");

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	filter "system:windows"
		staticruntime "On"

	defines
	{
		"TRC_WINDOWS",
		"TRC_CORE"
	}

	postbuildcommands
	{
		( "{COPY} %{cfg.buildtarget.relpath} ../bin/" .. OutputDir .. "/TestApp" )
	}

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
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
		"TRC_WINDOWS"
	}

	links
	{
		"trace"
	}

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"