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

		inline static std::shared_ptr<spdlog::logger>& getLogger() { return s_logger; }

	};

}

// Log macros
#define GLEZ_TRACE(...)    ::glez::log::getLogger()->trace(__VA_ARGS__)
#define GLEZ_INFO(...)     ::glez::log::getLogger()->info(__VA_ARGS__)
#define GLEZ_WARN(...)     ::glez::log::getLogger()->warn(__VA_ARGS__)
#define GLEZ_ERROR(...)    ::glez::log::getLogger()->error(__VA_ARGS__)
#define GLEZ_CRITICAL(...) ::glez::log::getLogger()->critical(__VA_ARGS__)
