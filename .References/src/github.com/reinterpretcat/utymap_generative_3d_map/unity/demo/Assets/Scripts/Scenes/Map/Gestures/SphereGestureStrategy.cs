using System;
using Assets.Scripts.Scenes.Map.Tiling;
using TouchScript.Gestures.TransformGestures;
using UnityEngine;

namespace Assets.Scripts.Scenes.Map.Gestures
{
    internal class SphereGestureStrategy : GestureStrategy
    {
        /// <summary> Value depends on radius and camera settings. </summary>
        private const float MagicAngleLimitCoeff = 2;

        private const float RotationSpeed = 1000f;
        private const float RotationMinSpeed = 1f;
        private const float RotationFactor = 0.01f;
        
        private const float ZoomSpeed = 1000f;
        private const float ZoomMinSpeed = 5f;
        private const float ZoomFactor = 1f;
        
        private readonly float _radius;

        public SphereGestureStrategy(TileController tileController,
                                     ScreenTransformGesture twoFingerMoveGesture,
                                     ScreenTransformGesture manipulationGesture,
                                     float radius) :
            base(tileController, twoFingerMoveGesture, manipulationGesture)
        {
            _radius = radius;
        }

        /// <inheritdoc />
        public override void OnManipulationTransform(Transform pivot, Transform camera)
        {
            var speed = Mathf.Max(RotationSpeed * InterpolateByZoom(RotationFactor), RotationMinSpeed);
            var rotation = Quaternion.Euler(
                -ManipulationGesture.DeltaPosition.y / Screen.height * speed,
                ManipulationGesture.DeltaPosition.x / Screen.width * speed,
                ManipulationGesture.DeltaRotation);

            SetRotation(pivot, camera, rotation);
        }

        /// <inheritdoc />
        public override void OnTwoFingerTransform(Transform pivot, Transform camera)
        {
            var speed = Mathf.Max(ZoomSpeed * InterpolateByZoom(ZoomFactor), ZoomMinSpeed);
            
            // zoom
            camera.transform.localPosition += Vector3.forward * (TwoFingerMoveGesture.DeltaScale - 1f) * speed;

            // rotation
            var rotation = Quaternion.Euler(0, 0, TwoFingerMoveGesture.DeltaRotation / 5);

            SetRotation(pivot, camera, rotation);
        }

        /// <summary> Sets rotation to pivot with limit. </summary>
        private void SetRotation(Transform pivot, Transform camera, Quaternion rotation)
        {
            pivot.localRotation *= rotation;
            pivot.localEulerAngles = new Vector3(
                LimitAngle(pivot.eulerAngles.x, CalculateLimit(camera)),
                pivot.eulerAngles.y,
                LimitAngle(pivot.eulerAngles.z, 23.5f));
        }

        private float CalculateLimit(Transform camera)
        {
            var pole = new Vector3(0, _radius, 0);
            var center = Vector3.zero;
            var position = new Vector3(0, 0, Vector3.Distance(camera.transform.position, center));

            var a = Vector3.Distance(position, center);
            var b = Vector3.Distance(position, pole);
            var c = _radius;

            var cosine = (a * a + b * b - c * c) / (2 * a * b);
            return (float) Math.Acos(cosine) * Mathf.Rad2Deg * MagicAngleLimitCoeff;
        }
    }
}
