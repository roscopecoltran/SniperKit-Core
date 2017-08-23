# Terrain generator

Terrain generation is one of the most complex parts in utymap and it is essential to understand how it is working.

## Table of content

- [General idea](#general-idea)
- [Terrain Regions](#terrain-regions)
- [Terrain layers](#terrain-layers)
- [Z-fighting problem solution](#z-fighting-problem-solution)
- [Using textures](#using-textures)
- [Elevation](#elevation)
- [Clipping bounds](#clipping-bounds)
- [Terrain extras](#terrain-extras)
	- [Forest](#forest)
	- [Water](#water)
	- [What's about roads, railways?](#whats-about-roads-railways)

## General idea

You might expect that terrain generator contains some separate logic for roads, water, other flat areas, but the core idea is that it does NOT know anything about what types of object it generates. Instead it abstracts them by __region__ concept which is just a _3D surface_ with additional properties.

## Terrain Regions

As mentioned above, terrain region is represented by 2D surface:

``` C++
struct Region final {
    /// Layer flag. If it is set all regions with such flag should be merged together.
    bool isLayer() const;
    /// Level value: zero for objects on terrain surface.
    int level;
    /// Area of polygon.
    double area;
    /// Context is optional: might be empty if region is part of a layer.
    /// In this case, context is read from canvas definition.
    std::shared_ptr<const RegionContext> context;
    /// Geometry of region.
    ClipperLib::Paths geometry;
}
```

Each region has a region specific properties such as area, geometry, etc. and shared style properties from a context:

``` C++
/// Region context encapsulates information about region style.
struct RegionContext final {
  const utymap::mapcss::Style style;
  /// Prefix in mapcss.
  const std::string prefix;

  const utymap::builders::MeshBuilder::GeometryOptions geometryOptions;
  const utymap::builders::MeshBuilder::AppearanceOptions appearanceOptions;
}
```
All style properties are known by [MeshBuilder](core/src/builders/MeshBuilder.hpp) and has to be defined in mapcss for region objects:

``` CSS
area|z9-16[kind=park] {
	clip: true;
    builder: terrain;
    mesh-name: terrain_park;
	max-area: 0.25%;

    color: gradient(#98fb98, #064807 10%, #035804 50%, #808080);
    color-noise-freq: 0;
    ele-noise-freq: 0;

    texture-index: 0;
    texture-type: background;
    texture-scale: 50;
}
```

Declaration meaning:
* [_Optional_] __clip__: should region be clipped by tile borders? Default is false.
* [_Required_] __builder__: builder name.
* [_Optional_] __terrain__layer__: a name of terrain layer (see [Terrain Layers](#Terrain_Layers)). If it is defined, other properties are ignored.
* [_Optional_] __mesh-name__: name of generated mesh. If it is not present than default name is used.
* [_Required?_] __max-area__: triangulation setting for max triangle area relative to tile area.
* [_Optional_] __height_offset__: height offset of any point of region. Default is 0. __Note__, as all points are shifted either up or down, some additional points are inflated on sides to avoid possible gaps.
* [_Required_] __color__: fill color represented by gradient or color. Hex and name representations are supported.
* [_Optional_] __color-noise-freq__: color noise in absolute values. Default is 0.
* [_Optional_] __ele-noise-freq__: elevation noise in absolute values. Default is 0.
* [_Required_] __texture_index__: texture index provided by _@texture_ definitions. See [Textures](#Using textures) section for details.
* [_Required_] __texture-type__: texture type inside given texture.
* [_Required_] __texture-scale__: texture scale ratio relative to tile size.

## Terrain layers

Multiple regions can be merged into one __layer__ object to build single mesh from different objects by using __terrain_layer__ declaration: `terrain_layer: road;`

In this case, declarations have to be defined in canvas rule with the corresponding prefix and additional sorting declaration:

``` CSS
canvas|z16 {
    road-max-area: 0.1%;
    road-color-noise-freq: 0;
    road-ele-noise-freq: 0;
    road-color: gray;
    road-sort-order: 1008;
    road-mesh-name: terrain_road;
    road-height-offset:-0.3m;
}
```

## Z-fighting problem solution

As it is possible that different regions overlap, it is important to understand how terrain builder avoids [z-fighting problem](https://en.wikipedia.org/wiki/Z-fighting).

In general, this problem is solved with z-index usage. However, utymap supports z-index setting only for layers: _sort-order_ definition is used for that. For other regions the algorithm sorts them according to their area in ascending order so that smaller region is processed first.

After region or layer is processed, its polygon shape is clipped from background region and all others non-processed polygons. Last step is to render the rest as terrain background mesh to avoid gaps. Background is represented by unnamed layer with _sort-order_ 0, so you can tweak its style in canvas definition.

## Using textures

You can use textures for regions as described in [Using  Textures](texture.md)

## Elevation

By default, most of the scenes use _Flat_ elevation provider which always returns the same height for the all geocoordinates.
You can use real elevation data by specifying _Grid_ (recommended) or _SRTM_ elevation provider when tile is loaded. Under hood, utymap unity library tries to download and cache elevation data from mapzen (Grid) or NASA (SRTM) servers.

Check _Elevation_ scene for working example.

## Clipping bounds

Terrain is always rendered as square and element's bounds will be clipped if they are outside quadkey bounding box. To avoid storing extra points, there is specific clip declaration available. If it specified, element geometry will be clipped during data import procedure.

## Terrain extras

For some complex objects, it might be not enough to generate just surface mesh. For example, let's say we want to generate a forest with uniformly distributed tree. Of course, it is possible to do it using extension mechanism provided by managed library. However, it might be expensive due to extra data marshaling or/and rebuilding polygon mesh. That's why utymap core library supports mesh extra mechanism by __mesh-extra__ declaration.

Mesh extra is defined by ExtrasFuncs map inside _SurfaceGenerator_ class:

```C++
const std::unordered_map<std::string, TerraExtras::ExtrasFunc> ExtrasFuncs =
    {
        {"forest", std::bind(&TerraExtras::addForest, _1, _2)},
        {"water", std::bind(&TerraExtras::addWater, _1, _2)},
    };
};
```
where map key is name of extras and value is function defined the following way:

```C++
/// Specifies Extras function signature.
typedef std::function<void(const utymap::builders::BuilderContext &, TerraExtras::Context &)> ExtrasFunc;
```
As you can see, there are two extras already.

### Forest

Builds a forest using lsystem for tree generation. Here is example of mapcss rule:

```CSS
area|z16[leisure=garden] {
    color: gradient(#98fb98, #adff2f 5%, #006400 10%, #556b2f 80%, #b8860b);
    texture-type: grass;

    mesh-extras: forest;
    mesh-name: terrain_park;
    tree-frequency: 60;
    tree-chunk-size: 30;

    lsys: tree;
    lsys-size: 1m;
    lsys-colors: brown,green;
    lsys-texture-indices: 0,0;
    lsys-texture-types: background,tree;
    lsys-texture-scales: 200,50;
}
```

For more details about lsystem generation, check [Tree generation via L-System](lsystem.md) section.

### Water

Do nothing so far as the water is emulated by _height-offset_ setting

### What's about roads, railways?

So far, there is nothing special about them: roads and railways are represented by specific layers. It is not optimal as there is no nice road connection, crossroads, signs.
This can be implemented theoretically by specific mesh extra or element builder extension. It is in long-term TODO list.
