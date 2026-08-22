// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_CONTAINER_MATH_ARRAY_H_
#define CORE_CONTAINER_MATH_ARRAY_H_

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <numeric>
#include <ostream>
#include <stdexcept>
#include <utility>

#include "core/real_t.h"
#include "core/util/log.h"
#include "core/util/root.h"

namespace bdm {

/// Array with a fixed number of elements. It implements the same behaviour
/// of the standard `std::array<T, N>` container. However, it provides also
/// several custom mathematical operations (e.g. Sum(), Norm() etc.).
template <class T, std::size_t N>
class MathArray {  // NOLINT
 public:
  /// Default constructor
  MathArray() = default;

  /// Constructor which accepts an std::initializer_list to set
  /// the array's content.
  /// \param l an initializer list
  MathArray(std::initializer_list<T> l) {
    if (l.size() > N) {
      throw std::length_error("MathArray initializer exceeds its capacity");
    }
    std::copy(l.begin(), l.end(), data_.begin());
  }

  /// Return a pointer to the underlying data.
  /// \return cont T pointer to the first entry of the array.
  inline T* data() { return data_.data(); }  // NOLINT

  inline const T* data() const { return data_.data(); }  // NOLINT

  /// Return the size of the array.
  /// \return integer denoting the array's size.
  inline size_t size() const { return data_.size(); }  // NOLINT

  /// Check if the array is empty.
  /// \return true if size() == 0, false otherwise.
  inline bool empty() const { return data_.empty(); }  // NOLINT

  /// Overloaded array subscript operator. It does not perform
  /// any boundary checks.
  /// \param idx element's index.
  /// \return the requested element.
  T& operator[](size_t idx) { return data_[idx]; }

  /// Const overloaded array subscript operator.
  /// \param idx element's index.
  /// \return the requested element.
  const T& operator[](size_t idx) const { return data_[idx]; }

  /// Returns the element at the given position. It will throw
  /// an std::out_of_range exception if the given index is out
  /// of the array's boundaries.
  /// \param idx the index of the element.
  /// \return the requested element.
  T& at(size_t idx) { return data_.at(idx); }  // NOLINT

  const T& at(size_t idx) const { return data_.at(idx); }  // NOLINT

  const T* begin() const { return data_.begin(); }  // NOLINT

  const T* end() const { return data_.end(); }  // NOLINT

  T* begin() { return data_.begin(); }  // NOLINT

  T* end() { return data_.end(); }  // NOLINT

  /// Returns the element at the beginning of the array.
  /// \return first element.
  T& front() { return data_.front(); }  // NOLINT

  const T& front() const { return data_.front(); }  // NOLINT

  /// Return the element at the end of the array.
  /// \return last element.
  T& back() { return data_.back(); }  // NOLINT

  const T& back() const { return data_.back(); }  // NOLINT

  MathArray& operator=(const MathArray& other) = default;

  /// Equality operator.
  /// \param other a MathArray instance.
  /// \return true if they have the same content, false otherwise.
  bool operator==(const MathArray& other) const { return data_ == other.data_; }

  bool operator!=(const MathArray& other) const { return !operator==(other); }

  MathArray& operator++() {
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      ++data_[i];
    }
    return *this;
  }

  MathArray operator++(int) {
    MathArray tmp(*this);
    operator++();
    return tmp;
  }

  MathArray& operator--() {
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      --data_[i];
    }
    return *this;
  }

  MathArray operator--(int) {
    MathArray tmp(*this);
    operator--();
    return tmp;
  }

  MathArray& operator+=(const MathArray& rhs) {
    assert(N == rhs.size());
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] += rhs[i];
    }
    return *this;
  }

  MathArray operator+(const MathArray& rhs) {
    assert(size() == rhs.size());
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] + rhs[i];
    }
    return tmp;
  }

  const MathArray operator+(const MathArray& rhs) const {
    assert(size() == rhs.size());
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] + rhs[i];
    }
    return tmp;
  }

  MathArray& operator+=(const T& rhs) {
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] += rhs;
    }
    return *this;
  }

  MathArray operator+(const T& rhs) {
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] + rhs;
    }
    return tmp;
  }

  MathArray& operator-=(const MathArray& rhs) {
    assert(size() == rhs.size());
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] -= rhs[i];
    }
    return *this;
  }

  MathArray operator-(const MathArray& rhs) {
    assert(size() == rhs.size());
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] - rhs[i];
    }
    return tmp;
  }

  const MathArray operator-(const MathArray& rhs) const {
    assert(size() == rhs.size());
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] - rhs[i];
    }
    return tmp;
  }

  MathArray& operator-=(const T& rhs) {
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] -= rhs;
    }
    return *this;
  }

  MathArray operator-(const T& rhs) {
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] - rhs;
    }
    return tmp;
  }

  T& operator*=(const MathArray& rhs) = delete;

  T operator*(const MathArray& rhs) {
    assert(size() == rhs.size());
    T result = 0;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      result += data_[i] * rhs[i];
    }
    return result;
  }

  const T operator*(const MathArray& rhs) const {
    assert(size() == rhs.size());
    T result = 0;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      result += data_[i] * rhs[i];
    }
    return result;
  }

  MathArray& operator*=(const T& k) {
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] *= k;
    }
    return *this;
  }

  MathArray operator*(const T& k) {
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] * k;
    }
    return tmp;
  }

  const MathArray operator*(const T& k) const {
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] * k;
    }
    return tmp;
  }

  MathArray& operator/=(const T& k) {
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] /= k;
    }
    return *this;
  }

  MathArray operator/(const T& k) {
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] / k;
    }
    return tmp;
  }

  /// Fill the MathArray with a constant value.
  /// \param k the constant value
  /// \return the array
  MathArray& fill(const T& k) {  // NOLINT
    data_.fill(k);
    return *this;
  }

  /// Return the sum of all the array's elements.
  /// \return sum of the array's content.
  T Sum() const { return std::accumulate(begin(), end(), T{}); }

  /// Checks if vector is a zero vector, e.g. if all entries are zero.
  bool IsZero() const {
    for (size_t i = 0; i < N; i++) {
      if (data_[i] != 0) {
        return false;
      }
    }
    return true;
  }

  /// Compute the norm of the array's content.
  /// \return array's norm.
  T Norm() const {
    T result = 0;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      result += data_[i] * data_[i];
    }
    result = std::sqrt(result);

    return result;
  }

  /// Normalize the array in-place.
  void Normalize() {
    T norm = Norm();
    Normalize(norm);
  }

  /// Normalize the array in-place.\n
  /// If the calling code has already calculated the norm,
  /// this function signature ensures that the norm calculation
  /// is not duplicated.
  void Normalize(T norm) {
    if (norm == 0) {
      Log::Fatal("MathArray::Normalize",
                 "You tried to normalize a zero vector. "
                 "This cannot be done. Exiting.");
    }
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] /= norm;
    }
  }

  /// Get a nomalized copy of the MathArray.
  MathArray GetNormalizedArray() const {
    MathArray normalized_array(*this);
    normalized_array.Normalize();
    return normalized_array;
  }

  /// Compute the entry wise product given another array
  /// of the same size.
  /// \param rhs the other array
  /// \return a new array with the result
  MathArray EntryWiseProduct(const MathArray& rhs) const {
    assert(rhs.size() == N);
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; ++i) {
      tmp[i] = data_[i] * rhs[i];
    }
    return tmp;
  }

 private:
  std::array<T, N> data_{};
};

template <class T, std::size_t N>
std::ostream& operator<<(std::ostream& o, const MathArray<T, N>& arr) {
  for (size_t i = 0; i < N; i++) {
    o << arr[i];
    if (i != N - 1) {
      o << ", ";
    }
  }
  return o;
}

// Note: 1) We pass by value to allow for copy-elision and move semantics
//          optimization.
//       2) We do return MathArray<T, N> because a const MathArray<T, N>
//          prevents move semantics in C++11.
//       see https://tinyurl.com/left-multiply
/// Template function to multiply array with scalar from the left.
template <class T, std::size_t N>
MathArray<T, N> operator*(T const& scalar, MathArray<T, N> array) {
  return array *= scalar;
}

/// Aliases for a size 3 MathArray
using Real3 = MathArray<real_t, 3>;
using Float3 = MathArray<float, 3>;
using Double3 = MathArray<double, 3>;

/// Aliases for a size 4 MathArray
using Real4 = MathArray<real_t, 4>;
using Float4 = MathArray<float, 4>;
using Double4 = MathArray<double, 4>;

}  // namespace bdm

#endif  // CORE_CONTAINER_MATH_ARRAY_H_
