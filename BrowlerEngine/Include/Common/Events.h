#pragma once

BRWL_NS


//enum class CommonEvent {
//	MAX,
//	MIN = 0
//};


template<typename Event, Event event>
struct TypeConvert {
	static_assert(std::integral_constant<bool, false>, "No type convert defined for this type.");
};


//template<CommonEvent event>
//typename TypeConvert<CommonEvent, event>::ResultType* castParam(void* param) {
//	return static_cast<typename TypeConvert<event>::ResultType*>(param);
//}





BRWL_NS_END