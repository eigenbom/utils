// #define BSP_OBJECT_POOL_LOG_ERROR(message) to log errors

#ifndef BSP_OBJECT_POOL_H
#define BSP_OBJECT_POOL_H

#include <array>
#include <algorithm>
#include <cstdint>
#include <iterator>

#ifdef BSP_OBJECT_POOL_LOG_ERROR
#include <sstream>
#endif

#include "storage_pool.h"

namespace bsp {

// TODO: Move ID into template type
using ID = uint32_t;

class object_pool_base {
public:
	virtual ~object_pool_base() = default;
	virtual void clear() = 0;
};

// A tightly-packed pool for objects
// The first ID is always 0 / InvalidID
// Heavily inspired by an article on the Bitsquid blog.
template<typename T> class object_pool : public object_pool_base {
public:
	using value_type = T;
	using reference = T&;
	using const_reference = const T&;
	using size_type = int;

	class iterator: public std::iterator<std::input_iterator_tag, T> {
	public:
		using value_type = T;
		using reference = T&;
		using const_reference = const T&;

	public:
		iterator(object_pool& array, unsigned int index);
		iterator& operator++();
		// TODO: iterator operator++(int&);
		bool operator==(const iterator& rhs) const;
		bool operator!=(const iterator& rhs) const;
		reference operator*();
		const_reference operator*() const;
	private:
		object_pool& object_pool_;
		storage_pool<T>& storage_pool_;
		unsigned int i_ = 0;  // element index
		unsigned int di_ = 0; // storage index
		typename storage_pool<T>::storage db_;
	};

	// TODO: const iterators

public:
	// PRE: size <= max_size()
	object_pool(size_type size = max_size())
		: capacity_{size}, 
		  objects_{size},
		  objects_grow_size_(size){
		assert(size <= max_size());
		clear();
	}
	~object_pool() final override = default;
	object_pool(const object_pool&) = delete;
	object_pool& operator=(const object_pool&) = delete;
	
	ID push_back(const T& value = T()) {
		index_type& in = next_index();
		T* newT = new (&objects_[in.index]) T(value);
		newT->id = in.id;
		return newT->id;
	}

	template <typename U>
	ID push_back(U&& value) {
		index_type& in = next_index();
		T* newT = new (&objects_[in.index]) T(std::forward<U>(value));
		newT->id = in.id;
		return newT->id;
	}

	int count(ID id) const {
		const index_type& in = index(id);
		return (in.id == id && in.index != USHRT_MAX) ? 1 : 0;
	}

	reference operator[](ID id) {
		return objects_[index(id).index];
	}

	const_reference operator[](ID id) const {
		return objects_[index(id).index];
	}

	reference front() { return objects_[0]; }

	const_reference front() const { return objects_[0]; }

	const storage_pool<T>& objects() const { return objects_; }

	size_type size() const { return num_objects_; }

	size_type capacity() const { return capacity_; }

	static constexpr size_type max_size() { return MaxSize; }

	void remove(ID id) {
		index_type& in = index(id);
		assert(in.id == id);

		const uint32_t id_increment = 0x10000;
		in.id = ID { static_cast<uint32_t>(id) + id_increment }; // increment id to avoid conflicts

		T& o = objects_[in.index];
		assert(o.id == id);
		destroy(o);

		// Move object at back into the hole left by deletion
		if (in.index != num_objects_ - 1) {
			new (&o) T(std::move(objects_[num_objects_ - 1]));
			indices_[mask_index(o.id)].index = in.index;
		}
		num_objects_--;

		// Update enqueue
		in.index = std::numeric_limits<uint16_t>::max();
		indices_[freelist_enque_].next = mask_index(id);
		freelist_enque_ = mask_index(id);
	}

	void clear() final override {
		for (size_type i = 0; i < num_objects_; i++) {
			destroy(objects_[i]);
		}
		num_objects_ = 0;
		for (size_type i = 0; i < max_size(); ++i) {
			auto& index = indices_[i];
			index.id = (ID) i;
			index.next = static_cast<uint16_t>(i + 1);
			index.index = std::numeric_limits<uint16_t>::max();
		}
		freelist_deque_ = 0;
		freelist_enque_ = static_cast<uint16_t>(capacity_ - 1);
	}

	iterator begin() { return iterator(*this, 1); }

	iterator end()   { return iterator(*this, size()); }
	
	// Debugging
	void check_internal_consistency() const {
		// want to be able to go from deque to enque and cover range
		int ni = freelist_deque_;
		if (freelist_deque_ == capacity_) {
			if (freelist_deque_ != freelist_enque_){
				error("object_pool: freelist_deque_ != freelist_enque_");
			}
		}
		else {
			int count = 1;
			while (true) {
				if (ni == freelist_enque_)
					break;
				ni = indices_[ni].next;
				count++;
			}
			if (count != capacity_ - num_objects_){
				error("object_pool: count != capacity_ - num_objects_");
			}
		}
	}

protected:
	static const size_type max_size_ = 0xffff;
	size_type objects_grow_size_ = 0;
	size_type capacity_ = 0;
	size_type num_objects_ = 0;
	uint16_t freelist_enque_ = 0;
	uint16_t freelist_deque_ = 0;

	struct index_type {
		ID id 		   = ID {0}; // TODO: document as invalid
		uint16_t index = 0;
		uint16_t next  = 0;
	};

	std::array<index_type, max_size_> indices_;
	storage_pool<T> objects_;

protected:

	inline uint32_t mask_index(ID id){
		const uint32_t index_mask = 0xffff;
		return static_cast<uint32_t>(id) & index_mask;
	}

	inline index_type& index(ID id) {
		return indices_[mask_index(id)];
	}

	inline const index_type& index(ID id) const {
		return indices_[mask_index(id)];
	}

	index_type& next_index() {
		if (num_objects_ >= max_size()) {
			error("object_pool: capacity exceeded");
		}

		if (num_objects_ >= capacity_) {
			// Resize
			size_type new_size = std::min(capacity_ + objects_grow_size_, max_size());
			if (new_size <= max_size()) {
				int num_new_objects = new_size - capacity_;
				const int numResizeAttempts = 10;
				for (int i = 0; i < numResizeAttempts; i++) {
					bool added = objects_.append_storage(num_new_objects);
					if (added) break;
					else num_new_objects /= 2; // try again with smaller size
				}
				capacity_ = objects_.size();
				indices_[freelist_enque_].next = static_cast<uint16_t>(num_objects_ + 1);
				freelist_enque_ = static_cast<uint16_t>(capacity_ - 1);
			}
			else {
				error("object_pool: capacity exceeded");
			}
		}

		index_type& in = indices_[freelist_deque_];
		freelist_deque_ = in.next;
		in.index = static_cast<uint16_t>(num_objects_++);
		return in;
	}

	void destroy(T& object){
		object.~T();
	}

	void error(const char* message) const {
#ifdef BSP_OBJECT_POOL_LOG_ERROR
		BSP_OBJECT_POOL_LOG_ERROR(message);
#endif
	}
};
  
template <typename T> const typename object_pool<T>::size_type object_pool<T>::max_size_;

template<typename C>
object_pool<C>::iterator::iterator(object_pool<C>& array, unsigned int ri) : object_pool_(array), storage_pool_(array.objects_) {
	i_ = 0;
	di_ = 0;
	for (; di_ < storage_pool_.numArrays(); di_++) {
		auto& dbz = storage_pool_.array(di_);
		if (ri >= dbz.offset && ri < (dbz.offset + dbz.count)) {
			i_ = ri - dbz.offset;
			db_ = dbz;
			break;
		}
	}
}

template<typename C> 
typename object_pool<C>::iterator& object_pool<C>::iterator::operator++() {
	++i_;
	// Search for next valid datablock
	while (true) {
		if (i_ == db_.count) {
			// Go to next datablock
			i_ = 0; 
			++di_;
			if (di_ >= storage_pool_.numArrays()) return *this;
			else db_ = storage_pool_.array(di_);
		}
		const auto& c = db_.data[i_];
		if (c.enabled() && !c.toBeRemoved()) break;
		++i_;
	}
	return *this;
}

template<typename C>
bool object_pool<C>::iterator::operator==(const typename object_pool<C>::iterator& rhs) const {
	return (i_ == rhs.i_ && di_ == rhs.di_) || (di_ == rhs.di_ && i_ >= rhs.i_) || (di_ > rhs.di_);
}

template<typename C>
bool object_pool<C>::iterator::operator!=(const typename object_pool<C>::iterator& rhs) const {
	return !(*this == rhs);
}

template<typename C> typename object_pool<C>::iterator::reference object_pool<C>::iterator::operator*() { return db_.data[i_]; }
template<typename C> typename object_pool<C>::iterator::const_reference object_pool<C>::iterator::operator*() const { return db_.data[i_]; }

}

#endif
