#ifndef MAIN_HPP
#define MAIN_HPP
#include <iostream>
#include <tuple>
#include <bitset>

template<typename T, typename std::enable_if<std::is_integral<T>::value, void>::type ... >
static inline auto utype (T _t) noexcept
{
	return _t;
}

template<typename E, typename std::enable_if<std::is_enum<E>::value, void>::type ... >
static inline auto utype (E _e) noexcept
{
	return static_cast<typename std::underlying_type<E>::type>(_e);
}

template<typename T, typename ... Rest, typename std::enable_if<sizeof... (Rest) == 0>::type ... >
static inline constexpr size_t tp_size()
{
	return sizeof(T);
}

template<typename T, typename ... Rest, typename std::enable_if<sizeof... (Rest) != 0>::type ...>
static inline constexpr size_t tp_size()
{
	return sizeof (T) + tp_size<Rest...>();
}

template<size_t bs, size_t shift, typename T>
static inline void hashme(std::bitset<bs>& _h, T _t)
{
	std::bitset<bs> h_aux(_t);
//	std::cout << "(hashme) h_aux:  " << h_aux.to_string() << "\n";
	h_aux <<= 8 * shift;
//	std::cout << "(hashme) h_aux:  " << h_aux.to_string() << "\n";
	_h |= h_aux;
}

template<size_t bs, size_t elem, size_t shift, typename T, typename ... Rest, typename std::enable_if<elem == sizeof ... (Rest)>::type ...>
static inline size_t bitset_hash(std::bitset<bs>&& _h, std::tuple<T, Rest...>&& _tpl)
{
	hashme<bs, shift>(_h, utype(std::get<elem>(_tpl)));
//	std::cout << "(bitset_hash) h: " << _h.to_string() << "\n";
	return std::hash<std::bitset<bs>>()(_h);
}

template<size_t bs, size_t elem, size_t shift, typename T, typename ... Rest, typename std::enable_if<elem != sizeof ... (Rest)>::type ...>
static inline size_t bitset_hash(std::bitset<bs>&& _h, std::tuple<T, Rest...>&& _tpl)
{
	hashme<bs, shift>(_h, utype(std::get<elem>(_tpl)));
//	std::cout << "(bitset_hash) h: " << _h.to_string() << "\n";
	return bitset_hash<bs, elem + 1, shift + sizeof(decltype (std::get<elem>(_tpl))), T, Rest...>(std::forward<decltype (_h)>(_h), std::forward<decltype (_tpl)>(_tpl));
}

template<typename T, typename ... Rest>
static inline size_t tuple_hash(std::tuple<T, Rest ...>&& _tpl)
{
	std::bitset<8 * tp_size<T, Rest...>()> h(utype(std::get<0>(_tpl)));
//	std::cout << "(tuple_hash) h:  " << h.to_string() << "\n";
	return bitset_hash<8 * tp_size<T, Rest...>(), 1, sizeof (T), T, Rest...>(std::move(h), std::forward<decltype (_tpl)>(_tpl));
}

#endif // MAIN_HPP
