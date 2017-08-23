using Assets.Scripts.Scenes.Map.Tiling;
using TouchScript.Gestures.TransformGestures;
using UnityEngine;

namespace Assets.Scripts.Scenes.Map.Gestures
{
    internal class SurfaceGestureStrategy : GestureStrategy
    {
        private const float PanMaxSpeed = 1f;
        private const float PanMinSpeed = 0.005f;
        private const float PanFactor = 0.05f;

        private const float ZoomMaxSpeed = 100f;
        private const float ZoomMinSpeed = 1f;
        private const float ZoomFactor = 0.5f;

        private const float TintLimit= 22.5f;
        private const float TintSpeed = 0.1f;

        public SurfaceGestureStrategy(TileController tileController, 
                                      ScreenTransformGesture twoFingerMoveGesture,
                                      ScreenTransformGesture manipulationGesture) :
            base(tileController, twoFingerMoveGesture, manipulationGesture)
        {
        }

        public override void OnManipulationTransform(Transform pivot, Transform camera)
        {
            // pan
            var speed = Mathf.Max(PanMaxSpeed * InterpolateByZoom(PanFactor), PanMinSpeed);
            pivot.localPosition += new Vector3(ManipulationGesture.DeltaPosition.x, 0, ManipulationGesture.DeltaPosition.y) * -speed;
        }

        public override void OnTwoFingerTransform(Transform pivot, Transform camera)
        {
            // tint
            if (SetTint(pivot, camera))
                return;

            // zoom
            var speed = Mathf.Max(ZoomMaxSpeed * InterpolateByZoom(ZoomFactor), ZoomMinSpeed);
            pivot.localPosition += Vector3.up * (1 - TwoFingerMoveGesture.DeltaScale) * speed;

            // rotation
            pivot.localRotation *= Quaternion.Euler(0, TwoFingerMoveGesture.DeltaRotation / 5, 0);
        }

        /// <remarks> Experimental. Should be replaced with custom gesture. </remarks>
        private bool SetTint(Transform pivot, Transform camera)
        {
            var pointer1 = TwoFingerMoveGesture.ActivePointers[0];
            var pointer2 = TwoFingerMoveGesture.ActivePointers[1];

            var delta1 = pointer1.Position - pointer1.PreviousPosition;
            var delta2 = pointer2.Position - pointer2.PreviousPosition;

            // different direction
            if (delta1.y < 0 != delta2.y < 0)
                return false;

            // ignore small values
            if (Mathf.Abs(delta1.y) < 2f || Mathf.Abs(delta2.y) < 2f)
                return false;

            // rather zoom than tint
            if (Mathf.Abs(delta1.x / delta1.y) > 0.5f || Mathf.Abs(delta2.x / delta2.y) > 0.5f)
                return false;

            // fingers are too far
            if (Mathf.Abs(delta1.y - delta2.y) > 1)
                return false;

            var angle = pivot.localRotation.eulerAngles.x + (delta1.y + delta2.y) * TintSpeed / 2;
            pivot.localRotation = Quaternion.Euler(LimitAngle(angle, TintLimit), 0, 0);
            return true;
        }
    }
}
