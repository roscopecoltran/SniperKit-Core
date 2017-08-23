# Understanding mapcss

[MapCss](http://wiki.openstreetmap.org/wiki/MapCSS/0.2) is used to specify which elements should be rendered, when and how.

## Table of content

- [Ideas](#ideas)
- [Runtime processing](#runtime-processing)
- [Tag evaluation](#tag-evaluation)

## Ideas

UtyMap does not follow current OSM's MapCss format in all details. Instead, it adapts the format by introducing new declarations, e.g.:

* __builders__ - comma separated list of builder names. Builder is essential concept in UtyMap implemented inside native library. In general, builder is responsible for building mesh using raw map data. By default, UtyMap provides various buit-in builders to generate world. However, application is not limited by built-in builders, it is possible to use custom ones. For details, please refer to builder specific sections.
* __color__ - color represented by color or gradient string.

So, there is no strict schema for declarations: you can use custom names for your builders or treat existing ones differently.

## Runtime processing

When UtyMap processes element, it goes through all MapCss rules in the order they are defined (similar to CSS). So, definition order is important as the same declarations are overridden. Then, it looks for builder declaration and invokes corresponding builder.

Please note, that mapcss is used for data import and rendering.

## Tag evaluation

Utymap supports tag evaluation by _eval_ keyword:

```
height: eval("tag('building:levels') * 3.2");
height: eval("tag('height') - tag('roof:height')");
roof-color: eval("tag('building:color')");
```

This is quite useful if actual property value (e.g. height) depends on element's tag values. It allows to avoid enormous conditional statements in source code. Please note, that syntax is quite limited so far.
