using System;
using System.Collections.Generic;
using Assets.Scripts.Core;
using Assets.Scripts.Core.Plugins;
using Assets.Scripts.Scenes.Map.Gestures;
using Assets.Scripts.Scenes.Map.Plugins;
using Assets.Scripts.Scenes.Map.Spaces;
using Assets.Scripts.Scenes.Map.Tiling;
using TouchScript.Gestures.TransformGestures;
using UnityEngine;
using UtyMap.Unity;
using UtyMap.Unity.Animations.Time;
using UtyMap.Unity.Data;
using UtyMap.Unity.Infrastructure.Primitives;

using Component = UtyDepend.Component;
using Space = Assets.Scripts.Scenes.Map.Spaces.Space;

namespace Assets.Scripts.Scenes.Map
{
    /// <summary> Provides an entry point for building the map and reacting on user interaction with it. </summary>
    internal class MapBehaviour : MonoBehaviour
    {
        #region User controlled settings

        public double StartLatitude = 52.53171;
        public double StartLongitude = 13.38721;
        [Range(1, 16)]
        public float StartZoom = 1;

        public Transform Planet;
        public Transform Surface;
        public Transform Pivot;
        public Camera Camera;

        public ScreenTransformGesture TwoFingerMoveGesture;
        public ScreenTransformGesture ManipulationGesture;

        public bool ShowPosition = true;

        #endregion

        private CompositionRoot _compositionRoot;
        private int _currentSpaceIndex;
        private List<Range<int>> _lods;
        private List<Space> _spaces;

        #region Unity lifecycle methods

        #region Public properties

        /// <summary> Gets current tile controller. </summary>
        public TileController TileController { get { return _spaces[_currentSpaceIndex].TileController; } }

        #endregion

        void Start()
        {
            _compositionRoot = InitTask.Run((container, config) =>
            {
                container
                    .Register(Component.For<Stylesheet>().Use<Stylesheet>(@"mapcss/default/index.mapcss"))
                    .Register(Component.For<MaterialProvider>().Use<MaterialProvider>())
                    .Register(Component.For<GameObjectBuilder>().Use<GameObjectBuilder>())
                    .Register(Component.For<IElementBuilder>().Use<LabelElementBuilder>().Named("label"));
            });

            var mapDataStore = _compositionRoot.GetService<IMapDataStore>();
            var stylesheet = _compositionRoot.GetService<Stylesheet>();
            var materialProvider = _compositionRoot.GetService<MaterialProvider>();
            var startCoord = new GeoCoordinate(StartLatitude, StartLongitude);

            // scaled radius of Earth in meters, approx. 1:1000
            const float planetRadius = 6371f;
            const float surfaceScale = 0.01f;
            const float detailScale = 1f;

            _lods = new List<Range<int>> { new Range<int>(1, 8), new Range<int>(9, 15), new Range<int>(16, 16) };

            var sphereController = new SphereTileController(mapDataStore, stylesheet, ElevationDataType.Flat, Pivot, _lods[0], planetRadius);
            var surfaceController = new SurfaceTileController(mapDataStore, stylesheet, ElevationDataType.Flat, Pivot, _lods[1], startCoord, surfaceScale, 1000);
            var detailController = new SurfaceTileController(mapDataStore, stylesheet, ElevationDataType.Flat, Pivot,_lods[2], startCoord, detailScale, 500);

            var sphereGestures = new SphereGestureStrategy(sphereController, TwoFingerMoveGesture, ManipulationGesture, planetRadius);
            var surfaceGestures = new SurfaceGestureStrategy(surfaceController, TwoFingerMoveGesture, ManipulationGesture);
            var detailGestures = new SurfaceGestureStrategy(detailController, TwoFingerMoveGesture, ManipulationGesture);

            _spaces = new List<Space>
            {
                new SphereSpace(sphereController, sphereGestures, Planet, materialProvider),
                new SurfaceSpace(surfaceController, surfaceGestures, Surface, materialProvider),
                new SurfaceSpace(detailController, detailGestures, Surface, materialProvider)
            };

            DoTransition(startCoord, StartZoom);
        }

        void OnEnable()
        {
            TwoFingerMoveGesture.Transformed += TwoFingerTransformHandler;
            ManipulationGesture.Transformed += ManipulationTransformedHandler;
        }

        void OnDisable()
        {
            TwoFingerMoveGesture.Transformed -= TwoFingerTransformHandler;
            ManipulationGesture.Transformed -= ManipulationTransformedHandler;
        }

        void Update()
        {
            var space = _spaces[_currentSpaceIndex];

            space.Update(Time.deltaTime);

            // check whether space change is needed
            if (space.TileController.IsAboveMax && _currentSpaceIndex > 0)
                DoTransition(space, _spaces[--_currentSpaceIndex]);
            else if (space.TileController.IsBelowMin && _currentSpaceIndex != _spaces.Count - 1)
                DoTransition(space, _spaces[++_currentSpaceIndex]);
        }

        void OnGUI()
        {
            if (ShowPosition)
            {
                var tileController = _spaces[_currentSpaceIndex].TileController;
                var labelText = String.Format("Position: {0}\nZoom: {1}",
                    tileController.Coordinate,
                    tileController.Zoom);

                GUI.contentColor = Color.red;
                GUI.Label(new Rect(0, 0, Screen.width, Screen.height), labelText);
            }
        }

        void OnDestroy()
        {
            foreach (var space in _spaces)
                space.Dispose();

            _compositionRoot.Dispose();
        }

        #endregion

        #region Touch handles

        private void ManipulationTransformedHandler(object sender, EventArgs e)
        {
            _spaces[_currentSpaceIndex].GestureStrategy.OnManipulationTransform(Pivot, Camera.transform);
        }

        private void TwoFingerTransformHandler(object sender, EventArgs e)
        {
            _spaces[_currentSpaceIndex].GestureStrategy.OnTwoFingerTransform(Pivot, Camera.transform);
        }

        #endregion

        #region Space change logic

        /// <summary> Performs initial space transition based on zoom level and lod ranges. </summary>
        private void DoTransition(GeoCoordinate coordinate, float zoom)
        {
            for (int i = 0; i < _lods.Count; ++i)
                if (_lods[i].Contains((int)zoom))
                {
                    _currentSpaceIndex = i;
                    break;
                }

            Space to = _spaces[_currentSpaceIndex];
            // enter from top by default
            to.EnterTop(coordinate);
            // make instant animation
            to.Animator.AnimateTo(coordinate, zoom, TimeSpan.Zero, new LinearInterpolator());
        }

        /// <summary> Performs transition from one space to another. </summary>
        private void DoTransition(Space from, Space to)
        {
            bool isFromTopSpace = from.TileController.LodRange.Maximum < to.TileController.LodRange.Maximum;
            bool hadRunningAnimation = from.Animator.IsRunningAnimation;
            var coordinate = from.TileController.Coordinate;

            // exit from old space
            from.Leave();

            // make transition
            if (isFromTopSpace) 
                to.EnterTop(coordinate);
            else 
                to.EnterBottom(coordinate);

            // continue old space animation if it is running
            if (hadRunningAnimation)
                to.Animator.ContinueFrom(from.Animator);
            // otherwise make instant transition
            else
            {
                var zoom = isFromTopSpace
                    ? to.TileController.LodRange.Minimum
                    : to.TileController.LodRange.Maximum + 0.99f;
                to.Animator.AnimateTo(coordinate, zoom, TimeSpan.Zero, new LinearInterpolator());
            }
        }

        #endregion
    }
}
