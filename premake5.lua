workspace "GraphicsPlayground"
	architecture "x86"

	configurations
	{
		"Debug",
		"Release"
	}

	targetdir ("out/bin/%{prj.name}/%{cfg.longname}")
	objdir ("out/obj/%{prj.name}/%{cfg.longname}")

project "Engine"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"

	files
	{
		"engine/**.h",
		"engine/**.cpp",
	}

	includedirs
	{
		"engine",
		"extern/glm/include",
		"extern/stb/include",
		"extern/assimp/include"
	}

	libdirs
	{
		"extern/assimp/lib/x86"
	}

	links
	{
		"d3d11.lib",
		"d3dcompiler.lib",
		"assimp-vc142-mtd.lib",
		--"user32.lib"
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
			"extern/stb/include",
			"extern/assimp/include"
		}

		libdirs
		{
			"extern/assimp/lib/x86"
		}

		links
		{
			"Engine",
			"d3d11.lib",
			"d3dcompiler.lib",
			"assimp-vc142-mtd.lib",
			--"user32.lib"
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