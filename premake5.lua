
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
IncludeDir["EASTL"] = "externals/EASTL/include"
IncludeDir["glm"] = "externals/glm"
IncludeDir["Vulkan"] = "externals/Vulkan"

project "trace"
	location "trace"
	kind "StaticLib"
	language "C++"
	staticruntime "On"
	cppdialect "C++17"

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
		"%{IncludeDir.GLEW}",
		"%{IncludeDir.EASTL}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.Vulkan}",
		-- please remove these includes before generating projects, i have issues with my visual studio
		"C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.10150.0\\ucrt"
	}


	libdirs
	{
		"externals/libs"
	}


	links
	{
		"opengl32.lib",
		"glew32s.lib",
		"glfw3.lib",
		"vulkan-1.lib",
		"shaderc_combined.lib",
		"spirv-cross-core.lib",
		"spirv-cross-cpp.lib",
		"spirv-cross-glsl.lib"
	}	

	filter "configurations:Debug"
		symbols "On"
		runtime "Debug"
		buildoptions "/MD"

	defines
	{
		"TRC_WINDOWS",
		"TRC_CORE",
		"TRC_ASSERT_ENABLED",
		"TRC_DEBUG_BUILD",
		"GLEW_STATIC"
	}
	links
	{
		"EASTL_d.lib"
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

		links
		{
			"EASTL.lib"
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
	cppdialect "C++17"
	
	
	targetdir ( "bin/" .. OutputDir .. "/%{prj.name}" )		
	objdir ( "bin-int/" .. OutputDir .. "/%{prj.name}" )

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"trace/src",
		"%{IncludeDir.EASTL}",
		"%{IncludeDir.glm}",
		-- please remove these includes before generating projects, i have issues with my visual studio
		"C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.10150.0\\ucrt"
	}

	libdirs
	{
		"externals/libs",
		-- please remove these lib directory before generating projects, i have issues with my visual studio
		"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.10150.0\\ucrt\\x64"
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
		buildoptions "/MD"

	filter "configurations:Release"
		optimize "On"