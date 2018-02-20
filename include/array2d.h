// Customise the behaviour of array2d by defining these before including it:
// #define BSP_ARRAY2D_ALLOCATION(array, bytes) to receive allocations

#ifndef BSP_ARRAY2D_H
#define BSP_ARRAY2D_H

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

namespace bsp {

template<typename T> class array2d {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = int;

public:
    array2d() = default;
    
    array2d(size_type width, size_type height, const T& value = T()) : width_(width), height_(height), data_(width_ * height_, value), null_value_() {
        assert(width >= 0 && height >= 0); 
    }

    array2d(const array2d& other):width_(other.width_), height_(other.height_), data_(other.data_), null_value_(other.null_value_){}

    array2d& operator=(const array2d& other){
        data_ = other.data_;
        width_ = other.width_;
        height_ = other.height_;
        null_value_ = other.null_value_;
        return *this;
    }

    array2d(array2d&& other):width_(other.width_), height_(other.height_), data_(std::move(other.data_)), null_value_(other.null_value_){
        other.width_ = 0;
        other.height_ = 0;
    }

    array2d& operator=(array2d&& other){
        data_ = std::move(other.data_);
        width_ = other.width_;
        height_ = other.height_;
        null_value_ = other.null_value_;
        other.width_ = 0;
        other.height_ = 0;
        return *this;
    }

    inline size_type width() const { return width_; }

    inline size_type height() const { return height_; }

    void resize(size_type width, size_type height, const T& value = T()){
        assert(width >= 0 && height >= 0);
        
    #ifdef BSP_ARRAY2D_ALLOCATION
        int bytes = (sizeof(T) * width * height);
        BSP_ARRAY2D_ALLOCATION(*this, bytes);
    #endif

        width_ = width;
        height_ = height;
        data_.resize(width_ * height_);
        fill(value);
        null_value_ = T();
    }

    inline void fill(const T& t) { 
        std::fill(data_.begin(), data_.end(), t); 
    }

    inline reference operator()(size_type i, size_type j) { 
        return valid_index(i, j) ? data_[index(i,j)] : null_value_; 
    }
    
    inline const_reference operator()(size_type i, size_type j) const { 
        return valid_index(i, j) ? data_[index(i,j)] : null_value_; 
    }
    
    inline bool valid_index(size_type i, size_type j) const {
        return i >= 0 && i < width_ && j >= 0 && j < height_; 
    }
    
protected:
    size_type width_  = 0;
    size_type height_ = 0;
    std::vector<T> data_;
    T null_value_;

protected:
    inline size_type index(size_type i, size_type j) const { return i + j*width_; }

	template<typename T_>
	friend std::ostream& operator<<(std::ostream&, const array2d<T_>&);
};

template<typename T>
inline std::ostream& operator<<(std::ostream& out, const array2d<T>& array) {
    using size_type = typename array2d<T>::size_type;

	out << "array2d {";
    for (size_type row = 0; row < array.height_; row++){
        out << "{";
        for (size_type col = 0; col < array.width_; col++){
            out << array(col, row);
            if (col < array.width_ - 1) out << ", ";
        }
        out << "}";
        if (row < array.height_ - 1) out << ", ";
    }
    out << "}";
	return out;
}

}

#endif