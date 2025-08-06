#pragma once
#include <unordered_map>
#include <optional>
#include <mutex>
#include <thread>
#include <list>

template<typename K, typename V>
class ThreadSafe_LRU
{
	using Clock = std::chrono::steady_clock;
	using TimePoint = Clock::time_point;
	using Duration = std::chrono::milliseconds;

	struct Cache_Eviction
	{
		K key;
		V value;
		TimePoint expiry;
	};
	
	typedef typename std::list<Cache_Eviction>::iterator list_iterator;

	std::unordered_map<K, list_iterator> cache;
	std::list<Cache_Eviction> list;
	size_t capacity;
	Duration default_ttl;
	mutable std::mutex mtx;

	size_t hits;
	size_t misses;
	size_t total_requests;

public:

	struct Cache_Stats
	{
		size_t _hits;
		size_t _misses;
		size_t _total_requests;
	};


	ThreadSafe_LRU(const size_t& cap, Duration ttl) : capacity(cap), default_ttl(ttl)
	{		
		hits = 0;
		misses = 0;
		total_requests = 0;
	}

	// retrieve value from key
	std::optional<V> get(const K& key)
	{
		std::lock_guard<std::mutex> lock(mtx);
		++total_requests;
		auto it = cache.find(key);
		if (it == cache.end())
		{
			++misses;
			return std::nullopt;
		}

		auto node = it->second;
		if (node->expiry <= Clock::now())
		{
			list.erase(node);
			cache.erase(it);
			++misses;
			return std::nullopt;
		}

		list.splice(list.begin(), list, node);
		++hits;
		return node->value;
	}

	//remove key
	bool remove(const int& key)
	{
		std::lock_guard<std::mutex> lock(mtx);
		auto it = cache.find(key);
		if (it != cache.end())
		{
			list.erase(it->second);
			cache.erase(it);
			return true;
		}
		return false;
	}

	// return current cache size
	size_t size() const
	{
		std::lock_guard<std::mutex> lock(mtx);
		size_t count = 0;
		TimePoint time = Clock::now();

		for (auto& l : list)
		{
			if (l.expiry > time)
				++count;
		}

		return count;
	}

	// insert/Update cache
	void put(const K& key, V value, Duration ttl = Duration(0))
	{
		std::lock_guard<std::mutex> lock(mtx);
		cleanup_expired();

		auto it = cache.find(key);
		if (it != cache.end())
		{
			list.erase(it->second);
			cache.erase(it);
		}

		if (list.size() >= capacity)
		{
			auto node = list.back();
			list.pop_back();
			cache.erase(node.key);
		}

		TimePoint expiry_time = Clock::now() + (ttl.count() > 0 ? ttl : default_ttl);
		list.push_front({key, std::move(value), expiry_time});
		cache[key] = list.begin();
	}

	size_t cleanup_expired()
	{
		//std::lock_guard<std::mutex> lock(mtx);
		size_t cnt = 0;
		TimePoint time = Clock::now();

		for (auto it = list.begin(); it != list.end();)
		{
			if (it->expiry <= time)
			{
				cache.erase(it->key);
				it = list.erase(it);
				++cnt;
			}
			else
				++it;
		}
		return cnt;
	}

	Cache_Stats get_stats() const
	{
		std::lock_guard<std::mutex> lock(mtx);
		return Cache_Stats{ hits, misses, total_requests };
	}
};
