using System.Collections.Generic;
using System.Threading;

namespace UtyMap.Unity.Infrastructure.Primitives
{
    public sealed class SafeDictionary<TKey, TValue>
    {
        private readonly ReaderWriterLockSlim _lock = new ReaderWriterLockSlim();
        private readonly Dictionary<TKey, TValue> _dictionary = new Dictionary<TKey, TValue>();

        public bool TryGetValue(TKey key, out TValue value)
        {

            try
            {
                _lock.EnterReadLock();
                return _dictionary.TryGetValue(key, out value);
            }
            finally
            {
                if (_lock.IsReadLockHeld) _lock.ExitReadLock();
            }
        }

        public bool TryAdd(TKey key, TValue value)
        {
            try
            {
                _lock.EnterWriteLock();
                if (_dictionary.ContainsKey(key))
                    return false;

                _dictionary.Add(key, value);
                return true;
            }
            finally
            {
                if (_lock.IsWriteLockHeld) _lock.ExitWriteLock();
            }
        }

        public bool TryRemove(TKey key)
        {
            try
            {
                _lock.EnterWriteLock();
                if (!_dictionary.ContainsKey(key))
                    return false;

                _dictionary.Remove(key);
                return true;
            }
            finally
            {
                if (_lock.IsWriteLockHeld) _lock.ExitWriteLock();
            }
        }

        public void Clear()
        {
            try
            {
                _lock.EnterWriteLock();
               _dictionary.Clear();
            }
            finally
            {
                if (_lock.IsWriteLockHeld) _lock.ExitWriteLock();
            }
        }
    }
}
