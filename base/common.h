#ifndef __COMMON_H
#define __COMMON_H

#ifdef WIN32

#ifdef BASE_EXPORTS
#define BASE_API __declspec(dllexport)
#else
#define BASE_API __declspec(dllimport)
#endif

#else

#ifdef BASE_EXPORTS
#define BASE_API 
#else
#define BASE_API 
#endif

#endif

#pragma warning( disable: 4251 )

#define BEGIN_NS(ns_name) namespace ns_name{
#define END_NS(ns_name) }

//#include <memory>
//
//template<typename To, typename From>
//inline To static_cast(From const& f)
//{
//	return f;
//}
//
//// should really belong to base/Types.h, but <memory> is not included there.
//
//template<typename T>
//inline T* get_pointer(const std::shared_ptr<T>& ptr)
//{
//	return ptr.get();
//}
//
//template<typename T>
//inline T* get_pointer(const std::unique_ptr<T>& ptr)
//{
//	return ptr.get();
//}
//
//template<typename To, typename From>
//inline ::std::shared_ptr<To> down_pointer_cast(const ::std::shared_ptr<From>& f) {
//	if (false)
//	{
//		static_cast<From*, To*>(0);
//	}
//
//#ifndef NDEBUG
//	assert(f == nullptr || dynamic_cast<To*>(get_pointer(f)) != nullptr);
//#endif
//	return ::std::static_pointer_cast<To>(f);
//}

#endif