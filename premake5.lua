-- General workspace settings
workspace "GreekBot"
	configurations { "Debug", "Release" }
	if _ACTION ~= "xcode4" then
		platforms "x64"
		platforms "x86" -- Enable 32bit builds for anything other than macOS
	end
	location "project"

project "GreekBot"
	kind         "ConsoleApp"
	language     "C++"
	cppdialect   "C++17"
	characterset "MBCS"
	targetdir    "bin/%{cfg.buildcfg}"
	files        "source2/**"
	includedirs {
		"source2",
		"source2/**.h",
		"source2/**.cpp"
	}
	
	links {
		"crypto",
		"ssl",
		"boost_json"
	}
	sysincludedirs {
		"/usr/local/opt/openssl@1.1/include",
		"/usr/local/Cellar/boost/1.76.0/include"
	}
	syslibdirs {
		"/usr/local/opt/openssl@1.1/lib",
		"/usr/local/Cellar/boost/1.76.0/lib"
	}
	
	if _ACTION ~= "xcode4" then
		disablewarnings "26812"
	end
	
	filter "configurations:Debug"
		symbols  "On"
		optimize "Off"
		defines  "DEBUG"
	filter "configurations:Release"
		symbols  "Off"
		optimize "On"
	filter "platforms:x86"
		architecture "x86"
	filter "platforms:x64"
		architecture "x86_64"