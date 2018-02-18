// Customise the behaviour of fixed_map by defining these before including it:
// #define BSP_FIXED_MAP_THROWS to get runtime_error on overflow
// #define BSP_FIXED_MAP_LOG_ERROR(message) to log errors

#ifndef BSP_FIXED_MAP_H
#define BSP_FIXED_MAP_H

#include <array>
#include <functional>
#include <iostream>
#include <iterator>
#include <ostream>

namespace bsp {

namespace detail {
template<class, class Enable = void> struct is_iterator : std::false_type {};
template<typename T_>
struct is_iterator<
	T_, typename std::enable_if<
			std::is_base_of<std::input_iterator_tag,
							typename std::iterator_traits<T_>::iterator_category>::value ||
			std::is_same<std::output_iterator_tag,
						 typename std::iterator_traits<T_>::iterator_category>::value>::type>
	: std::true_type {};
} // namespace detail

// A simple fixed-size inlined hash map
// Note! Order is non-deterministic across platforms
template<typename Key, typename T, int Capacity, class Hash = std::hash<Key>> class fixed_map {
	static_assert(Capacity > 0, "Capacity <= 0!");

public:
	using key_type = Key;
	using mapped_type = T;
	struct value_type {
		key_type key;
		mapped_type value;
		bool valid = false;
	};
	using reference = T&;
	using const_reference = const T&;
	using iterator = value_type*;
	using const_iterator = const value_type*;
	using size_type = std::size_t;

public:
	fixed_map(const T& invalidValue = T()) : size_(0), invalid_value_(invalidValue) { clear(); }

	template<class Container> fixed_map(const Container& els) : fixed_map(els.begin(), els.end()) {}

	fixed_map(std::initializer_list<std::pair<Key, T>> list)
		: fixed_map(list.begin(), list.end()) {}

	inline void clear() {
		size_ = 0;
		std::fill(data_.begin(), data_.end(), value_type());
	}

	inline bool empty() const { return size_ == 0; }

	inline size_type size() const { return size_; }

	static constexpr inline size_type max_size() { return Capacity; }

	bool has(const key_type& key) const {
		size_type index = hash(key);
		size_type oindex = index;
		while (true) {
			if (data_[index].valid && data_[index].key == key) {
				return true;
			}
			index = (index + 1) % max_size();
			if (index == oindex) {
				break;
			}
		}
		return false;
	}

	inline const_reference find(const key_type& key) const {
		size_type index = hash(key);
		size_type oindex = index;
		while (true) {
			if (data_[index].valid && data_[index].key == key) {
				return data_[index].value;
			}
			index = (index + 1) % max_size();
			if (index == oindex) {
				break;
			}
		}
		return invalid_value_;
	}

	inline reference find(const key_type& key) {
		return const_cast<reference>(static_cast<const fixed_map*>(this)->find(key));
	}

	reference operator[](const key_type& key) { return find(key); }

	const_reference operator[](const key_type& key) const { return find(key); }

	template<typename Key_> iterator insert(const Key_& key, const T& value) {
		if (size_ >= max_size()) {
			error("fixed_map: trying to insert too many elements");
			return begin();
		}
		size_type index = hash(key);
		size_type oindex = index;
		while (data_[index].valid) {
			index = (index + 1) % max_size();
			if (index == oindex) {
				error("fixed_map: tried to insert too many elements");
				return begin();
			}
		}
		data_[index].key = key;
		data_[index].value = value;
		data_[index].valid = true;
		size_++;
		return &data_[index];
	}

	iterator begin() { return data_.begin(); }
	iterator end() { return begin() + max_size(); }

	const_iterator begin() const { return data_.begin(); }
	const_iterator end() const { return begin() + max_size(); }

protected:
	size_type size_ = 0;
	std::array<value_type, Capacity> data_;
	T invalid_value_;

protected:
	template<typename Iter,
			 typename = typename std::enable_if<detail::is_iterator<Iter>::value>::type>
	fixed_map(Iter begin_, Iter end_) {
		auto size = std::distance(begin_, end_);
		if (size > max_size()) throw std::runtime_error("fixed_map: too many elements");
		for (; begin_ != end_; ++begin_) {
			insert(begin_->first, begin_->second);
		}
	}

	static size_type hash(const key_type& key) { return ((size_type) Hash{}(key)) % max_size(); }

	void error(const char* message) const {
#ifdef BSP_FIXED_MAP_LOG_ERROR
		BSP_FIXED_MAP_LOG_ERROR(message);
#endif
#ifdef BSP_FIXED_MAP_THROWS
		throw std::runtime_error(message);
#endif
	}

	template<typename Key_, typename T_, int Capacity_, class Hash_>
	friend std::ostream& operator<<(std::ostream&, const fixed_map<Key_, T_, Capacity_, Hash_>&);
};

template<typename Key_, typename T_, int Capacity_, class Hash_>
inline std::ostream& operator<<(std::ostream& out,
								const fixed_map<Key_, T_, Capacity_, Hash_>& map) {
	out << "fixed_map<" << Capacity_ << "> {";
	if (map.empty())
		out << "}";
	else {
		for (auto it = map.data_.begin(); it != map.data_.end(); ++it) {
			const auto& el = *it;
			if (el.valid)
				out << el.key << ": " << el.value;
			else
				out << "_";
			if (std::next(it) != map.data_.end())
				out << ", ";
		}
		out << "}";
	}
	return out;
}

} // namespace bsp

#endif