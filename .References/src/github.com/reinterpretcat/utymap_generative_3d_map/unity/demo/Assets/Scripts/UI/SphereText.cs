using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UtyMap.Unity;

namespace Assets.Scripts.UI
{
    /// <summary> Draws text on sphere. </summary>
    [ExecuteInEditMode]
    public class SphereText : Text
    {
        public float Radius = 100f;
        public GeoCoordinate Coordinate;

        protected override void OnPopulateMesh(VertexHelper vh)
        {
            base.OnPopulateMesh(vh);

            var vertices = new List<UIVertex>();
            vh.GetUIVertexStream(vertices);

            var circumference = 2.0f / 360.0f * Mathf.PI * Radius;
            var rotation = Quaternion.Euler(new Vector3((float)-Coordinate.Latitude, 90f - (float) Coordinate.Longitude, 0));
            for (var i = 0; i < vertices.Count; i++)
            {
                var v = vertices[i];

                var xPercentCircumference = v.position.x / circumference;
                var yPercentCircumference = v.position.y / circumference;
                var zOffset = Quaternion.Euler(0, -xPercentCircumference, 0) * Vector3.forward;
                var yOffset = Quaternion.Euler(-yPercentCircumference, 0.0f, 0.0f) * Vector3.up;
                v.position = rotation * (yOffset * v.position.y + zOffset * Radius);

                vertices[i] = v;
            }
            
            vh.Clear();
            vh.AddUIVertexTriangleStream(vertices);
        }

        void LateUpdate()
        {
            rectTransform.sizeDelta = new Vector2(Radius, Radius);
        }
    }
}
