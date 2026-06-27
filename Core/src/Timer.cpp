#include "pch.hpp"
#include "Timer.hpp"

#include <stdexcept>

namespace Zenyth {

	Timer::Timer() {
		LARGE_INTEGER freq, now;

		if (!::QueryPerformanceFrequency(&freq))
			throw std::runtime_error("Timer: QueryPerformanceFrequency not supported");

		m_frequency = freq.QuadPart;

		::QueryPerformanceCounter(&now);
		m_startTime = now.QuadPart;
		m_prevTime = now.QuadPart;
	}

	void Timer::Reset() {
		LARGE_INTEGER now;
		::QueryPerformanceCounter(&now);

		m_startTime = now.QuadPart;
		m_prevTime = now.QuadPart;
		m_deltaTime = 0.f;
		m_totalTime = 0.0;
		m_frameCount = 0;
	}

	float Timer::Tick() {
		LARGE_INTEGER now;
		::QueryPerformanceCounter(&now);

		m_deltaTime = static_cast<float>(now.QuadPart - m_prevTime)
			/ static_cast<float>(m_frequency);
		m_totalTime = static_cast<double>(now.QuadPart - m_startTime)
			/ static_cast<double>(m_frequency);
		m_prevTime = now.QuadPart;
		++m_frameCount;

		return m_deltaTime;
	}

} // namespace Zenyth
