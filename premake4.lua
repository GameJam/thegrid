local platform

if os.get() == "windows" then
	platform = "win32"
elseif os.get() == "macosx" then
	platform = "osx"
else
	error("unrecognized platform")
end


solution "TheGrid"
    configurations { "Debug", "Release" }
    location "build"
	debugdir "working"
    
  	if platform == "win32" then 
    	defines { "_CRT_SECURE_NO_WARNINGS", "WIN32" }
    end
    
    vpaths { 
        ["Header Files"] = { "**.h" },
        ["Source Files"] = { "**.cpp" },
    }

project "TheGrid"
    kind "WindowedApp"
    location "build"
    language "C++"
    files {
		"src/*.h",
		"src/*.cpp",
		"src/" .. platform .. "/*.cpp",
		"src/" .. platform .. "/*.h"
	}		
    includedirs {
		"libs/SDL/include",
		"libs/FreeImage/include",
	}
	libdirs {
		"libs/SDL/lib",
		"libs/FreeImage/lib",
	}
    links {
		"SDL",
		"SDLmain",
		"freeimage",	    
	}
	if platform == "win32" then
		links {
			"opengl32",
			"glu32",
			"ws2_32",
			"winmm",
		}
	end

    configuration "Debug"
        defines { "DEBUG" }
        flags { "Symbols" }
        targetdir "bin/debug"

    configuration "Release"
        defines { "NDEBUG" }
        flags { "Optimize" }
        targetdir "bin/release"