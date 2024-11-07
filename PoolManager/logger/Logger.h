#pragma once
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <format>


class Logger final
{
private:
	mutable std::ofstream m_logFile;
	mutable std::mutex m_mutex;
	std::filesystem::path m_logFilePath;

	explicit Logger(const char* logfileName, bool truncate) noexcept;
	~Logger() noexcept;

	template<class T>
	friend struct std::default_delete;
public:
	static std::unique_ptr<Logger> create_logger(const char* logfileName, bool truncate) noexcept;

	template <typename... Types>
	void Write(const std::string_view str, Types&&... args) const&;
};

Logger::Logger(const char* logfileName, bool truncate) noexcept
	: m_logFilePath(std::filesystem::current_path().append(std::format("{}.log", logfileName)))
	, m_logFile{}
{
	if (truncate)
	{
		m_logFile.open(m_logFilePath, std::ofstream::out | std::ofstream::trunc);
	}
	else
	{
		m_logFile.open(m_logFilePath, std::ofstream::out | std::ofstream::app);

	}
}
Logger::~Logger() noexcept
{
	m_logFile.close();
	m_logFilePath.clear();
}

template <typename... Types>
void Logger::Write(const std::string_view str, Types&&... args) const&
{
	std::scoped_lock<std::mutex> lock(m_mutex);
	m_logFile << std::vformat(str, std::make_format_args(args...)) << std::endl;
}

std::unique_ptr<Logger> Logger::create_logger(const char* logfileName, bool truncate) noexcept
{
	std::unique_ptr<Logger> instance{ new Logger(logfileName, truncate) };
	return std::move(instance);
}