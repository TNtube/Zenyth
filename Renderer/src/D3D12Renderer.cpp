#include "pch.hpp"
#include "Texture.hpp"
#include "D3D12Renderer.hpp"

#include "Filesystem.hpp"
#include "stb_image.h"

using Microsoft::WRL::ComPtr;

std::string HrToString(const HRESULT hr)
{
	char s_str[64] = {};
	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return {s_str};
}

class HrException : public std::runtime_error
{
public:
	explicit HrException(const HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
	HRESULT Error() const { return m_hr; }
private:
	const HRESULT m_hr;
};

inline void ThrowIfFailed(const HRESULT hr)
{
	if (FAILED(hr))
	{
		throw HrException(hr);
	}
}

std::wstring GetAssetFullPath(LPCWSTR assetName)
{
	std::wstring assetPath = L"assets/";
	return assetPath + assetName;
}

namespace zenyth::renderer {
	void D3D12Renderer::Init(const HWND hwnd, const uint32_t width, const uint32_t height)
	{
		m_hwnd = hwnd;

		m_width = width;
		m_height = height;

		m_viewport.Width = static_cast<float>(width);
		m_viewport.Height = static_cast<float>(height);
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		m_scissorRect.right = m_scissorRect.left + static_cast<LONG>(width);
		m_scissorRect.bottom = m_scissorRect.top + static_cast<LONG>(height);

		LoadPipeline();
		LoadAssets();
	}

	void D3D12Renderer::LoadPipeline() {

		UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
		// Enable the debug layer (requires the Graphics Tools "optional feature").
		// NOTE: Enabling the debug layer after device creation will invalidate the active device.
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();

				// Enable additional debug layers.
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
		}
#endif

		ComPtr<IDXGIFactory4> factory;
		ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));
		
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter);

		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
			));

		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.Width = m_width;
		swapChainDesc.Height = m_height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> swapChain;
		ThrowIfFailed(factory->CreateSwapChainForHwnd(
			m_commandQueue.Get(),		// Swap chain needs the queue so that it can force a flush on it.
			m_hwnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
			));

		// This sample does not support fullscreen transitions.
		ThrowIfFailed(factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));

		ThrowIfFailed(swapChain.As(&m_swapChain));
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		// Create descriptor heaps.
		{
			// Describe and create a render target view (RTV) descriptor heap.
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = FrameCount*10;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

			m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			// Describe and create a shader resource view (SRV) heap for the texture.
			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
			srvHeapDesc.NumDescriptors = FrameCount*10;
			srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

			m_cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			// Describe and create a depth stencil view (DSV) descriptor heap.
			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
			dsvHeapDesc.NumDescriptors = 1;
			dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
		}

		// create depth stencil resources
		{
			CD3DX12_RESOURCE_DESC depthResourceDesc(
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				0,
				static_cast<UINT>(m_viewport.Width),
				static_cast<UINT>(m_viewport.Height),
				1,
				1,
				DXGI_FORMAT_D32_FLOAT,
				1,
				0,
				D3D12_TEXTURE_LAYOUT_UNKNOWN,
				D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

			D3D12_CLEAR_VALUE clearValue;
			clearValue.Format = DXGI_FORMAT_D32_FLOAT;
			clearValue.DepthStencil.Depth = 1.0f;
			clearValue.DepthStencil.Stencil = 0;

			const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
			ThrowIfFailed(m_device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&depthResourceDesc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&clearValue,
				IID_PPV_ARGS(&m_depthStencil)));

			m_device->CreateDepthStencilView(m_depthStencil.Get(), nullptr, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		}

		// Create frame resources.
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

			// Create a RTV for each frame.
			for (UINT n = 0; n < FrameCount; n++)
			{
				auto& frameResource = m_frameResources[n];
				ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&frameResource.RenderTarget)));
				m_device->CreateRenderTargetView(frameResource.RenderTarget.Get(), nullptr, rtvHandle);
				rtvHandle.Offset(1, m_rtvDescriptorSize);


				ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frameResource.CommandAllocator)));

				// Create the command list.
				ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frameResource.CommandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&frameResource.CommandList)));
				ThrowIfFailed(frameResource.CommandList->Close());
			}
		}
	}

	void D3D12Renderer::LoadAssets()
	{
		// Create an empty root signature.
		{
			D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

			// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

			if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
			{
				featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
			}

			CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

			CD3DX12_ROOT_PARAMETER1 rootParameters[2];
			rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
			rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

			D3D12_STATIC_SAMPLER_DESC sampler = {};
			sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler.MipLODBias = 0;
			sampler.MaxAnisotropy = 0;
			sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
			sampler.MinLOD = 0.0f;
			sampler.MaxLOD = D3D12_FLOAT32_MAX;
			sampler.ShaderRegister = 0;
			sampler.RegisterSpace = 0;
			sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


			// Allow input layout and deny uneccessary access to certain pipeline stages.
			D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
			rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			ComPtr<ID3DBlob> signature;
			ComPtr<ID3DBlob> error;
			ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
			ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
		}

		// Create the pipeline state, which includes compiling and loading shaders.
		{
			auto vertexShaderData = fs::readFileToByteArray(GetAssetFullPath(L"shaders_VSMain.cso"));
			auto pixelShaderData = fs::readFileToByteArray(GetAssetFullPath(L"shaders_PSMain.cso"));

			// Define the vertex input layout.
			D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			// Describe and create the graphics pipeline state object (PSO).

			CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
			depthStencilDesc.DepthEnable = true;
			depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			depthStencilDesc.StencilEnable = FALSE;

			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
			psoDesc.pRootSignature = m_rootSignature.Get();
			psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShaderData.data(), vertexShaderData.size());
			psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShaderData.data(), pixelShaderData.size());
			psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			psoDesc.DepthStencilState = depthStencilDesc;
			psoDesc.SampleMask = UINT_MAX;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets = 1;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			psoDesc.SampleDesc.Count = 1;
			ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
		}


		// Create the vertex buffer.
		{
			float aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);

			// Define the geometry for a triangle.
			Vertex triangleVertices[] =
			{
				{ { 0.0f, 0.25f * aspectRatio, 0.0f }, { 0.5f, 0.0f } },
				{ { 0.25f, -0.25f * aspectRatio, 0.0f }, { 1.0f, 1.0f } },
				{ { -0.25f, -0.25f * aspectRatio, 0.0f }, { 0.0f, 1.0f } }
			};

			constexpr UINT vertexBufferSize = sizeof(triangleVertices);

			// Note: using upload heaps to transfer static data like vert buffers is not
			// recommended. Every time the GPU needs it, the upload heap will be marshalled
			// over. Please read up on Default Heap usage. An upload heap is used here for
			// code simplicity and because there are very few verts to actually transfer.
			auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

			ThrowIfFailed(m_device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_vertexBuffer)));

			// Copy the triangle data to the vertex buffer.
			UINT8* pVertexDataBegin;
			CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
			ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
			memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
			m_vertexBuffer->Unmap(0, nullptr);

			// Initialize the vertex buffer view.
			m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
			m_vertexBufferView.StrideInBytes = sizeof(Vertex);
			m_vertexBufferView.SizeInBytes = vertexBufferSize;
		}

		// Create the constant buffer.
		for (int n = 0; n < FrameCount; n++)
		{
			auto& frameResource = m_frameResources[n];
			const UINT constantBufferSize = sizeof(SceneConstantBuffer);    // CB size is required to be 256-byte aligned.
			CD3DX12_HEAP_PROPERTIES cboHeapProps(D3D12_HEAP_TYPE_UPLOAD);
			{
				CD3DX12_RESOURCE_DESC cboBufferDesc =CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);
				ThrowIfFailed(m_device->CreateCommittedResource(
					&cboHeapProps,
					D3D12_HEAP_FLAG_NONE,
					&cboBufferDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&frameResource.ConstantBufferA)));

				// Describe and create a constant buffer view.
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.BufferLocation = frameResource.ConstantBufferA->GetGPUVirtualAddress();
				cbvDesc.SizeInBytes = constantBufferSize;
				CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), n * ResourceCount, m_cbvSrvDescriptorSize);
				m_device->CreateConstantBufferView(&cbvDesc, cbvHandle);

				// Map and initialize the constant buffer. We don't unmap this until the
				// app closes. Keeping things mapped for the lifetime of the resource is okay.
				CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
				ThrowIfFailed(frameResource.ConstantBufferA->Map(0, &readRange, reinterpret_cast<void**>(&frameResource.pCbvDataBeginA)));
				m_transformA.position.z() = 0.5f;
				m_constantBufferDataA.transform = m_transformA.getMatrix();
				memcpy(frameResource.pCbvDataBeginA, &m_constantBufferDataA, sizeof(m_constantBufferDataA));
			}
			{
				CD3DX12_RESOURCE_DESC cboBufferDesc =CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);
				ThrowIfFailed(m_device->CreateCommittedResource(
					&cboHeapProps,
					D3D12_HEAP_FLAG_NONE,
					&cboBufferDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&frameResource.ConstantBufferB)));

				// Describe and create a constant buffer view.
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.BufferLocation = frameResource.ConstantBufferB->GetGPUVirtualAddress();
				cbvDesc.SizeInBytes = constantBufferSize;
				CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), n * ResourceCount + 1, m_cbvSrvDescriptorSize);
				m_device->CreateConstantBufferView(&cbvDesc, cbvHandle);

				// Map and initialize the constant buffer. We don't unmap this until the
				// app closes. Keeping things mapped for the lifetime of the resource is okay.
				CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
				ThrowIfFailed(frameResource.ConstantBufferB->Map(0, &readRange, reinterpret_cast<void**>(&frameResource.pCbvDataBeginB)));
				m_constantBufferDataB.transform = m_transformB.getMatrix();
				memcpy(frameResource.pCbvDataBeginB, &m_constantBufferDataB, sizeof(m_constantBufferDataB));
			}
		}


		// Note: ComPtr's are CPU objects but this resource needs to stay in scope until
		// the command list that references it has finished executing on the GPU.
		// We will flush the GPU at the end of this method to ensure the resource is not
		// prematurely destroyed.
		ComPtr<ID3D12Resource> textureUploadHeap;

		auto& frameResource = m_frameResources[0];
		ThrowIfFailed(frameResource.CommandAllocator->Reset());
		ThrowIfFailed( frameResource.CommandList->Reset(frameResource.CommandAllocator.Get(), m_pipelineState.Get()));

		// Create the texture.
		{
			// Copy data to the intermediate upload heap and then schedule a copy
			// from the upload heap to the Texture2D.
			auto [width, height, channels, data] = GenerateTextureData();

			// Describe and create a Texture2D.
			D3D12_RESOURCE_DESC textureDesc = {};
			textureDesc.MipLevels = 1;
			textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			textureDesc.Width = width;
			textureDesc.Height = height;
			textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			textureDesc.DepthOrArraySize = 1;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

			CD3DX12_HEAP_PROPERTIES heapPropsDefault(D3D12_HEAP_TYPE_DEFAULT);
			ThrowIfFailed(m_device->CreateCommittedResource(
				&heapPropsDefault,
				D3D12_HEAP_FLAG_NONE,
				&textureDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_texture)));

			const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);

			// Create the GPU upload buffer.
			CD3DX12_HEAP_PROPERTIES heapPropsUpload(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
			ThrowIfFailed(m_device->CreateCommittedResource(
				&heapPropsUpload,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&textureUploadHeap)));

			D3D12_SUBRESOURCE_DATA textureData = {};
			textureData.pData = data.data();
			textureData.RowPitch = width * channels;
			textureData.SlicePitch = textureData.RowPitch * height;

			auto pixelShaderTransition = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			UpdateSubresources(frameResource.CommandList.Get(), m_texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
			frameResource.CommandList->ResourceBarrier(1, &pixelShaderTransition);

			// Describe and create a SRV for the texture.
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = textureDesc.Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;
			// SRV at slot 1 and 3
			for (int i = 0; i < FrameCount; i++)
			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), i * ResourceCount + 2, m_cbvSrvDescriptorSize);
				m_device->CreateShaderResourceView(m_texture.Get(), &srvDesc, srvHandle);
			}
		}

		// Close the command list and execute it to begin the initial GPU setup.
		ThrowIfFailed(frameResource.CommandList->Close());
		ID3D12CommandList* ppCommandLists[] = { frameResource.CommandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// Create synchronization objects and wait until assets have been uploaded to the GPU.
		{
			ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

			// Create an event handle to use for frame synchronization.
			m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (m_fenceEvent == nullptr)
			{
				ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
			}

			// Wait for the command list to execute; we are reusing the same command
			// list in our main loop but for now, we just want to wait for setup to
			// complete before continuing.

			m_fenceValue = 1;
			ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValue));
			frameResource.FenceValue = m_fenceValue;
			WaitForFence(m_fenceValue);
			m_fenceValue++;
		}
	}

	Texture D3D12Renderer::GenerateTextureData()
	{
		int width, height, channels;
		unsigned char* data = stbi_load("assets/textures/container.jpg", &width, &height, &channels, 4);
		return Texture{width, height, 4, {data, data + width * height * 4} };
	}


	void D3D12Renderer::BeginFrame()
	{
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		auto& frameResource = m_frameResources[m_frameIndex];
		WaitForFence(frameResource.FenceValue);

		const float translationSpeed = 0.005f;
		const float offsetBounds = 1.25f;

		m_transformA.position.x() += translationSpeed;
		if (m_transformA.position.x() > offsetBounds)
		{
			m_transformA.position.x() = -offsetBounds;
		}
		m_constantBufferDataA.transform = m_transformA.getMatrix();

		m_transformB.position.y() += translationSpeed;
		if (m_transformB.position.y() > offsetBounds)
		{
			m_transformB.position.y() = -offsetBounds;
		}
		m_constantBufferDataB.transform = m_transformB.getMatrix();

		memcpy(frameResource.pCbvDataBeginA, &m_constantBufferDataA, sizeof(m_constantBufferDataA));
		memcpy(frameResource.pCbvDataBeginB, &m_constantBufferDataB, sizeof(m_constantBufferDataB));

		// Record all the commands we need to render the scene into the command list.
		PopulateCommandList();
	}

	void D3D12Renderer::EndFrame()
	{
		auto& frameResource = m_frameResources[m_frameIndex];

		// Execute the command list.
		ID3D12CommandList* ppCommandLists[] = { frameResource.CommandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// Present the frame.
		ThrowIfFailed(m_swapChain->Present(1, 0));

		m_fenceValue++;
		ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValue));
		frameResource.FenceValue = m_fenceValue;
	}

	void D3D12Renderer::Resize(uint32_t width, uint32_t height)
	{
	}


	void D3D12Renderer::PopulateCommandList() const
	{
		auto& frameResource = m_frameResources[m_frameIndex];

		auto& commandAllocator = frameResource.CommandAllocator;
		auto& commandList = frameResource.CommandList;
		auto& renderTarget = frameResource.RenderTarget;


		ThrowIfFailed(commandAllocator->Reset());
		ThrowIfFailed(commandList->Reset(commandAllocator.Get(), m_pipelineState.Get()));

		// Set necessary state.
		commandList->SetGraphicsRootSignature(m_rootSignature.Get());

		ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandleA(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), m_frameIndex * ResourceCount, m_cbvSrvDescriptorSize);
		commandList->SetGraphicsRootDescriptorTable(0, cbvHandleA);
		CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle(cbvHandleA, 2, m_cbvSrvDescriptorSize);
		commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);
		commandList->RSSetViewports(1, &m_viewport);
		commandList->RSSetScissorRects(1, &m_scissorRect);

		// Indicate that the back buffer will be used as a render target.
		const auto rtBarrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &rtBarrier);

		const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<int>(m_frameIndex), m_rtvDescriptorSize);
		const CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

		// Record commands.
		constexpr float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		commandList->DrawInstanced(3, 1, 0, 0);

		CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandleB(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), m_frameIndex * ResourceCount + 1, m_cbvSrvDescriptorSize);
		commandList->SetGraphicsRootDescriptorTable(0, cbvHandleB);
		commandList->DrawInstanced(3, 1, 0, 0);

		// Indicate that the back buffer will now be used to present.
		const auto presentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		commandList->ResourceBarrier(1, &presentBarrier);

		ThrowIfFailed(commandList->Close());
	}

	void D3D12Renderer::WaitForFence(const std::uint64_t fenceValue) const
	{
		// Wait until the previous frame is finished.
		if (m_fence->GetCompletedValue() < fenceValue)
		{
			ThrowIfFailed(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent));
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}


	// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
	// If no such adapter can be found, *ppAdapter will be set to nullptr.
	_Use_decl_annotations_
	void D3D12Renderer::GetHardwareAdapter(
		IDXGIFactory1* pFactory,
		IDXGIAdapter1** ppAdapter,
		const bool requestHighPerformanceAdapter)
	{
		*ppAdapter = nullptr;

		ComPtr<IDXGIAdapter1> adapter;

		ComPtr<IDXGIFactory6> factory6;
		if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
		{
			for (
				UINT adapterIndex = 0;
				SUCCEEDED(factory6->EnumAdapterByGpuPreference(
					adapterIndex,
					requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
					IID_PPV_ARGS(&adapter)));
				++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
			}
		}

		if(adapter.Get() == nullptr)
		{
			for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
			}
		}

		*ppAdapter = adapter.Detach();
	}
}
