using System;
using System.Collections.Generic;
using Assets.Scripts.Core.Plugins;
using Assets.Scripts.UI;
using UnityEngine;
using UtyMap.Unity;
using UtyMap.Unity.Utils;

namespace Assets.Scripts.Scenes.Map.Plugins
{
    /// <summary> Builds a label. </summary>
    internal sealed class LabelElementBuilder : IElementBuilder
    {
        private readonly MapBehaviour _mapBehaviour;
        private readonly Dictionary<string, Font> _fontCache = new Dictionary<string, Font>();

        public LabelElementBuilder()
        {
            _mapBehaviour = GameObject.FindObjectOfType<MapBehaviour>();
        }

        /// <inheritdoc />
        public string Name { get { return "label"; } }

        /// <inheritdoc />
        public GameObject Build(Tile tile, Element element)
        {
            var gameObject = new GameObject(GetName(element));
            gameObject.transform.SetParent(tile.GameObject.transform);

            return element.Styles.ContainsKey("type") && element.Styles["type"] == "flat"
                ? BuildFlatText(gameObject, element)
                : BuildSphereText(gameObject, element);
        }

        /// <summary> Builds text on sphere. </summary>
        private GameObject BuildSphereText(GameObject gameObject, Element element)
        {
            var sphereText = gameObject.AddComponent<SphereText>();

            sphereText.Coordinate = element.Geometry[0];
            // NOTE should be in sync with sphere size and offsetted polygons
            sphereText.Radius = 6371f + 25f;

            var font = new FontWrapper(element.Styles, _fontCache);
            sphereText.font = font.Font;
            sphereText.fontSize = font.Size;
            sphereText.color = font.Color;
            sphereText.text = GetText(element);
            sphereText.alignment = TextAnchor.MiddleCenter;
           
            return gameObject;
        }

        /// <summary> Builds flat text. </summary>
        private GameObject BuildFlatText(GameObject gameObject, Element element)
        {
            var text = gameObject.AddComponent<TextMesh>();

            var controller = _mapBehaviour.TileController;
            var height = controller.HeightRange.Minimum + 1;
            gameObject.transform.position = controller.Projection.Project(element.Geometry[0], height);
            gameObject.transform.rotation = Quaternion.Euler(90, 0, 0);

            var font = new FontWrapper(element.Styles, _fontCache);
            //text.font = font.Font;
            text.fontSize = font.Size;
            text.color = font.Color;
            text.text = GetText(element);
            text.anchor = TextAnchor.MiddleCenter;
            text.alignment = TextAlignment.Center;

            gameObject.transform.localScale *= font.Scale;

            return gameObject;
        }

        private static string GetText(Element element)
        {
            return element.Tags["name"];
        }

        private static string GetName(Element element)
        {
            return String.Format("place:{0}[{1}]", element.Id, element.Tags["name"]);
        }

        private struct FontWrapper
        {
            public readonly Font Font;
            public readonly int Size;
            public readonly Color Color;
            public readonly float Scale;

            public FontWrapper(Dictionary<string, string> styles, Dictionary<string, Font> fontCache)
            {
                Size = int.Parse(styles["font-size"]);
                var fontName = styles["font-name"];
                if (!fontCache.ContainsKey(fontName))
                    fontCache.Add(fontName, Font.CreateDynamicFontFromOSFont(fontName, Size));
                Font = fontCache[fontName];
                Color = ColorUtils.FromUnknown(styles["font-color"]);

                if (!styles.ContainsKey("font-scale") || !float.TryParse(styles["font-scale"], out Scale))
                    Scale = 1;
            }
        }
    }
}
