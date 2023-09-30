
workspace "Trace"
	architecture "x64"
	startproject "Trace_Ed"

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
IncludeDir["stb_image"] = "externals/stb_image"
IncludeDir["spdlog"] = "externals/spdlog"
IncludeDir["obj_loader"] = "externals/obj_loader"
IncludeDir["tiny_obj_loader"] = "externals/tiny_obj_loader"
IncludeDir["msdfgen"] = "externals/msdfgen"
IncludeDir["msdf_atlas_gen"] = "externals/msdf_atlas_gen"
IncludeDir["imgui"] = "externals/imgui"
IncludeDir["entt"] = "externals/entt"
IncludeDir["yaml_cpp"] = "externals/yaml_cpp"


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
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/_externals/**.cpp"
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
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.obj_loader}",
		"%{IncludeDir.tiny_obj_loader}",
		"%{IncludeDir.msdfgen}",
		"%{IncludeDir.msdf_atlas_gen}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}",
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
		"spirv-cross-glsl.lib",
		"msdfgen.lib",
		"msdf_atlas_gen.lib",
		"yaml_cpp.lib"
	}
	
	filter "files:trace/_externals/**.cpp"
	flags { "NoPCH" }

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
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.entt}",
		-- please remove these includes before generating projects, i have issues with my visual studio
		"C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.10150.0\\ucrt"
	}

	libdirs
	{
		"externals/libs",
		-- please remove these lib directory before generating projects, i have issues with my visual studio
		"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.10150.0\\ucrt\\x64"
	}

	defines
	{
		"TRC_APP",
	}

	filter "system:windows"
	
	defines
	{
		"TRC_WINDOWS",
	}

	links
	{
		"trace",
	}

	filter "configurations:Debug"
		symbols "On"
		buildoptions "/MD"

	defines
	{
		"TRC_ASSERT_ENABLED",
		"TRC_DEBUG_BUILD"
	}

	filter "configurations:Release"
		optimize "On"

	defines
	{
		"TRC_RELEASE_BUILD"
	}

project "Trace_Ed"
	kind "ConsoleApp"
	language "C++"
	location "Trace_Ed"
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
		"%{IncludeDir.glm}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.entt}",
		-- please remove these includes before generating projects, i have issues with my visual studio
		"C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.10150.0\\ucrt"
	}

	libdirs
	{
		"externals/libs",
		-- please remove these lib directory before generating projects, i have issues with my visual studio
		"C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.10150.0\\ucrt\\x64"
	}

	links
	{
		"trace",
	}

	defines
	{
		"TRC_APP",
		"TRC_EDITOR"
	}

	filter "system:windows"
	
	defines
	{
		"TRC_WINDOWS",
	}

	

	filter "configurations:Debug"
		symbols "On"
		buildoptions "/MD"
		defines
		{
			"TRC_ASSERT_ENABLED",
			"TRC_DEBUG_BUILD"
		}

	filter "configurations:Release"
		optimize "On"
		defines
		{
			"TRC_RELEASE_BUILD"
		}


