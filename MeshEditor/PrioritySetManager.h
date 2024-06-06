#pragma once
#include "MeshDefs.h"
#include "DualDefs.h"
#include "FuncDefs.h"

#include <set>
#include <map>
#include <deque>

template <typename NodeType, typename NodeTypeComp, typename Identifier>
class PrioritySetManager {
	typedef typename std::multiset<NodeType*, NodeTypeComp> priority_set;
	typedef typename std::multiset<NodeType*, NodeTypeComp>::iterator priority_set_iterator;
public:
	PrioritySetManager (){}
	~PrioritySetManager(){
		foreach (NodeType *node, core_priority_queue)
			delete node;
	}
public:
	bool empty () {
		return core_priority_queue.empty ();
	}
	priority_set_iterator begin (){
		return core_priority_queue.begin ();
	}
	priority_set_iterator end (){
		return core_priority_queue.end ();
	}
	priority_set_iterator insert (NodeType *new_node, Identifier &identifier){
		auto locate = core_priority_queue.insert (new_node);
		identifier_set_mapping.insert (std::make_pair (identifier, locate));
		return locate;
	}
	bool exists (Identifier &identifier){
		auto locate = identifier_set_mapping.find (identifier);
		return locate != identifier_set_mapping.end ();
	}
	priority_set_iterator find (Identifier &identifier){
		auto locate = identifier_set_mapping.find (identifier);
		if (locate == identifier_set_mapping.end ())
			return core_priority_queue.end ();
		else
			return locate->second;
	}
	void update (NodeType *node, Identifier &identifier){
		auto locate = identifier_set_mapping.find (identifier);
		assert (locate != identifier_set_mapping.end ());
		NodeType *old_node = *(locate->second);
		core_priority_queue.erase (locate->second);
		auto it = core_priority_queue.insert (node);
		locate->second = it;
		//别忘了在内存中析构它
		delete old_node;
	}
	NodeType * top (){
		return *(core_priority_queue.begin ());
	}
	void pop (Identifier &identifier){
		identifier_set_mapping.erase (identifier);
		core_priority_queue.erase (core_priority_queue.begin ());
	}
	NodeType * retrieve (Identifier &identifier){
		auto locate = identifier_set_mapping.find (identifier);
		if (locate == identifier_set_mapping.end ())
			return NULL;
		NodeType *old_node = *(locate->second);
		return old_node;
	}
	void erase (Identifier &identifier, bool delete_after = false){
		auto locate = identifier_set_mapping.find (identifier);
		if (locate == identifier_set_mapping.end ())
			return;
		NodeType *old_node = *(locate->second);
		core_priority_queue.erase (locate->second);
		identifier_set_mapping.erase (locate);
		//别忘了在内存中析构它
		if (delete_after)
			delete old_node;
	}
	void erase (NodeType *node, Identifier &identifier, bool delete_after = false){
		auto locate = identifier_set_mapping.find (identifier);
		if (locate == identifier_set_mapping.end ())
			return;
		core_priority_queue.erase (locate->second);
		identifier_set_mapping.erase (locate);
		if (delete_after)
			delete node;
	}
private:
	std::hash_map<Identifier, priority_set_iterator> identifier_set_mapping;
	priority_set core_priority_queue;
};

template <typename NodeType, typename Identifier>
class NormalSetManager {
	typedef typename std::map<Identifier, NodeType*>::iterator normal_set_iterator;
public:
	NormalSetManager (){}
	~NormalSetManager (){
		foreach (auto &p, normal_set)
			delete p.second;
	}
public:
	bool empty () {
		return normal_set.empty ();
	}
	normal_set_iterator begin () {
		return normal_set.begin ();
	}
	normal_set_iterator end () {
		return normal_set.end ();
	}
	void insert (NodeType *new_node, Identifier &identifier){
		normal_set.insert (std::make_pair (identifier, new_node));
	}
	bool exists (Identifier &identifier){
		auto locate = normal_set.find (identifier);
		return locate != normal_set.end ();
	}
	NodeType * retrieve (Identifier &identifier){
		auto locate = normal_set.find (identifier);
		if (locate == normal_set.end ())
			return NULL;
		else
			return locate->second;
	}
	void erase (NodeType *node, Identifier &identifier, bool delete_after = false){
		auto locate = normal_set.find (identifier);
		if (locate == normal_set.end ())
			return;
		normal_set.erase (locate);
		if (delete_after)
			delete node;
	}
private:
	std::map<Identifier, NodeType*> normal_set;
};

template <typename T>
void entity_list_to_vector (ENTITY_LIST &entity_list, std::vector<T> &vec)
{
	T tmp;
	for (int i = 0; i != entity_list.count (); ++i)
	{
		tmp = static_cast<T> (entity_list[i]);
		vec.push_back (tmp);
	}
}

template <typename T>
void entity_list_to_set (ENTITY_LIST &entity_list, std::set<T> &s)
{
	T tmp;
	for (int i = 0; i != entity_list.count (); ++i)
	{
		tmp = static_cast<T> (entity_list[i]);
		s.insert (tmp);
	}
}

template <typename T>
void init_matrix (std::vector<std::vector<T> > &m, int row, int column, T val)
{
	m.resize (row);
	for (int i = 0; i != row; ++i)
		m[i].resize (column, val);
}

template <typename T>
void set_to_vector (const std::set<T> &s, std::vector<T> &v)
{
	for (auto it = s.begin (); it != s.end (); ++it)
		v.push_back (*it);
}

template <typename T>
void vector_to_set (const std::vector<T> &v, std::set<T> &s)
{
	for (auto it = v.begin (); it != v.end (); ++it)
		s.insert (*it);
}

template <typename T>
void vector_to_unordered_set (const std::vector<T> &v, std::unordered_set<T> &s)
{
	for (auto it = v.begin (); it != v.end (); ++it)
		s.insert (*it);
}

template <typename T>
void unordered_set_to_vector (std::unordered_set<T> &s, std::vector<T> &v)
{
	for (auto it = s.begin (); it != s.end (); ++it)
		v.push_back (*it);
}

template <typename Container, typename T>
bool contains (Container &container, T &val)
{
	if (std::find (container.begin (), container.end (), val) != container.end ())
		return true;
	else
		return false;
}

template <typename Container>
bool contains (Container &container1, Container &container2)
{
	for(auto iter = container2.begin(); iter != container2.end(); iter++){
		if (std::find (container1.begin (), container1.end (), *iter) == container1.end ())
			return false;
	}
	return true;
}

template <typename T>
bool intersects (const std::set<T> &set1, const std::set<T> &set2)
{
	std::vector<T> comm;
	std::set_intersection (set1.begin (), set1.end (), set2.begin (), set2.end (),
		std::back_inserter (comm));
	return !comm.empty ();
}

template <typename T>
bool intersects (const std::unordered_set<T> &set1, const std::unordered_set<T> &set2)
{
	foreach (auto &x, set1){
		foreach (auto &y, set2){
			if (x == y) return true;
		}
	}
	return false;
}

template <typename T>
bool equals (const std::unordered_set<T> &set1, const std::unordered_set<T> &set2)
{
	if (set1.size () != set2.size ()) return false;

	foreach (auto &x, set1){
		if (set2.find (x) == set2.end ())
			return false;
	}
	return true;
}

template <typename T>
bool intersects (const std::vector<T> &vec1, const std::vector<T> &vec2)
{
	std::vector<T> comm;
	std::set_intersection (vec1.begin (), vec1.end (), vec2.begin (), vec2.end (),
		std::back_inserter (comm));
	return !comm.empty ();
}

template <typename T>
std::vector<T> entitylist_to_stdvector (ENTITY_LIST entity_list)
{
	std::vector<T> result;
	for (int i = 0; i != entity_list.count (); ++i)
		result.push_back ((T)(entity_list[i]));
	return result;
}

template <typename T>
T pop_begin_element (std::set<T> &myset)
{
	T res = *(myset.begin ());
	myset.erase (myset.begin ());
	return res;
}

template <typename T>
T pop_begin_element (std::vector<T> &myset)
{
	T res = *(myset.begin ());
	myset.erase (myset.begin ());
	return res;
}

template <typename T>
T pop_begin_element (std::unordered_set<T> &myset)
{
	T res = *(myset.begin ());
	myset.erase (myset.begin ());
	return res;
}

template <typename T>
T pop_begin_element (QSet<T> &myset)
{
	T res = *(myset.begin ());
	myset.erase (myset.begin ());
	return res;
}

template<typename KeyT, typename ValT>
ValT find_key_from_value (std::hash_map<KeyT, ValT> &mapping, ValT &val)
{
	ValT result;
	foreach (auto &p, mapping){
		if (p.second == val){
			result = p.first;
			break;
		}
	}
	return result;
}

template<typename KeyT, typename ValT>
ValT find_key_from_value (std::map<KeyT, ValT> &mapping, ValT &val)
{
	ValT result;
	foreach (auto &p, mapping){
		if (p.second == val){
			result = p.first;
			break;
		}
	}
	return result;
}
template<typename T>
std::unordered_set<T> intersection (const std::unordered_set<T> &set1, const std::unordered_set<T> &set2)
{
	std::unordered_set<T> res;
	foreach (const auto & x, set1) {
		if (contains(set2, x)) res.insert(x);
	}
	return res;
}



template<typename T>
std::unordered_set<T> difference (const std::unordered_set<T> &set1, const std::unordered_set<T> &set2)
{
	std::unordered_set<T> res;
	foreach (const auto &x, set1) {
		if (!contains(set2, x)) res.insert(x);
	}
	return res;
}

template <typename T>
std::unordered_set<T> get_union(const std::unordered_set<T> &set1, const std::unordered_set<T> &set2)
{
	std::unordered_set<T> ans;
	foreach (const auto &x, set1) {
		ans.insert(x);
	}
	foreach (const auto &x, set2) {
		ans.insert(x);
	}
	return ans;
}
