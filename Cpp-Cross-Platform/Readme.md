# C++ in multiplatform environment

Examples of using C++ and C# with Xamarin.

* `CppLibrary` is a core library.
* `CSharpProjects`
  * `CppSharpSharedLibrary` a wrapper for the core library using [CppSharp](https://github.com/mono/CppSharp).
  * `iOSXLExample`
  * `AndroidXLExample`
  * `WindowsXLExample`
  * `MacXLExample`
* `Precompiled`
* `Toolchains` is needed for `cmake`.
* `compile.sh` compiles the core library for all platforms, and move results to the `Precompiled`.

## Notes

* Code from `CSharp-Cpp-Interop` and `CSharp-Cpp-SWIG` won't work on iOS, cause of AOT. On the other hand `CppSharp` works, but with modifications in the generated file: [MonoPInvokeCallbackAttribute](https://developer.xamarin.com/api/type/MonoTouch.MonoPInvokeCallbackAttribute/), [Limitations](https://developer.xamarin.com/guides/ios/advanced_topics/limitations/#Reverse_Callbacks).
* A name of the library in `DllImport` is changed depending on a platform.

