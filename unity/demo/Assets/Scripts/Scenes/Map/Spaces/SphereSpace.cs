using Assets.Scripts.Core.Plugins;
using Assets.Scripts.Scenes.Map.Animations;
using Assets.Scripts.Scenes.Map.Gestures;
using Assets.Scripts.Scenes.Map.Plugins;
using Assets.Scripts.Scenes.Map.Tiling;
using UnityEngine;
using UtyMap.Unity;

namespace Assets.Scripts.Scenes.Map.Spaces
{
    internal sealed class SphereSpace : Space
    {
        /// <inheritdoc />
        public override SpaceAnimator Animator { get; protected set; }

        public SphereSpace(SphereTileController tileController, SphereGestureStrategy gestureStrategy, 
            Transform planet, MaterialProvider materialProvider) :
                base(tileController, gestureStrategy, planet, materialProvider)
        {
            Animator = new SphereAnimator(tileController);
        }

        protected override void OnEnter(GeoCoordinate coordinate, bool isFromTop)
        {
            Camera.GetComponent<Skybox>().material = MaterialProvider.GetSharedMaterial(@"Skyboxes/Space/Skybox");

            Pivot.rotation = Quaternion.Euler(new Vector3((float)coordinate.Latitude, 270 - (float)coordinate.Longitude, 0));
            Camera.transform.localPosition = new Vector3(0, 0, isFromTop
                ? -TileController.HeightRange.Maximum
                : -TileController.HeightRange.Minimum);

            TileController.Update(Target);
        }

        protected override void OnExit()
        {
        }
    }
}
