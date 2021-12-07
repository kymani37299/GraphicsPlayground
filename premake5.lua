workspace "GraphicsPlayground"
	architecture "x86"

	configurations
	{
		"Debug",
		"Release"
	}

	targetdir ("out/bin/%{prj.name}/%{cfg.longname}")
	objdir ("out/obj/%{prj.name}/%{cfg.longname}")

include "extern/imgui"

project "GP"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"

	files
	{
		"gp/**.h",
		"gp/**.cpp",
		"gp/**.hpp",
		"gp/**.hlsl"
	}

	includedirs
	{
		"gp",
		"extern/glm/include",
		"extern/stb/include",
		"extern/imgui/src",
		"extern/cgitf"
	}

	links
	{
		"d3d11.lib",
		"d3dcompiler.lib",
		"dxguid.lib",
		"ImGui"
	}

	postbuildcommands {
	  "{COPY} %{cfg.targetdir}/GP.dll ./"
	}

	filter { "files:**.hlsl" }
		flags "ExcludeFromBuild"

	filter { "configurations:Debug" }
		symbols "On"
		defines
		{
			"_GP",
			"DEBUG",
			"_DEBUG",
			"_CONSOLE"
		}

	filter { "configurations:Release" }
		optimize "On"
		defines
		{
			"_GP"
		}

project "Demo"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++17"
	files
	{
		"demo/**.h",
		"demo/**.cpp",
		"demo/**.hlsl"
	}

	includedirs
	{
		"demo",
		"gp",
		"extern/glm/include",
	}

	links
	{
		"GP"
	}

	filter { "files:**.hlsl" }
		flags "ExcludeFromBuild"

	filter { "configurations:Debug" }
		symbols "On"
		defines
		{
			"DEBUG",
			"_DEBUG",
			"_CONSOLE"
		}

	filter { "configurations:Release" }
		optimize "On"

project "Samples"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++17"
	files
	{
		"samples/**.h",
		"samples/**.cpp",
		"samples/**.hlsl"
	}

	includedirs
	{
		"demo",
		"gp",
		"extern/glm/include",
	}

	links
	{
		"GP"
	}

	filter { "files:**.hlsl" }
		flags "ExcludeFromBuild"

	filter { "configurations:Debug" }
		symbols "On"
		defines
		{
			"DEBUG",
			"_DEBUG",
			"_CONSOLE"
		}

	filter { "configurations:Release" }
		optimize "On"