# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.tstunity.Debug:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/Debug/tstunity.bundle/Contents/MacOS/tstunityd:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/Debug/tstunity.bundle/Contents/MacOS/tstunityd


PostBuild.nng.Debug:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/external/nng/Debug/libnngd.dylib:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/external/nng/Debug/libnngd.dylib


PostBuild.nng_static.Debug:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/external/nng/Debug/libnng_staticd.a:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/external/nng/Debug/libnng_staticd.a


PostBuild.tstunity.Release:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/Release/tstunity.bundle/Contents/MacOS/tstunity:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/Release/tstunity.bundle/Contents/MacOS/tstunity


PostBuild.nng.Release:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/external/nng/Release/libnng.dylib:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/external/nng/Release/libnng.dylib


PostBuild.nng_static.Release:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/external/nng/Release/libnng_static.a:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/external/nng/Release/libnng_static.a


PostBuild.tstunity.MinSizeRel:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/MinSizeRel/tstunity.bundle/Contents/MacOS/tstunity:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/MinSizeRel/tstunity.bundle/Contents/MacOS/tstunity


PostBuild.nng.MinSizeRel:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/external/nng/MinSizeRel/libnng.dylib:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/external/nng/MinSizeRel/libnng.dylib


PostBuild.nng_static.MinSizeRel:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/external/nng/MinSizeRel/libnng_static.a:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/external/nng/MinSizeRel/libnng_static.a


PostBuild.tstunity.RelWithDebInfo:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/RelWithDebInfo/tstunity.bundle/Contents/MacOS/tstunity:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/RelWithDebInfo/tstunity.bundle/Contents/MacOS/tstunity


PostBuild.nng.RelWithDebInfo:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/external/nng/RelWithDebInfo/libnng.dylib:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/external/nng/RelWithDebInfo/libnng.dylib


PostBuild.nng_static.RelWithDebInfo:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/external/nng/RelWithDebInfo/libnng_static.a:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/xcode-hid-sections/external/nng/RelWithDebInfo/libnng_static.a




# For each target create a dummy ruleso the target does not have to exist
