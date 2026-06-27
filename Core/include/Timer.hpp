#pragma once
#include <cstdint>

namespace Zenyth {

	class Timer {
	public:
		Timer();

		// Call once per frame. Returns delta time in seconds.
		float Tick();

		void Reset();

		[[nodiscard]] float    DeltaTime()  const { return m_deltaTime; }
		[[nodiscard]] double   TotalTime()  const { return m_totalTime; }
		[[nodiscard]] uint64_t FrameCount() const { return m_frameCount; }

	private:
		int64_t  m_frequency = 0;
		int64_t  m_prevTime = 0;
		int64_t  m_startTime = 0;

		float    m_deltaTime = 0.f;
		double   m_totalTime = 0.0;
		uint64_t m_frameCount = 0;
	};

} // namespace Zenyth
