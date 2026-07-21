#pragma once

/*
 * Copyright (c) 2025, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <tuple>
#include <utility>

namespace sqlpp::detail {
template <template<typename> typename Predicate, typename T>
  requires(Predicate<T>::value)
auto tupelize(T t) -> std::tuple<T> {
  return std::make_tuple(std::move(t));
}

template <template<typename> typename Predicate, typename... Args>
  requires((Predicate<Args>::value && ...))
auto tupelize(std::tuple<Args...> t) -> std::tuple<Args...> {
  return t;
}

template <template<typename> typename Predicate, typename T>
  requires(not Predicate<T>::value)
auto tupelize(T) -> std::tuple<> {
  return {};
}

template <template<typename> typename Predicate, typename... Args>
  requires(not (Predicate<Args>::value && ...))
auto tupelize(std::tuple<Args...>) -> std::tuple<> {
  return {};
}

template <template<typename> typename Predicate, typename... Args>
struct flat_tuple {
  using type = decltype(std::tuple_cat(tupelize<Predicate>(std::declval<Args>())...));
};

template <template<typename> typename Predicate, typename... Args>
using flat_tuple_t = typename flat_tuple<Predicate, Args...>::type;
}  // namespace sqlpp::detail
