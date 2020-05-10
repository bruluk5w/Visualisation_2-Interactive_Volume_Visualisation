#pragma once

BRWL_NS


template<typename Event, Event event>
struct TypeConvert {
	// std::is_same_v is only used to make the static assert only be evaluted if this template is used as a fallback for a non existent specialization
	static_assert(std::integral_constant<bool, false>::value && std::is_same_v<Event, int>, "No type convert defined for this type. Use the the macros below to define a specialization for your event.");
};


// Defines specific overloads which provide the association between event enum value and the event parameter type for the function defined by EVENT_PARAMETER_MAP(EventType)
#define MAP_EVENT_PARAMETER(EventType, EnumVal, ParameterType, ...) \
template<> \
struct TypeConvert<EventType::EnumVal> { \
	using ResultType = ParameterType; \
}; \

// Defines a function for casting the event parameter void* to the right data type associated with the enum value of the event
#define EVENT_PARAMETER_MAP(EventType, ...) \
template<EventType event> \
typename TypeConvert<EventType, event>::ResultType* castParam(void* param) { \
	return static_cast<typename TypeConvert<event>::ResultType*>(param); \
} \
MAP_EVENT_PARAMETER(EventType, __VA_ARGS__)


BRWL_NS_END