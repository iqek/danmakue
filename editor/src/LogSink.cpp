#include "LogSink.h"

namespace Editor {

// no color codes here, they're only meaningful to terminal sinks
LogSink::LogSink(LogPanel& panel): panel(panel){
	set_pattern("[%T] %n: %v");
}

void LogSink::sink_it_(const spdlog::details::log_msg& msg){
	spdlog::memory_buf_t formatted;
	formatter_->format(msg, formatted);
	panel.AddLine(std::string(formatted.data(), formatted.size()));
}

}
