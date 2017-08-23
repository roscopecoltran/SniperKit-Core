using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace UtyMap.Unity.Infrastructure.Primitives
{
    /// <summary>
    ///     The standard range tree implementation. Keeps a root node and
    ///     forwards all queries to it.
    ///     Whenenver new items are added or items are removed, the tree
    ///     goes "out of sync" and is rebuild when it's queried next.
    /// </summary>
    /// <typeparam name="TKey">The type of the range.</typeparam>
    /// <typeparam name="TValue">The type of the data items.</typeparam>
    public class RangeTree<TKey, TValue> : ICollection<RangeValuePair<TKey, TValue>>
    {
        private bool _autoRebuild;
        private readonly IComparer<TKey> _comparer;
        private bool _isInSync;
        private List<RangeValuePair<TKey, TValue>> _items;
        private RangeTreeNode<TKey, TValue> _root;

        /// <summary>
        ///     Initializes an empty tree.
        /// </summary>
        public RangeTree() : this(Comparer<TKey>.Default)
        {
        }

        /// <summary>
        ///     Initializes an empty tree.
        /// </summary>
        public RangeTree(IComparer<TKey> comparer)
        {
            _comparer = comparer;
            _autoRebuild = true;
            Clear();
        }

        /// <summary>
        ///     Whether the tree should be rebuild automatically. Defaults to true.
        /// </summary>
        public bool AutoRebuild
        {
            get { return _autoRebuild; }
            set { _autoRebuild = value; }
        }

        /// <summary>
        ///     Whether the tree is currently in sync or not. If it is "out of sync"
        ///     you can either rebuild it manually (call Rebuild) or let it rebuild
        ///     automatically when you query it next.
        /// </summary>
        public bool IsInSync
        {
            get { return _isInSync; }
        }

        /// <summary>
        ///     Maximum key found in the tree
        /// </summary>
        public TKey Max
        {
            get { return _root.Max; }
        }

        /// <summary>
        ///     Minimum key found in the tree
        /// </summary>
        public TKey Min
        {
            get { return _root.Min; }
        }

        /// <summary>
        ///     All items of the tree.
        /// </summary>
        public IEnumerable<TValue> Values
        {
            get { return _items.Select(i => i.Value); }
        }

        /// <summary>
        ///     Performans a range query.
        ///     All items with overlapping ranges are returned.
        /// </summary>
        public IEnumerable<RangeValuePair<TKey, TValue>> this[TKey from, TKey to]
        {
            get
            {
                if (!_isInSync && _autoRebuild)
                    Rebuild();

                return _root.Query(from, to);
            }
        }

        /// <summary>
        ///     Performans a "stab" query with a single value.
        ///     All items with overlapping ranges are returned.
        /// </summary>
        public IEnumerable<RangeValuePair<TKey, TValue>> this[TKey value]
        {
            get
            {
                if (!_isInSync && _autoRebuild)
                    Rebuild();

                return _root.Query(value);
            }
        }

        /// <summary>
        ///     Count of all items.
        /// </summary>
        public int Count
        {
            get { return _items.Count; }
        }

        public bool IsReadOnly
        {
            get { return false; }
        }

        /// <summary>
        ///     Adds the specified item. Tree will go out of sync.
        /// </summary>
        public void Add(RangeValuePair<TKey, TValue> item)
        {
            Add(item.From, item.To, item.Value);
        }

        /// <summary>
        ///     Clears the tree (removes all items).
        /// </summary>
        public void Clear()
        {
            _root = new RangeTreeNode<TKey, TValue>(_comparer);
            _items = new List<RangeValuePair<TKey, TValue>>();
            _isInSync = true;
        }

        public bool Contains(RangeValuePair<TKey, TValue> item)
        {
            return _items.Contains(item);
        }

        public void CopyTo(RangeValuePair<TKey, TValue>[] array, int arrayIndex)
        {
            _items.CopyTo(array, arrayIndex);
        }

        public IEnumerator<RangeValuePair<TKey, TValue>> GetEnumerator()
        {
            if (!_isInSync && _autoRebuild)
                Rebuild();

            return _items.GetEnumerator();
        }

        /// <summary>
        ///     Removes the specified item. Tree will go out of sync.
        /// </summary>
        public bool Remove(RangeValuePair<TKey, TValue> item)
        {
            var removed = _items.Remove(item);
            _isInSync = _isInSync && !removed;
            return removed;
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        /// <summary>
        ///     Adds the specified item. Tree will go out of sync.
        /// </summary>
        public void Add(TKey from, TKey to, TValue value)
        {
            if (_comparer.Compare(from, to) == 1)
                throw new ArgumentOutOfRangeException();

            _isInSync = false;
            _items.Add(new RangeValuePair<TKey, TValue>(from, to, value));
        }

        /// <summary>
        ///     Rebuilds the tree if it is out of sync.
        /// </summary>
        public void Rebuild()
        {
            if (_isInSync)
                return;

            if (_items.Count > 0)
                _root = new RangeTreeNode<TKey, TValue>(_items, _comparer);
            else
                _root = new RangeTreeNode<TKey, TValue>(_comparer);
            _isInSync = true;
            _items.TrimExcess();
        }
    }

    /// <summary>
    ///     A node of the range tree. Given a list of items, it builds
    ///     its subtree. Also contains methods to query the subtree.
    ///     Basically, all interval tree logic is here.
    /// </summary>
    public class RangeTreeNode<TKey, TValue> : IComparer<RangeValuePair<TKey, TValue>>
    {
        private readonly IComparer<TKey> _comparer;
        private readonly TKey _center;
        private readonly RangeValuePair<TKey, TValue>[] _items;
        private readonly RangeTreeNode<TKey, TValue> _leftNode;
        private readonly RangeTreeNode<TKey, TValue> _rightNode;

        /// <summary>
        ///     Initializes an empty node.
        /// </summary>
        /// <param name="comparer">The comparer used to compare two items.</param>
        public RangeTreeNode(IComparer<TKey> comparer)
        {
            _comparer = comparer ?? Comparer<TKey>.Default;

            _center = default(TKey);
            _leftNode = null;
            _rightNode = null;
            _items = null;
        }

        /// <summary>
        ///     Initializes a node with a list of items, builds the sub tree.
        /// </summary>
        /// <param name="comparer">The comparer used to compare two items.</param>
        public RangeTreeNode(IList<RangeValuePair<TKey, TValue>> items, IComparer<TKey> comparer)
        {
            _comparer = comparer ?? Comparer<TKey>.Default;

            // first, find the median
            var endPoints = new List<TKey>(items.Count * 2);
            foreach (var item in items)
            {
                endPoints.Add(item.From);
                endPoints.Add(item.To);
            }
            endPoints.Sort(_comparer);

            // the median is used as center value
            _center = endPoints[endPoints.Count / 2];

            var inner = new List<RangeValuePair<TKey, TValue>>();
            var left = new List<RangeValuePair<TKey, TValue>>();
            var right = new List<RangeValuePair<TKey, TValue>>();

            // iterate over all items
            // if the range of an item is completely left of the center, add it to the left items
            // if it is on the right of the center, add it to the right items
            // otherwise (range overlaps the center), add the item to this node's items
            foreach (var o in items)
            {
                if (_comparer.Compare(o.To, _center) < 0)
                    left.Add(o);
                else if (_comparer.Compare(o.From, _center) > 0)
                    right.Add(o);
                else
                    inner.Add(o);
            }

            // sort the items, this way the query is faster later on
            if (inner.Count > 0)
            {
                if (inner.Count > 1)
                    inner.Sort(this);
                _items = inner.ToArray();
            }
            else
            {
                _items = null;
            }

            // create left and right nodes, if there are any items
            if (left.Count > 0)
                _leftNode = new RangeTreeNode<TKey, TValue>(left, _comparer);
            if (right.Count > 0)
                _rightNode = new RangeTreeNode<TKey, TValue>(right, _comparer);
        }

        public TKey Max
        {
            get
            {
                if (_rightNode != null)
                    return _rightNode.Max;
                if (_items != null)
                    return _items.Max(i => i.To);
                return default(TKey);
            }
        }

        public TKey Min
        {
            get
            {
                if (_leftNode != null)
                    return _leftNode.Min;
                if (_items != null)
                    return _items.Min(i => i.From);
                return default(TKey);
            }
        }

        /// <summary>
        ///     Returns less than 0 if this range's From is less than the other, greater than 0 if greater.
        ///     If both are equal, the comparison of the To values is returned.
        ///     0 if both ranges are equal.
        /// </summary>
        /// <param name="y">The other.</param>
        /// <returns></returns>
        int IComparer<RangeValuePair<TKey, TValue>>.Compare(RangeValuePair<TKey, TValue> x,
            RangeValuePair<TKey, TValue> y)
        {
            var fromComp = _comparer.Compare(x.From, y.From);
            if (fromComp == 0)
                return _comparer.Compare(x.To, y.To);
            return fromComp;
        }


        /// <summary>
        ///     Performans a "stab" query with a single value.
        ///     All items with overlapping ranges are returned.
        /// </summary>
        public IEnumerable<RangeValuePair<TKey, TValue>> Query(TKey value)
        {
            var results = new List<RangeValuePair<TKey, TValue>>();

            // If the node has items, check for leaves containing the value.
            if (_items != null)
            {
                foreach (var o in _items)
                {
                    if (_comparer.Compare(o.From, value) > 0)
                        break;
                    if (_comparer.Compare(value, o.From) >= 0 && _comparer.Compare(value, o.To) <= 0)
                    {
                        results.Add(o);
                    }
                }
            }

            // go to the left or go to the right of the tree, depending
            // where the query value lies compared to the center
            var centerComp = _comparer.Compare(value, _center);
            if (_leftNode != null && centerComp < 0)
                results.AddRange(_leftNode.Query(value));
            else if (_rightNode != null && centerComp > 0)
                results.AddRange(_rightNode.Query(value));

            return results;
        }

        /// <summary>
        ///     Performans a range query.
        ///     All items with overlapping ranges are returned.
        /// </summary>
        public IEnumerable<RangeValuePair<TKey, TValue>> Query(TKey from, TKey to)
        {
            var results = new List<RangeValuePair<TKey, TValue>>();

            // If the node has items, check for leaves intersecting the range.
            if (_items != null)
            {
                foreach (var o in _items)
                {
                    if (_comparer.Compare(o.From, to) > 0)
                        break;
                    if (_comparer.Compare(to, o.From) >= 0 && _comparer.Compare(@from, o.To) <= 0)
                        results.Add(o);
                }
            }

            // go to the left or go to the right of the tree, depending
            // where the query value lies compared to the center
            if (_leftNode != null && _comparer.Compare(from, _center) < 0)
                results.AddRange(_leftNode.Query(from, to));
            if (_rightNode != null && _comparer.Compare(to, _center) > 0)
                results.AddRange(_rightNode.Query(from, to));

            return results;
        }
    }

    /// <summary>
    ///     Represents a range of values.
    ///     Both values must be of the same type and comparable.
    /// </summary>
    /// <typeparam name="TKey">Type of the values.</typeparam>
    public struct RangeValuePair<TKey, TValue>
    {
        public TKey From { get; private set; }
        public TKey To { get; private set; }
        public TValue Value { get; private set; }

        /// <summary>
        ///     Initializes a new <see cref="RangeValuePair&lt;TKey, TValue&gt;" /> instance.
        /// </summary>
        public RangeValuePair(TKey from, TKey to, TValue value) : this()
        {
            From = from;
            To = to;
            Value = value;
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return string.Format("[{0} - {1}] {2}", From, To, Value);
        }

        public override int GetHashCode()
        {
            var hash = 23;
            if (From != null)
                hash = hash * 37 + From.GetHashCode();
            if (To != null)
                hash = hash * 37 + To.GetHashCode();
            if (Value != null)
                hash = hash * 37 + Value.GetHashCode();
            return hash;
        }
    }
}
