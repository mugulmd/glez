#include "log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace glez {

	std::shared_ptr<spdlog::logger> log::s_logger;

	void log::init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_logger = spdlog::stdout_color_mt("GLEZ");
		s_logger->set_level(spdlog::level::trace);
	}

}
