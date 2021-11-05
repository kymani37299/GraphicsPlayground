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

project "Engine"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"

	files
	{
		"engine/**.h",
		"engine/**.cpp",
		"engine/**.hpp"
	}

	includedirs
	{
		"engine",
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
	  "{COPY} %{cfg.targetdir}/Engine.dll ./"
	}

	filter { "configurations:Debug" }
		symbols "On"
		defines
		{
			"ENGINE",
			"DEBUG",
			"_DEBUG",
			"_CONSOLE"
		}

	filter { "configurations:Release" }
		optimize "On"
		defines
		{
			"ENGINE"
		}

project "Playground"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++17"
	files
	{
		"playground/**.h",
		"playground/**.cpp",
	}

	includedirs
	{
		"playground",
		"engine",
		"extern/glm/include",
	}

	links
	{
		"Engine"
	}

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