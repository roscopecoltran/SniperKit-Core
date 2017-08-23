using System;

namespace UtyMap.Unity.Infrastructure.Primitives
{
    /// <summary> Represents tagged union. </summary>
    public struct Union<T1, T2>
    {
        private readonly T1 _item1;
        private readonly T2 _item2;
        private readonly int _tag;

        /// <summary> Creates <see cref="Union"/> which holds T1 type value. </summary>
        public Union(T1 item)
        {
            _item1 = item;
            _item2 = default(T2);
            _tag = 0;
        }

        /// <summary> Creates <see cref="Union"/> which holds T2 type value. </summary>
        public Union(T2 item)
        {
            _item1 = default(T1);
            _item2 = item;
            _tag = 1;
        }

        /// <summary> Matches function to stored value. </summary>
        public T Match<T>(Func<T1, T> func1, Func<T2, T> func2)
        {
            switch (_tag)
            {
                case 0: return func1(_item1);
                case 1: return func2(_item2);
                default:
                    throw new InvalidOperationException("Unrecognized tag value: " + _tag);
            }
        }

        /// <summary> Matches action to stored value. </summary>
        public void Match(Action<T1> action1, Action<T2> action2)
        {
            switch (_tag)
            {
                case 0: 
                    action1(_item1);
                    break;
                case 1: 
                    action2(_item2);
                    break;
                default:
                    throw new InvalidOperationException("Unrecognized tag value: " + _tag);
            }
        }
    }
}
