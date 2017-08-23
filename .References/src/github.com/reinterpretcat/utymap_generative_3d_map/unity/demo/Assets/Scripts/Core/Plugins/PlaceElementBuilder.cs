using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using UnityEngine;
using UtyDepend;
using UtyMap.Unity;
using UtyMap.Unity.Infrastructure.IO;

namespace Assets.Scripts.Core.Plugins
{
    /// <summary> Builds Place of Interest as cube primitive with texture from Element. </summary>
    internal sealed class PlaceElementBuilder : IElementBuilder
    {
        private const int TextureWidth = 1104;
        private const int TextureHeight = 1169;
        private const string IconImageStyleKey = "icon-image";
        private const string IconSchemaFile = "config/icon.schema.txt";
        private static readonly Regex IconSchemaRegex = new Regex(@"([a-zA-z_]*?): (\d+)_(\d+)_(\d+)_(\d+)");
        // "eval(\"'amenity_' + tag('amenity')\")"
        private static readonly Regex EvalIconRegex = new Regex(@"eval\(""'([^']*)'[^']*'([^']*)'\)""\)");

        private readonly MaterialProvider _materialProvider;
        private Dictionary<string, Rect> _iconMapping;

        [Dependency]
        public PlaceElementBuilder(MaterialProvider customizationService,
            IFileSystemService fileSystemService, IPathResolver pathResolver)
        {
            _materialProvider = customizationService;
            var iconMappingString = fileSystemService.ReadText(pathResolver.Resolve(IconSchemaFile));
            _iconMapping = CreateIconMapping(iconMappingString);
        }

        /// <inheritdoc />
        public string Name { get { return "place"; } }

        /// <inheritdoc />
        public GameObject Build(Tile tile, Element element)
        {
            GameObject gameObject = GameObject.CreatePrimitive(PrimitiveType.Cube);
            gameObject.name = GetName(element);
            
            var transform = gameObject.transform;
            transform.parent = tile.GameObject.transform;
            // NOTE We use this builder only for nodes, so we know that geometry is represented by single geocoordinate.
            transform.position = tile.Projection.Project(element.Geometry[0], GetMinHeight(element) + element.Heights[0]);
            transform.localScale = new Vector3(2, 2, 2);

            gameObject.GetComponent<MeshFilter>().mesh.uv = GetUV(element);
            gameObject.GetComponent<MeshRenderer>().sharedMaterial = GetMaterial(element);

            return gameObject;
        }

        private Vector2[] GetUV(Element element)
        {
            Rect rect = GetUvRect(element);

            var p0 = new Vector2(rect.xMin, rect.yMin);
            var p1 = new Vector2(rect.xMax, rect.yMin);
            var p2 = new Vector2(rect.xMin, rect.yMax);
            var p3 = new Vector2(rect.xMax, rect.yMax);

            // Imagine looking at the front of the cube, the first 4 vertices are arranged like so
            //   2 --- 3
            //   |     |
            //   |     |
            //   0 --- 1
            // then the UV's are mapped as follows
            //    2    3    0    1   Front
            //    6    7   10   11   Back
            //   19   17   16   18   Left
            //   23   21   20   22   Right
            //    4    5    8    9   Top
            //   15   13   12   14   Bottom
            return new[]
            {
                p0, p1, p2, p3,
                p2, p3, p2, p3,
                p0, p1, p0, p1,
                p0, p3, p1, p2,
                p0, p3, p1, p2,
                p0, p3, p1, p2
            };
        }

        private string GetName(Element element)
        {
            return String.Format("place:{0}[{1}]", element.Id,
                element.Tags.Aggregate("", (s, t) => s+=String.Format("{0}={1},", t.Key, t.Value)));
        }

        private float GetMinHeight(Element element)
        {
            return element.Styles.ContainsKey("min-height")
                ? float.Parse(element.Styles["min-height"])
                : 0;
        }

        private Material GetMaterial(Element element)
        {
            return _materialProvider.GetSharedMaterial("Materials/" + element.Styles["material"]);
        }

        private Rect GetUvRect(Element element)
        {
            if (!element.Styles.ContainsKey(IconImageStyleKey))
                return new Rect();

            var match = EvalIconRegex.Match(element.Styles[IconImageStyleKey]);
            if (!match.Success || match.Groups.Count != 3)
                return new Rect();

            var key = match.Groups[1].Value + element.Tags[match.Groups[2].Value];
            if (!_iconMapping.ContainsKey(key))
                return new Rect();

            var rect = _iconMapping[key];

            var leftBottom = new Vector2(rect.x / TextureWidth, rect.y / TextureHeight);
            var rightUpper = new Vector2((rect.x + rect.width) / TextureWidth, (rect.y + rect.height) / TextureHeight);

            return new Rect(leftBottom.x, leftBottom.y, rightUpper.x - leftBottom.x, rightUpper.y - leftBottom.y);
        }

        private static Dictionary<string, Rect> CreateIconMapping(string iconMappingString)
        {
            var iconMapping = new Dictionary<string, Rect>();
            foreach (var line in iconMappingString.Split('\n'))
            {
                var match = IconSchemaRegex.Match(line);
                if (match.Success)
                {
                    var key = match.Groups[1].Value;
                    var value = new Rect(
                        Int32.Parse(match.Groups[2].Value),
                        Int32.Parse(match.Groups[3].Value),
                        Int32.Parse(match.Groups[4].Value),
                        Int32.Parse(match.Groups[5].Value));
                    iconMapping.Add(key, value);
                }
            }
            return iconMapping;
        }
    }
}

