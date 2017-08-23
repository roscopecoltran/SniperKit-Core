# How to..

## Table of content

- [Build your app using Unity3D](#build-your-app-using-unity3d)
- [Build your app without Unity3D](#build-your-app-without-unity3d)
- [Load map data for your region](#load-map-data-for-your-region)
- [Tweak mapcss styles](#tweak-mapcss-styles)
- [Customize POI](#customize-poi)
- [Customize specific buildings](#customize-specific-buildings)
- [Render non-flat terrain using elevation data](#render-non-flat-terrain-using-elevation-data)
- [Change zoom level dynamically from Globe to Ground level](#change-zoom-level-dynamically-from-globe-to-ground-level)
- [Fix crash happens all the time at specific location](#fix-crash-happens-all-the-time-at-specific-location)
- [Get support](#get-support)

### Build your app using Unity3D

The simplest way is to explore demo app. It consists of the following parts:
* platform depended native library (**UtyMap.Shared**). You need to compile it for each platform.
* unity specific managed library (**UtyMap.Unity**). You may need to rebuild it with different platform specific compilation symbols if you plan to use it outside UnityEditor.
* Some additional libraries used by UtyMap.Unity (UtyRx, UtyDepend). Just copy them to _Plugin_ directory.
* Demo app which demonstrates the simplest use cases.

Platform specific instruction can be found on __"Build on .."__ pages.


### Build your app without Unity3D

Native library has no dependency on Unity3D. You can use it inside other game engine or app framework.


###  Load map data for your region

utymap supports dynamic map data loading and it is used by default if there is no map data. Please note:
* it requires internet connection
* it loads data from OpenStreetMap or Mapzen  based on level of details

If you want to import map data from file, check __Import__ scene.


### Tweak mapcss styles

All colors, sizes, etc. are defined in MapCss files and you can easily tweak any value here. Changes will be applied after application restart. Please note, if mesh caching is enabled then all data will be regenerated.


### Customize POI

By default, only few POI are visualized: tree, barrier, lamps. __Customization__ scene shows how to do it if you need more by rendering textured cube. Also you can change any of these cubes to your specific implementation, e.g. render cache machines or traffic lights.


### Customize specific buildings

If you are not satisfied with OSM representation of some specific buildings or any other object, you can use your own 3D model instead or just remove object from scene. To achieve this, you should add specific rule in mapcss:

```CSS
element|z16[id=777] {
// your declarations
}
```

where _id_ is osm id. __Customization__ scene provides an example which uses custom prefab instead of POI.


### Render non-flat terrain using elevation data

You need to use Grid (recommended) or SRTM elevation provider. Check __Elevation__ scene for working example.


### Change zoom level dynamically from Globe to Ground level

Check __Map__ scene. It provides some ideas how it can be implemented.


### Fix crash happens all the time at specific location

This can be an issue inside core library related to polygon geometry processing, mapcss rules problems or something else. In such case, you can report exact location in glitter chat.


###  Get support

You may ask me using [gitter chat] (https://gitter.im/reinterpretcat/utymap).
