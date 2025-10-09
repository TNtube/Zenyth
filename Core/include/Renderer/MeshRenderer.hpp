#pragma once
#include "SubmeshRenderer.hpp"
#include "Data/Mesh.hpp"

namespace Zenyth
{
	class MeshRenderer
	{
	public:
		explicit MeshRenderer(const Mesh& mesh, DescriptorHeap& resourceHeap);

		void Submit(CommandBatch& commandBatch) const;

	private:
		std::vector<SubmeshRenderer> m_submeshRenderers;
	};
}
