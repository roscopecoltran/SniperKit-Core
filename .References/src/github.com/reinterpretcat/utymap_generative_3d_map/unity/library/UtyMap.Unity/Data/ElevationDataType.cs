namespace UtyMap.Unity.Data
{
    /// <summary> Specifies which elevation data will be used. </summary>
    public enum ElevationDataType
    {
        /// <summary> Flat elevation. </summary>
        Flat = 0,
        /// <summary> Srtm elevation from hgt files. </summary>
        Srtm,
        /// <summary> Elevation from text files in grid format. </summary>
        Grid
    }
}
