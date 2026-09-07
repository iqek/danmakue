#pragma once

#include "LogPanel.h"

#include <mutex>
#include <spdlog/sinks/base_sink.h>

namespace Editor {

// Forwards formatted spdlog messages into a LogPanel instead of a stream
class LogSink : public spdlog::sinks::base_sink<std::mutex> {
private:
	LogPanel& panel;

protected:
	void sink_it_(const spdlog::details::log_msg& msg) override;
	void flush_() override {}

public:
	explicit LogSink(LogPanel& panel);
};

}
