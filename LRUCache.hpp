#ifndef LRUCache_hpp
#define LRUCache_hpp

#include <list>
#include <map>
#include <iterator>

namespace ECE141 {
	template<typename KeyT, typename ValueT>
	class LRUCache {
	public:

		//OCF
		LRUCache(size_t aMaxSize) : maxsize{aMaxSize} {};

		void    put(const KeyT& aKey, const ValueT& aValue);
		ValueT& get(const KeyT& aKey);
		bool    contains(const KeyT& aKey) const;
		size_t  size() const; //current size

	protected:
		size_t maxsize; //prevent cache from growing past this size...

		//data members here...
		std::map<KeyT, bool> usedMap;
		std::map<KeyT, ValueT> cacheMap;
	};

	template<typename KeyT, typename ValueT>
	inline void LRUCache<KeyT, ValueT>::put(const KeyT& aKey, const ValueT& aValue)
	{
		if (cacheMap.size() == maxsize) { //If cache is full, replace
			auto theIter = usedMap.begin();
			while (theIter != usedMap.end()) {
				if (theIter->second == false) {
					cacheMap.erase(theIter->first);
					theIter = usedMap.erase(theIter);
				}
				else {
					++theIter;
					theIter->second = false;
				}
			}

			if (theIter == usedMap.end()) {
				cacheMap.erase(cacheMap.begin());
				usedMap.erase(usedMap.begin());
			}
		}
		// add new things to map
		usedMap[aKey] = false;
		cacheMap[aKey] = aValue;
	}

	template<typename KeyT, typename ValueT>
	inline ValueT& LRUCache<KeyT, ValueT>::get(const KeyT& aKey)
	{
		usedMap[aKey] = true;
		return cacheMap.at(aKey);
	}

	template<typename KeyT, typename ValueT>
	inline bool LRUCache<KeyT, ValueT>::contains(const KeyT& aKey) const
	{
		return cacheMap.count(aKey);
	}

	template<typename KeyT, typename ValueT>
	inline size_t LRUCache<KeyT, ValueT>::size() const
	{
		return cacheMap.size();
	}

}





#endif // !LRUCache_hpp


