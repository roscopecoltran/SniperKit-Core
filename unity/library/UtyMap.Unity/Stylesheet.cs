namespace UtyMap.Unity
{
    /// <summary> Represents stylesheet. </summary>
    public class Stylesheet
    {
        public string Path { get; private set; }

        public Stylesheet(string path)
        {
            Path = path;
        }
    }
}
