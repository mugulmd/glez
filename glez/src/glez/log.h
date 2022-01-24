#pragma once

#include <memory>

#include "core.h"

#include "spdlog/spdlog.h"

namespace glez {

	class GLEZ_API log 
	{
	private:
		static std::shared_ptr<spdlog::logger> s_logger;

	public:
		static void init();

		inline static std::shared_ptr<spdlog::logger>& get_logger() { return s_logger; }

	};

}

// Log macros
#define GLEZ_TRACE(...)    ::glez::log::get_logger()->trace(__VA_ARGS__)
#define GLEZ_INFO(...)     ::glez::log::get_logger()->info(__VA_ARGS__)
#define GLEZ_WARN(...)     ::glez::log::get_logger()->warn(__VA_ARGS__)
#define GLEZ_ERROR(...)    ::glez::log::get_logger()->error(__VA_ARGS__)
#define GLEZ_CRITICAL(...) ::glez::log::get_logger()->critical(__VA_ARGS__)
