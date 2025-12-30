#ifndef ENCODING_H
#define ENCODING_H

#include <cassert>
#include <type_traits>

template <typename U, typename T>
inline void pack_into_bits(U& u, unsigned int& start, const T& data) {
  const unsigned int w = T::width;
  u(start + w - 1, start) = data(w - 1, 0);
  start += w;
}

template <typename U, typename T>
inline void unpack_from_bits(const U& u, unsigned int& start, T& data) {
  const unsigned int w = T::width;
  data(w - 1, 0) = u(start + w - 1, start);
  start += w;
}

#endif