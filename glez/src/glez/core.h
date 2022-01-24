#pragma once

#ifdef GLEZ_EXPORTS
	#define GLEZ_API __declspec(dllexport)
#else
	#define GLEZ_API __declspec(dllimport)
#endif
