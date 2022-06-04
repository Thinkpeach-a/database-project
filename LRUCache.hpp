#ifndef LRUCache_hpp
#define LRUCache_hpp

#include <list>
#include <map>
#include <unordered_map>
//#include <iterator>

namespace ECE141 {
	template<typename KeyT, typename ValueT>
	class LRUCache {
	public:

		//OCF
		LRUCache(size_t aMaxSize) : maxsize{ aMaxSize } {};

		void    put(const KeyT& aKey, const ValueT& aValue);
		ValueT& get(const KeyT& aKey);
		bool    contains(const KeyT& aKey) const;
		size_t  size() const; //current size
		size_t  maxSize() { return maxsize; }

	protected:
		size_t maxsize; //prevent cache from growing past this size...

		//data members here...
		std::unordered_map<KeyT, bool> usedMap;
		std::unordered_map<KeyT, ValueT> cacheMap;
	};

	template<typename KeyT, typename ValueT>
	inline void LRUCache<KeyT, ValueT>::put(const KeyT& aKey, const ValueT& aValue)
	{
		//bool theMissFlag = false;
		if (maxsize != 0) {
			if (cacheMap.size() == maxsize) { //If cache is full, replace

				auto theIter = usedMap.begin();
				while (theIter != usedMap.end()) {
					if (theIter->second == false) {
						cacheMap.erase(theIter->first);
						theIter = usedMap.erase(theIter);
						//theMissFlag = true;
						break;
					}
					else {
						++theIter;
						theIter->second = false;
					}
				}
				if (theIter == usedMap.end()) { // if all blocks were recently used, remove first one
					cacheMap.erase(cacheMap.begin());
					usedMap.erase(usedMap.begin());
					//theMissFlag = true;
				}
			}
			// add new things to map
			usedMap[aKey] = false;
			cacheMap[aKey] = aValue;
			//return theMissFlag;
		}
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
		return cacheMap.count(aKey) != 0;
	}

	template<typename KeyT, typename ValueT>
	inline size_t LRUCache<KeyT, ValueT>::size() const
	{
		return cacheMap.size();
	}

}





#endif // !LRUCache_hpp


