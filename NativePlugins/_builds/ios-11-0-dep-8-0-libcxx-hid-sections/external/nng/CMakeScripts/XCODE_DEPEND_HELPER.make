# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.nng.Debug:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/ios-11-0-dep-8-0-libcxx-hid-sections/external/nng/Debug/libnngd.dylib:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/ios-11-0-dep-8-0-libcxx-hid-sections/external/nng/Debug/libnngd.dylib


PostBuild.nng_static.Debug:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/ios-11-0-dep-8-0-libcxx-hid-sections/external/nng/Debug/libnng_staticd.a:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/ios-11-0-dep-8-0-libcxx-hid-sections/external/nng/Debug/libnng_staticd.a


PostBuild.nng.Release:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/ios-11-0-dep-8-0-libcxx-hid-sections/external/nng/Release/libnng.dylib:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/ios-11-0-dep-8-0-libcxx-hid-sections/external/nng/Release/libnng.dylib


PostBuild.nng_static.Release:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/ios-11-0-dep-8-0-libcxx-hid-sections/external/nng/Release/libnng_static.a:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/ios-11-0-dep-8-0-libcxx-hid-sections/external/nng/Release/libnng_static.a


PostBuild.nng.MinSizeRel:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/ios-11-0-dep-8-0-libcxx-hid-sections/external/nng/MinSizeRel/libnng.dylib:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/ios-11-0-dep-8-0-libcxx-hid-sections/external/nng/MinSizeRel/libnng.dylib


PostBuild.nng_static.MinSizeRel:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/ios-11-0-dep-8-0-libcxx-hid-sections/external/nng/MinSizeRel/libnng_static.a:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/ios-11-0-dep-8-0-libcxx-hid-sections/external/nng/MinSizeRel/libnng_static.a


PostBuild.nng.RelWithDebInfo:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/ios-11-0-dep-8-0-libcxx-hid-sections/external/nng/RelWithDebInfo/libnng.dylib:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/ios-11-0-dep-8-0-libcxx-hid-sections/external/nng/RelWithDebInfo/libnng.dylib


PostBuild.nng_static.RelWithDebInfo:
/Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/ios-11-0-dep-8-0-libcxx-hid-sections/external/nng/RelWithDebInfo/libnng_static.a:
	/bin/rm -f /Users/lucmichalski/local/golang/src/github.com/hellowod/u3d-plugins-development/NativePlugins/_builds/ios-11-0-dep-8-0-libcxx-hid-sections/external/nng/RelWithDebInfo/libnng_static.a




# For each target create a dummy ruleso the target does not have to exist
