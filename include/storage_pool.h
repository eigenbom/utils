// #define BSP_STORAGE_POOL_LOG_ERROR(message) to log errors
// #define BSP_STORAGE_POOL_ALLOCATION(message, bytes) to report allocations
// For either of these you also need
// - template <typename T> struct type_name { static std::string get(){ return .... } }

#ifndef BSP_STORAGE_POOL_H
#define BSP_STORAGE_POOL_H

#include <cassert>
#include <limits>
#include <type_traits>
#include <list>
#include <string>
#include <typeinfo>

#if defined(BSP_STORAGE_POOL_LOG_ERROR) || defined(BSP_STORAGE_POOL_ALLOCATION)
#include <sstream>
#endif

namespace bsp {

template <typename T> void log_allocation(const T& owner, int count, int bytes){}

template <typename T> void log_error(const T& owner, const char* message){}

template <typename T> struct type_name {
	static std::string get(){
		return std::string(typeid(T).name());
	}
};

// Manages a list of uninitialised storages for T
// Typically use is to access storage() directly
// Note: Direct access through operator[] is O(N = storage.storage_count())
// TODO: Allow storage to shrink
template<typename T> class storage_pool {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = int;
	using aligned_storage_type = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

	struct storage_type {
		size_type bytes  = 0;
		size_type count  = 0;
		size_type offset = 0;
		T* data          = nullptr;

        storage_type() = default;
        storage_type(size_type bytes, size_type count, size_type offset, T* data):bytes(bytes), count(count), offset(offset), data(data){}
        storage_type(const storage_type&) = delete;
		storage_type& operator=(const storage_type&) = delete;
		storage_type(storage_type&& rhs):bytes(rhs.bytes), count(rhs.count), offset(rhs.offset), data(rhs.data){
			rhs.bytes = 0;
			rhs.count = 0;
			rhs.offset = 0;
			rhs.data = nullptr;
		}
	};

public:
    storage_pool() = default;

	explicit storage_pool(size_type count){
        assert(count > 0);
        allocate(count);
	}

    storage_pool(const storage_pool&) = delete;
	storage_pool& operator=(const storage_pool&) = delete;
    storage_pool(storage_pool&&) = delete;
	storage_pool& operator=(storage_pool&&) = delete;

	~storage_pool(){
		if (size() > 0) log_deallocation_internal(size(), bytes());
		for (auto& s: storages_) destroy(s);
	}

	bool allocate(size_type size) {
		assert(size > 0);

		auto new_bytes = size_of_value() * size;
		auto current_bytes = size_of_value() * size_;

		if(current_bytes > std::numeric_limits<size_type>::max() - new_bytes){
			allocation_error(std::numeric_limits<size_type>::max());
			return false;
		}

		auto total_bytes = current_bytes + new_bytes;
		log_allocation_internal(size_ + size, total_bytes);
		T* data = reinterpret_cast<T*>(new (std::nothrow) aligned_storage_type[new_bytes]);
		if (data == nullptr) {
			allocation_error(total_bytes);
			return false;
		}
		else {
            auto offset = size_;
			storages_.emplace_back(new_bytes, size, offset, data);
			size_ += size;
			return true;
		}
	}
	
	// Deallocates the most recently allocated storage
	void deallocate(){
		assert (storages_.size() > 0);
		auto& back_storage = storages_.back();
		auto num_bytes = back_storage.bytes;
		auto count = back_storage.count;
		size_ -= count;
		destroy(back_storage);
		storages_.pop_back();
		log_deallocation_internal(count, num_bytes);
	}

	inline size_type size() const { return size_; }

	inline size_type bytes() const { return size_ * size_of_value(); }

	inline size_type storage_count() const { return storages_.size(); }

	const storage_type& storage(size_type i) const { return *std::next(storages_.begin(), i); }

    inline reference operator[](size_type index) { return const_cast<reference>(static_cast<const storage_pool*>(this)->operator[](index)); }

	const_reference operator[](size_type index) const {
		for (const auto& d : storages_) {
			if (index >= d.offset && index < (d.offset + d.count)) {
				return d.data[index - d.offset];
			}
		}
		error("storage_pool: invalid index");
		return *(storages_.begin()->data);
	}

protected:
	size_type size_ = 0;
	std::list<storage_type> storages_;

protected:
	inline const size_type size_of_value() const { return static_cast<size_type>(sizeof(T)); };

	void destroy(storage_type& s){
		if (s.data) delete[]reinterpret_cast<aligned_storage_type*>(s.data);
		s.data = nullptr;		
	}

	void log_allocation_internal(size_type count, size_type bytes) const {
		log_allocation(*this, count, bytes);
	}

	void log_deallocation_internal(size_type count, size_type bytes) const {
		log_allocation(*this, count, -bytes);
	}

	void allocation_error(size_type bytes){
#ifdef BSP_STORAGE_POOL_LOG_ERROR
        std::ostringstream oss;
		oss << "couldn't allocate new memory (attempted " << (bytes / 1024) << "kB)";
        log_error(*this, oss.str().c_str());
#endif
	}

	void error(const char* message) const {
		log_error(*this, message);
	}
};

} // namespace bsp

#endif