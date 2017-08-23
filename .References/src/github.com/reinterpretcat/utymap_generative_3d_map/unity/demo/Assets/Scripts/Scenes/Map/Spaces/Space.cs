using System;
using Assets.Scripts.Core.Plugins;
using Assets.Scripts.Scenes.Map.Animations;
using Assets.Scripts.Scenes.Map.Gestures;
using Assets.Scripts.Scenes.Map.Plugins;
using Assets.Scripts.Scenes.Map.Tiling;
using UnityEngine;
using UtyMap.Unity;

namespace Assets.Scripts.Scenes.Map.Spaces
{
    internal abstract class Space : IDisposable
    {
        public readonly TileController TileController;
        public readonly GestureStrategy GestureStrategy;
        public abstract SpaceAnimator Animator { get; protected set; }

        protected readonly MaterialProvider MaterialProvider;

        protected readonly Transform Target;
        protected readonly Transform Pivot;
        protected readonly Camera Camera;
        protected readonly Transform Light;

        public Space(TileController tileController, GestureStrategy gestureStrategy, 
            Transform target, MaterialProvider materialProvider)
        {
            Target = target;
            TileController = tileController;
            MaterialProvider = materialProvider;
            GestureStrategy = gestureStrategy;

            Pivot = tileController.Pivot;
            Camera = tileController.Pivot.Find("Camera").GetComponent<Camera>();
            Light = tileController.Pivot.Find("Directional Light");
        }

        /// <summary> Called when space is entered. </summary>
        protected abstract void OnEnter(GeoCoordinate coordinate, bool isFromTop);

        /// <summary> Called when space is exited </summary>
        protected abstract void OnExit();

        /// <summary> Enters space from top. </summary>
        public void EnterTop(GeoCoordinate coordinate)
        {
            SetDefaults();
            OnEnter(coordinate, true);
        }

        /// <summary> Enters space from bottom. </summary>
        public void EnterBottom(GeoCoordinate coordinate)
        {
            SetDefaults();
            OnEnter(coordinate, false);
        }

        /// <summary> Notifies space about time since last update. </summary>
        public void Update(float deltaTime)
        {
            Animator.Update(deltaTime);
            TileController.Update(Target);
        }

        /// <summary> Performs cleanup actions. </summary>
        public void Leave()
        {
            Animator.Cancel();
            TileController.Dispose();
            Target.gameObject.SetActive(false);

            OnExit();
        }

        /// <inheritdoc />
        public void Dispose()
        {
            TileController.Dispose();
        }

        /// <summary> Performs init actions. </summary>
        private void SetDefaults()
        {
            Target.gameObject.SetActive(true);
            
            Camera.transform.localPosition = Vector3.zero;
            Camera.transform.localRotation = Quaternion.Euler(0, 0, 0);
            Pivot.localPosition = Vector3.zero;
            Pivot.localRotation = Quaternion.Euler(0, 0, 0);
            Light.localPosition = Vector3.zero;
            Light.localRotation = Quaternion.Euler(0, 0, 0);

            Camera.fieldOfView = TileController.FieldOfView;
        }
    }
}
