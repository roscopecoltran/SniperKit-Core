# Using textures

Additionaly to color, all core built-in builders, e.g. terrain, building, tree, etc. support textures.

## Table of content

- [MapCss definition](#mapcss-definition)
- [Texture map format](#texture-map-format)
- [Runtime usage](#runtime-usage)


## MapCss definition

It is not surprise, that texture usage is controlled by mapcss definitions:

``` CSS
area|z9-16[kind=park] {
    ...
    texture-index: 0;
    texture-type: background;
    texture-scale: 50;
}
```
Here:
* [_Required_] __texture_index__: texture index provided by _@texture_
* [_Required_] __texture-type__: texture type inside given texture.
* [_Required_] __texture-scale__: texture scale ratio relative to tile size.

## Texture map format

Texture index is inside texture mapping file:

```
1,2048,2048
background,0,1536,512,512
background,0,1024,512,512
barrier,512,1536,512,512
brick,512,1024,512,512
drive,0,512,512,512
facade,512,512,512,512
facade,1024,1536,512,512
facade,1024,1024,512,512
glass,1024,512,512,512
grass,0,0,512,512
metal,512,0,512,512
pedestrian,1024,0,512,512
roof,1536,1536,512,512
tree,1536,1024,512,512
water,1536,512,512,512
wood,1536,0,512,512
```

It has the following format:
```
[texture index],[texture atlas width],[texture atlas height]
[texture type name],[start x],[start y],[width][height]
[texture type name],[start x],[start y],[width][height]
...
```
So, this file reflects structure of real texture atlas which contains multiple images (_texture types_) inside. Texture atlas has to be included inside mapcss with the following rule:

`@texture url("default.detail-atlas.txt");`

## Runtime usage

In runtime, utymap uses this information to build proper texture coordinates and exposes this information through mesh's uvMap field:

```C++
void MeshBuilder::writeTextureMappingInfo(Mesh &mesh, const AppearanceOptions &appearanceOptions) const {
  ...
  mesh.uvMap.push_back(static_cast<int>(mesh.uvs.size()));
  mesh.uvMap.push_back(appearanceOptions.textureId);
  mesh.uvMap.push_back(appearanceOptions.textureRegion.atlasWidth);
  mesh.uvMap.push_back(appearanceOptions.textureRegion.atlasHeight);
  mesh.uvMap.push_back(appearanceOptions.textureRegion.x);
  mesh.uvMap.push_back(appearanceOptions.textureRegion.y);
  mesh.uvMap.push_back(appearanceOptions.textureRegion.width);
  mesh.uvMap.push_back(appearanceOptions.textureRegion.height);
}
```
