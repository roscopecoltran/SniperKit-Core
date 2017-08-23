# Understanding data model

Data model is important to understand how utymap renders scene from raw vector map data.

## Table of content

- [Elements](#elements)
- [Tags](#tags)
- [Data storage](#data-storage)
- [Optimizations](#optimizations)
    - [Mesh cache](#mesh-cache)
    - [Rendering less](#rendering-less)
    - [Compression](#compression)

## Elements

Elements (Node, Way, Area, Relation) are basic components of UtyMap's conceptual data model which is inspired by OpenStreetMap (see [osm docs](http://wiki.openstreetmap.org/wiki/Elements))
In general, data model consists of the following primitives:
* **Node** - point specified by latitude and longitude. Used to define some POI (Place Of Interest) on map.
* **Way** - open polyline specified by two or more points. Common usage is roads.
* **Area** - closed polygon specified by three or more points. Widely used for simple buildings, terrain regions (green zones, water, etc.)
* **Relation** - union of nodes, ways, areas or relations. Used for specifying complex shaped buildings, terrain regions, etc.

Each primitive has unique ID and set of tags. Tags define actual object represented by element data.
Based on their location, all elements are grouped inside one or more **quadkey**. Each quadkey is defined by zoom (level of details), x, y (longitude and latitude). UtyMap uses [Bing Maps](https://msdn.microsoft.com/en-us/library/bb259689.aspx) quadkey schema.

## Tags

Tag is just key-value pair, e.g. _building=yes_ or _kind=forest_. Actual values depend on raw data schema used by specific data source meaning that active mapcss __has to__ use this schema.
For example, OpenStreetMap defines some park areas as _leisure=park_, but mapzen uses _kind=park_.
Utymap does not perform any tag normalization in source code, instead, different mapcss styles has to be specified and used.

## Data storage
Currently, utymap can import data from the following formats:
* osm pbf
* osm xml
* geojson
* shape

During import, data is converted into internal format which supports simple geo spatial requests. There are two types of data storage:
* **in-memory** - stores all data in memory and all changes are discarded after application is unloaded. This type is useful for storing all changes applied to map data temporary (e.g. adding or removing new buildings).
* **persistent** - stores data on disc in UtyMap's own format. This type is used for importing large regions once as import operation may take a lot of time and consumes RAM. Hint: you can import region on one device and copy to another.

You can use more than one data storage of given types (actually, there is no limit). It might be useful to implement custom save or role management systems.

## Optimizations

### Mesh cache
Version 2.1. adds mesh caching mechanism which stores generated terrain meshes on disk to avoid expensive calculations for unchanged data. It is enabled by default and can be disabled if needed.

### Rendering less
Another optimization is reducing amount of data rendered by default. Can be easily done by revising mapcss rules.

### Compression
This is in TODO list. General idea is to use compression for all persistent data.
