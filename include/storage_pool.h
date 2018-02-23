// #define BSP_STORAGE_POOL_LOG_ERROR(message) to log errors
// #define BSP_STORAGE_POOL_ALLOCATION(message, bytes) to report allocations
// For either of these you also need
// #define BSP_TYPE_NAME(type) to return a string representing the typename

#ifndef BSP_STORAGE_POOL_H
#define BSP_STORAGE_POOL_H

#include <cassert>
#include <limits>
#include <type_traits>
#include <vector>

#if defined(BSP_STORAGE_POOL_LOG_ERROR) || defined(BSP_STORAGE_POOL_ALLOCATION)
#include <sstream>
#endif

namespace bsp {

// Manages a list of uninitialised storages for T
// Typically use is to access storage() directly
// Note: Direct access through operator[] is O(N = storage.storage_count())
template<typename T> class storage_pool {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = int;

	struct storage_type {
        using aligned_storage_type = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

		size_type bytes  = 0;
		size_type count  = 0;
		size_type offset = 0;
		T* data          = nullptr;

        storage_type() = default;
        storage_type(size_type bytes, size_type count, size_type offset, T* data):bytes(bytes), count(count), offset(offset), data(data){}
        ~storage_type(){ if (data != nullptr) delete[]reinterpret_cast<aligned_storage_type*>(data); }
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

	explicit storage_pool(size_type count):size_(0){
        assert(count > 0);
        const int initial_capacity = 100;
		storages_.reserve(initial_capacity);
        append_storage(count);
	}

    storage_pool(const storage_pool&) = delete;
	storage_pool& operator=(const storage_pool&) = delete;
    storage_pool(storage_pool&&) = delete;
	storage_pool& operator=(storage_pool&&) = delete;

	bool append_storage(size_type size) {
		assert(size > 0);

		auto new_bytes = size_of_value() * size;
		auto current_bytes = size_of_value() * size_;

		if(current_bytes > std::numeric_limits<size_type>::max() - new_bytes){
			allocation_error(std::numeric_limits<size_type>::max());
			return false;
		}

		auto total_bytes = current_bytes + new_bytes;
		log_allocation(size_ + size, total_bytes);
		T* data = reinterpret_cast<T*>(new (std::nothrow) typename storage_type::aligned_storage_type[new_bytes]);
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
	
	inline size_type size() const { return size_; }

	inline size_type bytes() const { return size_ * size_of_value(); }

	inline size_type storage_count() const { return storages_.size(); }

	const storage_type& storage(size_type i) const { return storages_[i]; }

    inline reference operator[](size_type index) { return const_cast<reference>(static_cast<const storage_pool*>(this)->operator[](index)); }

	const_reference operator[](size_type index) const {
		for (const auto& d : storages_) {
			if (index >= d.offset && index < (d.offset + d.count)) {
				return d.data[index - d.offset];
			}
		}
		error("storage_pool: invalid index");
		return *storages_[0].data;
	}

protected:
	size_type size_ = 0;
	std::vector<storage_type> storages_;

protected:
	inline const size_type size_of_value() { return static_cast<size_type>(sizeof(T)); };

	void log_allocation(size_type count, size_type bytes){
#ifdef BSP_STORAGE_POOL_ALLOCATION
        std::ostringstream oss;
		oss << "storage_pool<" << BSP_TYPE_NAME(T) << "> allocating " << count << " elements";
        BSP_STORAGE_POOL_ALLOCATION(oss.str(), bytes);        
#endif
	}

	void allocation_error(size_type bytes){
#ifdef BSP_STORAGE_POOL_LOG_ERROR
        std::ostringstream oss;
		oss << "storage_pool<" << BSP_TYPE_NAME(T) << "> couldn't allocate new memory. "
			<< "Attempted total " << (bytes / 1024) << "kB";
        BSP_STORAGE_POOL_LOG_ERROR(oss.str());
#endif
	}

	void error(const char* message) const {
#ifdef BSP_STORAGE_POOL_LOG_ERROR
		BSP_STORAGE_POOL_LOG_ERROR(message);
#endif
	}
};

} // namespace bsp

#endif