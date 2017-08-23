#ifndef CALLBACKS_HPP_DEFINED
#define CALLBACKS_HPP_DEFINED

#include <cstdint>

/// Callback which is called when directory should be created.
/// NOTE with C++11, directory cannot be created with header only libs.
typedef void OnNewDirectory(const char *path);

/// Callback which is called when mesh is built.
typedef void OnMeshBuilt(int tag,                                // a request tag
                         const char *name,                       // name
                         const double *vertices, int vertexSize, // vertices (x, y, elevation)
                         const int *triangles, int triSize,      // triangle indices
                         const int *colors, int colorSize,       // rgba colors
                         const double *uvs, int uvSize,          // absolute texture uvs
                         const int *uvMap, int uvMapSize);       // map with info about used atlas and texture region

/// Callback which is called when element is loaded.
typedef void OnElementLoaded(int tag,                                // a request tag
                             std::uint64_t id,                       // element id
                             const char **tags, int tagsSize,        // tags
                             const double *vertices, int vertexSize, // vertices (x, y, elevation)
                             const char **style, int styleSize);     // mapcss styles (key, value)

/// Callback which is called when error is occured.
typedef void OnError(const char *errorMessage);

#endif // CALLBACKS_HPP_DEFINED
