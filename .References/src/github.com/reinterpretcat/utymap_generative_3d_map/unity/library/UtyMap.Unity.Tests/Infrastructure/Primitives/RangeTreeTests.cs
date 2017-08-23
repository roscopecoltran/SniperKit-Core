using System.Linq;
using NUnit.Framework;
using UtyMap.Unity.Infrastructure.Primitives;

namespace UtyMap.Unity.Tests.Infrastructure.Primitives
{
    [TestFixture]
    class RangeTreeTests
    {
        [Test]
        public void CanHandleOneElement()
        {
            var tree = new RangeTree<float, string>();

            tree.Add(new RangeValuePair<float, string>(0, 10, "1"));

            Assert.AreEqual("1", tree[9].First().Value);
        }

        [Test]
        public void CanHandleNoElements()
        {
            var tree = new RangeTree<float, string>();

            tree.Add(new RangeValuePair<float, string>(0, 10, "1"));

            Assert.IsFalse(tree[11].Any());
        }

        [Test]
        public void CanHandleMoreThanOne()
        {
            var tree = new RangeTree<float, string>();

            tree.Add(new RangeValuePair<float, string>(0, 10, "1"));
            tree.Add(new RangeValuePair<float, string>(10, 20, "2"));

            Assert.AreEqual("1", tree[5].First().Value);
            Assert.AreEqual("2", tree[11].First().Value);
        }

        [Test]
        public void CanGetMinValue()
        {
            var tree = new RangeTree<float, string>();
            tree.Add(new RangeValuePair<float, string>(100, 200, "1"));
            tree.Add(new RangeValuePair<float, string>(300, 400, "2"));
            tree.Rebuild();

            var value = tree.Min;

            Assert.AreEqual(100, value);
        }

        [Test]
        public void CanGetMaxValue()
        {
            var tree = new RangeTree<float, string>();
            tree.Add(new RangeValuePair<float, string>(100, 200, "1"));
            tree.Add(new RangeValuePair<float, string>(300, 400, "2"));
            tree.Rebuild();

            var value = tree.Max;

            Assert.AreEqual(400, value);
        }
    }
}