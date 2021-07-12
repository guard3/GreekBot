workspace "GreekBot"
	configurations {
		"Debug",
		"Release"
	}
	platforms {
		"x86",
		"x64"
	}
	location "project"

project "GreekBot"
	kind         "ConsoleApp"
	language     "C++"
	cppdialect   "C++17"
	characterset "MBCS"
	targetdir    "bin\\%{cfg.buildcfg}"
	files        "source\\**"
	includedirs {
		"source",
		"source\\**"
	}
	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"CURL_STATICLIB"
	}
	
	links {
		"Ws2_32",
		"Wldap32",
		"Crypt32",
		"Normaliz"
	}
	sysincludedirs {
		"$(LIBCURL_DIR)\\include",
		"modules\\rapidjson\\include"
	}
	
	disablewarnings "26812"
	
	filter "configurations:Debug"
		links    "libcurl_a_debug"
		symbols  "On"
		optimize "Off"
		defines  "DEBUG"
	filter "configurations:Release"
		links    "libcurl_a"
		symbols  "Off"
		optimize "On"
	filter "platforms:x86"
		syslibdirs   "$(LIBCURL_DIR)\\lib\\x86"
		architecture "x86"
	filter "platforms:x64"
		syslibdirs   "$(LIBCURL_DIR)\\lib\\x64"
		architecture "x86_64"