#ifndef BSP_RING_BUFFER_H
#define BSP_RING_BUFFER_H

#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <initializer_list>
#include <stdexcept>
#include <ostream>
#include <utility>

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
}

template<typename T, int Capacity> class ring_buffer {
    static_assert(Capacity > 0, "Capacity <= 0!"); 

public:
    using value_type = T;
    using reference = T&;
	using const_reference = const T&;
	using size_type = int;

    class iterator : public std::iterator<std::bidirectional_iterator_tag, T> {
    public:
        using reference = typename ring_buffer::reference;
        using const_reference = typename ring_buffer::const_reference;
        using size_type = typename ring_buffer::size_type;
    
	public:
        iterator(ring_buffer& ring):ring_(ring), i_(ring.count()){}
		iterator(ring_buffer& ring, size_type offset):ring_(ring),offset_(offset){}		
        iterator& operator++(){ i_++; return *this; }
        iterator operator++(int){ iterator tmp(*this); ++(*this); return tmp; }
        iterator& operator--(){ i_ = (i_ + ring_.max_size() - 1) % ring_.max_size(); return *this; }
        iterator operator--(int){ iterator tmp(*this); --(*this); return tmp; }
		bool operator==(const iterator& rhs) const { return &ring_ == &rhs.ring_ && i_ == rhs.i_; }
        bool operator!=(const iterator& rhs) const { return !(*this == rhs); }
		reference operator*() { return ring_[(i_ + offset_) % ring_.max_size()]; }
		const_reference operator*() const { return ring_[(i_ + offset_) % ring_.max_size()]; }
		
    protected:
        ring_buffer& ring_;
        size_type offset_ = 0;
		size_type i_ = 0;     

        friend class const_iterator;   
	};

    class const_iterator : public std::iterator<std::bidirectional_iterator_tag, T> {
    public:
        using reference = typename ring_buffer::const_reference;
        using size_type = typename ring_buffer::size_type;
    
	public:
        const_iterator(const ring_buffer& ring):ring_(ring), i_(ring.count()){}
		const_iterator(const ring_buffer& ring, size_type offset):ring_(ring),offset_(offset){}		
        const_iterator(const iterator& it):ring_(it.ring_), i_(it.i_), offset_(it.offset_){}        
        const_iterator& operator++(){ i_++; return *this; }
        const_iterator operator++(int){ const_iterator tmp(*this); ++(*this); return tmp; }
        const_iterator& operator--(){ i_ = (i_ + ring_.max_size() - 1) % ring_.max_size(); return *this; }
        const_iterator operator--(int){ const_iterator tmp(*this); --(*this); return tmp; }
		bool operator==(const const_iterator& rhs) const { return &ring_ == &rhs.ring_ && i_ == rhs.i_; }
        bool operator!=(const const_iterator& rhs) const { return !(*this == rhs); }
        reference operator*() const { return ring_[(i_ + offset_) % ring_.max_size()]; }
		
    protected:
        const ring_buffer& ring_;
        size_type offset_ = 0;
		size_type i_ = 0;
	};

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:

    ring_buffer() = default;

    template <typename Container>
    ring_buffer(const Container& els):ring_buffer(els.begin(), els.end()){}
    ring_buffer(std::initializer_list<T> list):ring_buffer(list.begin(), list.end()){}

    // [[deprecated("use start() instead")]]
	size_type index() const { return start(); }

    size_type start() const { return start_; }

	size_type count() const { return count_; }

	static constexpr inline size_type max_size() { return Capacity; }

	void clear() {
		count_ = 0;
		start_ = 0;
	}

	bool empty() const { return count_ == 0; }

    bool valid_index(size_type index) const {
        if (index >= start_) return (index - start_) < count_;
        else return (max_size() - (start_ - index)) < count_;
    }

    // [[deprecated("use push_back() instead")]]
	void add(const T& element) {
		push_back(element);
	}

    // Add an element to the end of the ring buffer
    template <typename U>
    void push_back(U&& value){
        size_type next_index = (start_ + count_) % max_size();
		data_[next_index] = std::forward<U>(value);
		if (count_ >= max_size())
			start_ = (start_ + 1) % max_size();
		else
			count_++;
    }

    template<class... Args> 
    void emplace_back(Args&&... args) {
        size_type next_index = (start_ + count_) % max_size();
		data_[next_index] = T(std::forward<Args>(args)...);
		if (count_ >= max_size())
			start_ = (start_ + 1) % max_size();
		else
			count_++;
    }

    void pop_front(){
        assert(count_ > 0);
        start_ = (start_ + 1) % max_size();
        count_--;
    }

    reference front(){ return data_[start_]; }
    const_reference front() const { return data_[start_]; }

    reference back(){ return count_ == 0 ? front() : data_[(start_ + count_ - 1) % max_size()]; }
    const_reference back() const { return count_ == 0 ? front() : data_[(start_ + count_ - 1) % max_size()]; }

    // Directly indexes into underlying array
	reference operator[](size_type index) { return data_[index]; }

	const_reference operator[](size_type index) const { return data_[index]; }

    reference at(size_type index) { return const_cast<reference>(static_cast<const ring_buffer*>(this)->at(index)); }
    const_reference at(size_type index) const {
		if (index >= 0 && index < count_) {
			return data_[index];
		}
		else {
            throw std::runtime_error("ring_buffer: accessing invalid element");
			return data_[0];
		}
	}

    iterator begin() { return iterator(*this, 0); }

    iterator end() { return iterator(*this); }

    const_iterator begin() const { return const_iterator(*this, 0); }

    const_iterator end() const { return const_iterator(*this); }

    const_iterator cbegin() const { return const_iterator(*this, 0); }

    const_iterator cend() const { return const_iterator(*this); }

    reverse_iterator rbegin() { return std::reverse_iterator<iterator>(end()); }

    reverse_iterator rend() { return std::reverse_iterator<iterator>(iterator(*this, 0)); }

    const_reverse_iterator rbegin() const { return std::reverse_iterator<const_iterator>(end()); }

    const_reverse_iterator rend() const { return std::reverse_iterator<const_iterator>(const_iterator(*this, 0)); }

protected:
	std::array<T, Capacity> data_;
	size_type count_ = 0;
	size_type start_ = 0;

protected:
    template <typename Iter, typename = typename std::enable_if<detail::is_iterator<Iter>::value>::type>
    ring_buffer(Iter begin, Iter end){
        for (auto it = begin; it != end; ++it) push_back(*it);
    }

	template<typename T_, int Capacity_>
	friend std::ostream& operator<<(std::ostream&, const ring_buffer<T_,Capacity_>&);
};

template<typename T_, int Capacity_>
inline std::ostream& operator<<(std::ostream& out,
								const ring_buffer<T_,Capacity_>& ring) {
	out << "ring_buffer<" << Capacity_ << "> {";
	if (ring.empty())
		out << "}";
	else {
        using size_type = typename ring_buffer<T_,Capacity_>::size_type;
        for (size_type i = 0; i < ring.max_size(); ++i){
            if (ring.valid_index(i)) out << ring[i];
            else out << "_";
            if (i != ring.max_size() - 1) out << ", ";
        }
		out << "}";
	}
	return out;
}

} // namespace

#endif