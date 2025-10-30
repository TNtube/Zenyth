#pragma once

namespace Zenyth
{
	class CommandBatch;

	namespace EngineProfiling
	{
		void Update();

		void BeginBlock(const std::string& name, const CommandBatch* batch = nullptr);
		void EndBlock(const CommandBatch* batch = nullptr);

		void OnImGui();
	}

#ifdef NDEBUG
	class ScopedTimer
	{
	public:
		explicit ScopedTimer(const std::string&) {}
		ScopedTimer(const std::string&, CommandBatch&) {}
	};
#else
	class ScopedTimer
	{
	public:
		explicit ScopedTimer( const std::string& name ) : m_Context(nullptr)
		{
			EngineProfiling::BeginBlock(name);
		}
		ScopedTimer( const std::string& name, CommandBatch& Context ) : m_Context(&Context)
		{
			EngineProfiling::BeginBlock(name, m_Context);
		}
		~ScopedTimer()
		{
			EngineProfiling::EndBlock(m_Context);
		}

	private:
		CommandBatch* m_Context;
	};
#endif


}