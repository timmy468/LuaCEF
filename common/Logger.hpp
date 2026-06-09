#pragma once

#include <filesystem>
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

class Logger
{
  private:
    std::shared_ptr<spdlog::logger> logger_;

  private:
    Logger() = default;

    ~Logger() = default;

  public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;
    Logger(const Logger&&) = delete;
    Logger& operator=(const Logger&&) = delete;

    static Logger& Instance()
    {
        static Logger instance;
        return instance;
    }

    void SetLogFilePath(const std::filesystem::path& logFilePath)
    {
        try
        {
            std::filesystem::create_directories(logFilePath.parent_path());

            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);

            std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
            logger_ = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());

            // КРИТИЧНО: устанавливаем уровень логирования
            logger_->set_level(spdlog::level::trace); // Самый подробный уровень

            // КРИТИЧНО: сбрасываем буфер после каждого сообщения (для отладки)
            logger_->flush_on(spdlog::level::trace);

            logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

            spdlog::register_logger(logger_);
            spdlog::set_default_logger(logger_);

            logger_->info("Logger initialized successfully, log file: {}", logFilePath.string());
        }
        catch (const spdlog::spdlog_ex& ex)
        {
            throw std::runtime_error("Failed to initialize logger: " + std::string(ex.what()));
        }
    }

    template <typename... Args> void Log(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        if (logger_)
        {
            logger_->info(fmt, std::forward<Args>(args)...);
        }
        else
        {
            // Fallback to console only if logger not initialized
            spdlog::info(fmt, std::forward<Args>(args)...);
        }
    }

    // Convenience methods for different log levels
    template <typename... Args> void Debug(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        if (logger_)
            logger_->debug(fmt, std::forward<Args>(args)...);
        else
            spdlog::debug(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void Warn(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        if (logger_)
            logger_->warn(fmt, std::forward<Args>(args)...);
        else
            spdlog::warn(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void Error(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        if (logger_)
            logger_->error(fmt, std::forward<Args>(args)...);
        else
            spdlog::error(fmt, std::forward<Args>(args)...);
    }

    // Set log level
    void SetLevel(spdlog::level::level_enum level)
    {
        if (logger_)
            logger_->set_level(level);
        spdlog::set_level(level);
    }

    // Flush logs
    void Flush()
    {
        if (logger_)
            logger_->flush();
        spdlog::flush_every(std::chrono::seconds(3));
    }
};