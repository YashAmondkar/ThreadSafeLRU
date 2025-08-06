// LRUCache.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "ThreadSafe_LRU.h"
using namespace std;

int main()
{
    cout << "\n Thread Safe LRU Cache \n";
	ThreadSafe_LRU<int,int> lru_cache(10, std::chrono::seconds(10));
	lru_cache.put(1, 111);
	lru_cache.put(2, 222);
	lru_cache.put(3, 333);
	lru_cache.put(4, 444);
	std::this_thread::sleep_for(std::chrono::seconds(2));
	lru_cache.put(5, 555);
	lru_cache.put(6, 666);
	
	cout << " \n LRU Cache Size : " << lru_cache.size();

	cout << " \n LRU Cache Get Element \n";

	auto node = lru_cache.get(1);
	if (node)
		cout << "\n Value Found : ";
	else
		cout << "\n Value Not Found";
	
	bool result = lru_cache.remove(6);
	if (result)
		cout << "\n Value Removed";
	else
		cout << "\n Value Not Present in Cache";

	cout << " \n LRU Cache Size : " << lru_cache.size();

	auto stats = lru_cache.get_stats();
	cout << "\n Hits : " << stats._hits;
	cout << "\n Misses : " << stats._misses;
	cout << "\n Total Requests : " << stats._total_requests;

	return 0;
}
