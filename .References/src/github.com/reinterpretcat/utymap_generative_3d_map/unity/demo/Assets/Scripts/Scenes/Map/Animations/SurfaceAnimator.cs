using System;
using System.Collections.Generic;
using Assets.Scripts.Scenes.Map.Tiling;
using UnityEngine;
using UtyMap.Unity;
using UtyMap.Unity.Animations.Time;
using Animation = UtyMap.Unity.Animations.Animation;

namespace Assets.Scripts.Scenes.Map.Animations
{
    /// <summary> Handles surface animations. </summary>
    internal sealed class SurfaceAnimator : SpaceAnimator
    {
        public SurfaceAnimator(TileController tileController) : base(tileController)
        {
        }

        /// <inheritdoc />
        protected override Animation CreateAnimationTo(GeoCoordinate coordinate, float zoom, TimeSpan duration, ITimeInterpolator timeInterpolator)
        {
            var position = Pivot.localPosition;
            var position2D = TileController.Projection.Project(coordinate, 0);
            return CreatePathAnimation(Pivot, duration, timeInterpolator, new List<Vector3>()
            {
                position,
                new Vector3(position2D.x, TileController.GetHeight(zoom), position2D.z)
            });
        }
    }
}
