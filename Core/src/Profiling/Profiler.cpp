#include "pch.hpp"
#include "Profiling/Profiler.hpp"

#include "imgui.h"
#include "StepTimer.hpp"
#include "Profiling/GpuTimer.hpp"


namespace
{
	int64_t GetCurrentTick()
	{
		LARGE_INTEGER currentTick;
		QueryPerformanceCounter(&currentTick);
		return currentTick.QuadPart;
	}

	class BufferedValue
	{
	public:
		BufferedValue() : m_history{}, m_last(0), m_average(0), m_minimum(0), m_maximum(0)
		{
			for (float & i : m_history)
				i = 0.0f;
		}

		void RecordStat(const uint32_t FrameIndex, const float Value )
		{
			m_history[FrameIndex % HistorySize] = Value;
			m_last = Value;

			uint32_t usableCount = 0;
			m_minimum = std::numeric_limits<float>::max();
			m_maximum = 0.0f;
			m_average = 0.0f;

			for (float val : m_history)
			{
				if (val > 0.0f)
				{
					++usableCount;
					m_average += val;
					m_minimum = std::min(val, m_minimum);
					m_maximum = std::max(val, m_maximum);
				}
			}

			if (usableCount > 0)
				m_average /= static_cast<float>(usableCount);
			else
				m_minimum = 0.0f;
		}

		[[nodiscard]] float GetLast() const { return m_last; }
		[[nodiscard]] float GetAvg() const { return m_average; }

	private:
		static constexpr uint32_t HistorySize = 64;
		float m_history[HistorySize];
		float m_last;
		float m_average;
		float m_minimum;
		float m_maximum;
	};


	class TimerNode
	{
	public:
		explicit TimerNode(std::string name, TimerNode* parent = nullptr) : m_parent(parent), m_name(std::move(name)) {}

		[[nodiscard]] TimerNode* Parent() const { return m_parent; }

		[[nodiscard]] const std::vector<std::unique_ptr<TimerNode>>& GetChildren() const { return m_children; }
		[[nodiscard]] const std::string& GetName() const { return m_name; }

		[[nodiscard]] float GetCpuTime() const { return m_cpuTime.GetAvg(); }
		[[nodiscard]] float GetGpuTime() const { return m_gpuTime.GetAvg(); }

		TimerNode* GetChild(const std::string& name)
		{
			const auto iter = m_LUT.find(name);
			if (iter != m_LUT.end())
				return iter->second;

			auto& node = m_children.emplace_back();

			node = std::make_unique<TimerNode>(name, this);
			m_LUT[name] = node.get();

			return node.get();
		}

		void StartTimer(const CommandBatch* batch)
		{
			m_startTick = GetCurrentTick();
			if (batch)
				m_gpuTimer.Begin(*batch);
		}

		void EndTimer(const CommandBatch* batch)
		{
			m_endTick = GetCurrentTick();
			if (batch)
				m_gpuTimer.End(*batch);
		}

		void ComputeTimes(const uint32_t frame)
		{
			m_cpuTime.RecordStat(frame, StepTimer::TicksToMilliseconds(m_endTick - m_startTick));
			m_gpuTime.RecordStat(frame,  1000 * m_gpuTimer.GetTime());

			for (const auto& child : m_children)
				child->ComputeTimes(frame);

			m_startTick = 0;
			m_endTick = 0;
		}

		void SumAllTimes(float& cpu, float& gpu) const
		{
			cpu = 0.0f;
			gpu = 0.0f;
			for (const auto& child : m_children)
			{
				cpu += child->m_cpuTime.GetLast();
				gpu += child->m_gpuTime.GetLast();
			}
		}

	private:
		TimerNode* m_parent;
		std::string m_name;
		std::vector<std::unique_ptr<TimerNode>> m_children;
		std::unordered_map<std::string, TimerNode*> m_LUT;
		uint64_t m_startTick = 0;
		uint64_t m_endTick = 0;
		GpuTimer m_gpuTimer;
		BufferedValue m_cpuTime;
		BufferedValue m_gpuTime;
	};


	uint32_t FrameIndex = 1;
	auto s_rootNode = std::make_unique<TimerNode>("Total");
	TimerNode* s_currentNode = s_rootNode.get();
	BufferedValue s_totalCpuTime;
	BufferedValue s_totalGpuTime;

}

void EngineProfiling::Update()
{
	const auto frame = FrameIndex++;

	GpuTimeManager::BeginReadback();
	s_rootNode->ComputeTimes(frame);
	GpuTimeManager::EndReadback();

	float TotalCpuTime, TotalGpuTime;
	s_rootNode->SumAllTimes(TotalCpuTime, TotalGpuTime);
	s_totalCpuTime.RecordStat(frame, TotalCpuTime);
	s_totalGpuTime.RecordStat(frame, TotalGpuTime);
}

void EngineProfiling::BeginBlock(const std::string& name, const CommandBatch* batch)
{
	s_currentNode = s_currentNode->GetChild(name);
	s_currentNode->StartTimer(batch);
}

void EngineProfiling::EndBlock(const CommandBatch* batch)
{
	s_currentNode->EndTimer(batch);
	s_currentNode = s_currentNode->Parent();
}


void DisplayNode(const TimerNode* node, const bool root = false)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	const bool is_folder = !node->GetChildren().empty();

	constexpr ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_LabelSpanAllColumns;

	const auto cpu = root ? s_totalCpuTime.GetAvg() : node->GetCpuTime();
	const auto gpu = root ? s_totalGpuTime.GetAvg() : node->GetGpuTime();

	if (is_folder)
	{
		const auto open = ImGui::TreeNodeEx(node->GetName().c_str(), node_flags);

		ImGui::TableNextColumn();
		ImGui::Text("%f", cpu);
		ImGui::TableNextColumn();
		ImGui::Text("%f", gpu);

		if (open)
		{

			for (const auto& child : node->GetChildren())
				DisplayNode(child.get());
			ImGui::TreePop();
		}
	}
	else
	{
		ImGui::TreeNodeEx(node->GetName().c_str(), node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
		ImGui::TableNextColumn();
		ImGui::Text("%f", cpu);
		ImGui::TableNextColumn();
		ImGui::Text("%f", gpu);
	}
}

void EngineProfiling::OnImGui()
{
	static ImGuiTableFlags table_flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
	const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;

	if (ImGui::BeginTable("3ways", 3, table_flags))
	{
		// The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
		ImGui::TableSetupColumn("Cpu Timer", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
		ImGui::TableSetupColumn("Gpu Timer", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
		ImGui::TableHeadersRow();

		DisplayNode(s_rootNode.get(), true);

		ImGui::EndTable();
	}
}
