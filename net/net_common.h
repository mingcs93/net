#ifndef __NET_COMMON_H
#define __NET_COMMON_H

#ifdef WIN32

#ifdef NET_EXPORTS
#define NET_API __declspec(dllexport)
#else
#define NET_API __declspec(dllimport)
#endif

#else

#ifdef NET_EXPORTS
#define NET_API 
#else
#define NET_API 
#endif

#endif

#pragma warning( disable: 4251 )

#endif