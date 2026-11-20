#include "pans/logger/logger.h"
#include "logger/formatter.h"
#include "logger/logger_impl.h"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include <pans/macros.h>
#include "logger/appender_impl.h"
#include "logger/buffer.h"
#include "pans/logger/appender.h"

namespace pans{

constexpr std::string_view DEFAULT_LOG_NAME = "%d{%Y-%m-%d %H:%M:%S}.%u%Tthread-%t%Tfiber=%F%T[%p]%T%f:%l%T%m%n";

Logger::Logger(std::string name) 
    : m_impl(std::make_unique<Impl>(std::move(name)))
{};

Logger::~Logger() = default;

bool Logger::shouldLog(LogLevel::Level level) const noexcept
{
    return m_impl -> shouldLog(level);
}

void Logger::setLevel(LogLevel::Level level) noexcept
{
    m_impl -> setLevel(level);
}

LogLevel::Level Logger::getLevel() const noexcept
{
    return m_impl -> getLevel();
}

std::string_view Logger::getName() const noexcept
{
    return m_impl -> getName();
}

void Logger::setFormatter(std::string_view pattern)
{
    auto formatter = std::make_shared<const detail::Formatter>(pattern);
    // ASSERT_RETNONE2(formatter -> isValid(), "invalid logger format pattern");
    m_impl -> setFormatter(std::move(formatter));
}

std::string Logger::getFormatterPattern() const
{
    const auto formatter = m_impl -> getFormatter();
    return formatter == nullptr ? std::string() : formatter -> getPattern();
}

void Logger::addAppender(AppenderPtr appender)
{
    // TODO
    m_impl -> addAppender(std::move(appender));
}

void Logger::removeAppender(const AppenderPtr& appender)
{
    m_impl->removeAppender(appender);
}

void Logger::clearAppenders()
{
    m_impl -> clearAppenders();
}

void Logger::flush()
{
    m_impl -> flush();
}

void Logger::sync()
{
    m_impl -> sync();
}

Logger::Impl::Impl(std::string name)
    : m_name(std::move(name))
{
    if(m_name.empty())
    {
        throw std::invalid_argument("logger name should not be empty");
    }
    m_formatter = std::make_shared<const detail::Formatter>(DEFAULT_LOG_NAME);
}

bool Logger::Impl::shouldLog(LogLevel::Level level) const noexcept
{
    if(static_cast<u8>(level) < static_cast<u8>(getLevel()))
    {
        return false;
    }
    {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        if(!m_appenders.empty())
        {
            return true;
        }
    }
    return m_root != nullptr && m_root->shouldLog(level);
}

}