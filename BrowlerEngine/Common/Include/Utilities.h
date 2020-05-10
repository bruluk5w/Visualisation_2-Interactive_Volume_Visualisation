#pragma once

BRWL_NS


#define PI_F 3.141592653589793f
#define DEG_2_RAD_F 0.017453292519943f
#define RAD_2_DEG_F 57.295779513082320f

namespace Utils
{

	//min/max
	template<typename T>
	constexpr const T& min(const T& a, const T& b) { return a < b ? a : b; }
	template<typename T>
	constexpr const T& max(const T& a, const T& b) { return a < b ? b : a; }

	template <class... Types>
	struct type_list
	{
		template <std::size_t N>
		using type = typename std::tuple_element<N, std::tuple<Types...>>::type;
		static const int length = std::tuple_size<std::tuple<Types...>>::value;
	};

	constexpr uint32_t nextPowerOfTwo(uint32_t x) { x--; x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; return (x | (x >> 16)) + 1; }
	constexpr uint16_t nextPowerOfTwo(uint16_t x) { x--; x |= x >> 1; x |= x >> 2; x |= x >> 4; return (x | (x >> 8)) + 1; }
	constexpr uint8_t nextPowerOfTwo(uint8_t x) { x--; x |= x >> 1; x |= x >> 2; return (x | (x >> 4)) + 1; }

	template<typename T>
	using is_enum_class = std::integral_constant<bool, std::is_enum<T>::value && !std::is_convertible_v<T, int>>;

}

// count of elemets in c array
template<size_t N, typename T> constexpr size_t countof(T(&)[N]) { return N; }
#define ITERATE_ENUM_CLASS(ENUM, VAR, BEGIN, END) \
	for(ENUM VAR = ENUM::BEGIN; VAR < ENUM::END; VAR = static_cast<ENUM>(static_cast<std::underlying_type<ENUM>::type>(VAR) + 1))

#define ENUM_CLASS_TO_NUM(VAL) static_cast<std::underlying_type<std::remove_reference<decltype(VAL)>::type>::type>(VAL)

#define DEFINE_ENUM_CLASS_OPERATORS(ENUMTYPE) \
extern "C++" { \
	inline ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) { return  static_cast<ENUMTYPE>(ENUM_CLASS_TO_NUM(a) | ENUM_CLASS_TO_NUM(b)); } \
	inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) { a = static_cast<ENUMTYPE>(ENUM_CLASS_TO_NUM(a) | ENUM_CLASS_TO_NUM(b)); return a; } \
	inline ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) { return  static_cast<ENUMTYPE>(ENUM_CLASS_TO_NUM(a) & ENUM_CLASS_TO_NUM(b)); } \
	inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) { a = static_cast<ENUMTYPE>(ENUM_CLASS_TO_NUM(a) & ENUM_CLASS_TO_NUM(b)); return a; } \
	inline ENUMTYPE operator ~ (ENUMTYPE a) { return  static_cast<ENUMTYPE>(~ENUM_CLASS_TO_NUM(a)); } \
	inline ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) { return  static_cast<ENUMTYPE>(ENUM_CLASS_TO_NUM(a) ^ ENUM_CLASS_TO_NUM(b)); } \
	inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) { a = static_cast<ENUMTYPE>(ENUM_CLASS_TO_NUM(a) ^ ENUM_CLASS_TO_NUM(b)); return a; }\
	inline bool operator!(ENUMTYPE a) { return !static_cast<bool>(a); } \
}


BRWL_NS_END