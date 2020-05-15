#pragma once

BRWL_NS


namespace EventSysInternal
{
	template<typename EventType, EventType eventValue>
	struct ConversionDeclaration
	{
		// std::is_same_v is only used to make the static assert only be evaluted if this template is used as a fallback for a non existent specialization
		// If this fails then an event param is tried to be casted but there is no mapping which tells the resulting type.
		static_assert(std::integral_constant<bool, false>::value&& std::is_same_v<EventType, int>);
	};
}

template<auto EventVal>
typename EventSysInternal::ConversionDeclaration<decltype(EventVal), EventVal>::ResultTypeT* castParam(void* param)
{
	return static_cast<typename EventSysInternal::ConversionDeclaration<decltype(EventVal), EventVal>::ResultTypeT*>(param);
}

#define EVENT_PARAMETER_MAP_START namespace EventSysInternal {
#define EVENT_PARAMETER_MAP_END }
#define	MAP_EVENT_PARAMETER(EventType, EventVal, ResultType) \
template<> struct ConversionDeclaration<EventType, EventType::EventVal> \
{ \
	using ResultTypeT = ResultType; \
};


BRWL_NS_END