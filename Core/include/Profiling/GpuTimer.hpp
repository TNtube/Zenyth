#pragma once
namespace Zenyth
{
	class CommandBatch;

	namespace GpuTimeManager
	{
		void Init();
		void Destroy();

		void BeginReadback();
		void EndReadback();


		float GetTime(uint32_t timer);
	}


	class GpuTimer
	{
	public:
		GpuTimer();

		void Begin(const CommandBatch& batch) const;
		void End(const CommandBatch& batch) const;

		[[nodiscard]] float GetTime() const { return GpuTimeManager::GetTime(m_timer); }
	private:
		uint32_t m_timer;
	};
}
