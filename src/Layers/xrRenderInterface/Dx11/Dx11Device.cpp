#include "stdafx.h"

#include "Dx11Device.h"
#include "Dx11Buffer.h"
#include "Dx11Texture.h"
#include "Dx11RenderTarget.h"

#define CHECK_RESOURCED_DIMENSION(pResource, Dimension) \
	{ \
		if (pResource) \
		{ \
			eResourceDimension dimension = RESOURCE_DIMENSION_UNKNOWN; \
			pResource->GetType(&dimension); \
			R_ASSERT(dimension == Dimension); \
		} \
	}

CRenderRHI_DX11 g_RenderRHI_DX11Implementation;

CRenderRHI_DX11::CRenderRHI_DX11()
{
}

CRenderRHI_DX11::~CRenderRHI_DX11()
{
}

void CRenderRHI_DX11::Create(void* renderDevice, void* renderContext)
{
	m_pHWRenderDevice = (ID3D11Device*)renderDevice;
	m_pHWRenderContext = (ID3D11DeviceContext*)renderContext;

	// Initialize pixel formats
	g_PixelFormats[FMT_UNKNOWN].PlatformFormat = DXGI_FORMAT_UNKNOWN;
	g_PixelFormats[FMT_R8G8B8A8].PlatformFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	g_PixelFormats[FMT_R8G8].PlatformFormat = DXGI_FORMAT_R8G8_UNORM;
	g_PixelFormats[FMT_R8G8B8].PlatformFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	g_PixelFormats[FMT_R5G6B5].PlatformFormat = DXGI_FORMAT_B5G6R5_UNORM;
	g_PixelFormats[FMT_B8G8R8A8].PlatformFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	g_PixelFormats[FMT_G16R16].PlatformFormat = DXGI_FORMAT_R16G16_UNORM;
	g_PixelFormats[FMT_A16B16G16R16].PlatformFormat = DXGI_FORMAT_R16G16B16A16_UNORM;
	g_PixelFormats[FMT_L8].PlatformFormat = DXGI_FORMAT_R8_UNORM;
	g_PixelFormats[FMT_V8U8].PlatformFormat = DXGI_FORMAT_R8G8_SNORM;
	g_PixelFormats[FMT_Q8W8V8U8].PlatformFormat = DXGI_FORMAT_R8G8B8A8_SNORM;
	g_PixelFormats[FMT_V16U16].PlatformFormat = DXGI_FORMAT_R16G16_SNORM;
	g_PixelFormats[FMT_D24X8].PlatformFormat = DXGI_FORMAT_R24G8_TYPELESS;
	g_PixelFormats[FMT_D24S8].PlatformFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	g_PixelFormats[FMT_D32F_LOCKABLE].PlatformFormat = DXGI_FORMAT_R32_TYPELESS;
	g_PixelFormats[FMT_G16R16F].PlatformFormat = DXGI_FORMAT_R16G16_FLOAT;
	g_PixelFormats[FMT_A16B16G16R16F].PlatformFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	g_PixelFormats[FMT_R32F].PlatformFormat = DXGI_FORMAT_R32_FLOAT;
	g_PixelFormats[FMT_R16F].PlatformFormat = DXGI_FORMAT_R16_FLOAT;
	g_PixelFormats[FMT_A32B32G32R32F].PlatformFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	g_PixelFormats[FMT_UYVY].PlatformFormat = DXGI_FORMAT_UNKNOWN;
	g_PixelFormats[FMT_R8G8_B8G8].PlatformFormat = DXGI_FORMAT_G8R8_G8B8_UNORM;
	g_PixelFormats[FMT_YUY2].PlatformFormat = DXGI_FORMAT_UNKNOWN;
	g_PixelFormats[FMT_G8R8_G8B8].PlatformFormat = DXGI_FORMAT_R8G8_B8G8_UNORM;
	g_PixelFormats[FMT_DXT1].PlatformFormat = DXGI_FORMAT_BC1_UNORM;
	g_PixelFormats[FMT_DXT2].PlatformFormat = DXGI_FORMAT_BC2_UNORM;
	g_PixelFormats[FMT_DXT3].PlatformFormat = DXGI_FORMAT_BC2_UNORM;
	g_PixelFormats[FMT_DXT4].PlatformFormat = DXGI_FORMAT_BC3_UNORM;
	g_PixelFormats[FMT_DXT5].PlatformFormat = DXGI_FORMAT_BC3_UNORM;
	g_PixelFormats[FMT_R32].PlatformFormat = DXGI_FORMAT_R32_UINT;
	g_PixelFormats[FMT_X8R8G8B8].PlatformFormat = DXGI_FORMAT_B8G8R8X8_UNORM;
}

ITexture1D* CRenderRHI_DX11::CreateTexture1D(const STexture1DDesc& textureDesc, const SubresourceData* pSubresourceDesc)
{
	CD3D11Texture1D* pTexture1D = new CD3D11Texture1D();
	R_CHK(pTexture1D->Create(textureDesc, pSubresourceDesc));
	return pTexture1D;
}

ITexture2D* CRenderRHI_DX11::CreateTexture2D(const STexture2DDesc& textureDesc, const SubresourceData* pSubresourceDesc)
{
	CD3D11Texture2D* pTexture2D = new CD3D11Texture2D();
	R_CHK(pTexture2D->Create(textureDesc, pSubresourceDesc));
	return pTexture2D;
}

ITexture3D* CRenderRHI_DX11::CreateTexture3D(const STexture3DDesc& textureDesc, const SubresourceData* pSubresourceDesc)
{
	CD3D11Texture3D* pTexture3D = new CD3D11Texture3D();
	R_CHK(pTexture3D->Create(textureDesc, pSubresourceDesc));
	return pTexture3D;
}

IRenderTargetView* CRenderRHI_DX11::CreateRenderTargetView(IRHIResource* pResource, const SRenderTargetViewDesc* pDesc)
{
	CD3D11RenderTargetView* pRenderTargetView = new CD3D11RenderTargetView();
	R_CHK(pRenderTargetView->Create(pResource, pDesc));
	return pRenderTargetView;
}

IDepthStencilView* CRenderRHI_DX11::CreateDepthStencilView(IRHIResource* pResource, const SDepthStencilViewDesc* pDesc)
{
	CD3D11DepthStencilView* pDepthStencilView = new CD3D11DepthStencilView();
	R_CHK(pDepthStencilView->Create(pResource, pDesc));
	return pDepthStencilView;
}

IBuffer* CRenderRHI_DX11::CreateBuffer(eBufferType bufferType, const void* pData, u32 DataSize, bool bImmutable)
{
	CD3D11Buffer* pBuffer = new CD3D11Buffer();
	R_CHK(pBuffer->Create(bufferType, pData, DataSize, bImmutable));
	return pBuffer;
}

void CRenderRHI_DX11::SetVertexBuffer(u32 StartSlot, IBuffer* pVertexBuffer, const u32 Stride, const u32 Offset)
{
	ID3D11DeviceContext* pImmediateContext = (ID3D11DeviceContext*)m_pHWRenderContext;
	R_ASSERT(pImmediateContext);

	CD3D11Buffer* pAPIBuffer = (CD3D11Buffer*)pVertexBuffer;
	if (pAPIBuffer)
	{
		ID3D11Buffer* pD3DBuffer = pAPIBuffer->GetD3DObject();

		const UINT uStrides = Stride;
		const UINT uOffsets = Offset;
		pImmediateContext->IASetVertexBuffers(StartSlot, 1, &pD3DBuffer, &uStrides, &uOffsets);
	}
	else
	{
		pImmediateContext->IASetVertexBuffers(StartSlot, 0, nullptr, 0, 0);
	}
}

void CRenderRHI_DX11::SetIndexBuffer(IBuffer* pIndexBuffer, bool Is32BitBuffer, u32 Offset)
{
	ID3D11DeviceContext* pImmediateContext = (ID3D11DeviceContext*)m_pHWRenderContext;
	R_ASSERT(pImmediateContext);

	CD3D11Buffer* pAPIBuffer = (CD3D11Buffer*)pIndexBuffer;
	if (pAPIBuffer)
	{
		ID3D11Buffer* pD3DBuffer = pAPIBuffer->GetD3DObject();
		DXGI_FORMAT indicesFormat = Is32BitBuffer ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		pImmediateContext->IASetIndexBuffer(pD3DBuffer, indicesFormat, Offset);
	}
	else
	{
		pImmediateContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	}
}

void CRenderRHI_DX11::VSSetConstantBuffers(u32 StartSlot, u32 NumBuffers, IBuffer* const* ppConstantBuffers)
{
	// #TODO: TO SLOW

	ID3D11DeviceContext* pImmediateContext = (ID3D11DeviceContext*)m_pHWRenderContext;
	R_ASSERT(pImmediateContext);

	xr_vector< ID3D11Buffer* > buffers;

	for (int i = 0; i < NumBuffers; i++)
	{
		CD3D11Buffer* pAPIBuffer = (CD3D11Buffer*)ppConstantBuffers[i];
		buffers.push_back(pAPIBuffer ? pAPIBuffer->GetD3DObject() : NULL);
	}

	pImmediateContext->VSSetConstantBuffers(StartSlot, NumBuffers, buffers.data());
}

void CRenderRHI_DX11::PSSetConstantBuffers(u32 StartSlot, u32 NumBuffers, IBuffer* const* ppConstantBuffers)
{
	// #TODO: TO SLOW

	ID3D11DeviceContext* pImmediateContext = (ID3D11DeviceContext*)m_pHWRenderContext;
	R_ASSERT(pImmediateContext);

	xr_vector< ID3D11Buffer* > buffers;

	for (int i = 0; i < NumBuffers; i++)
	{
		CD3D11Buffer* pAPIBuffer = (CD3D11Buffer*)ppConstantBuffers[i];
		buffers.push_back(pAPIBuffer ? pAPIBuffer->GetD3DObject() : NULL);
	}

	pImmediateContext->PSSetConstantBuffers(StartSlot, NumBuffers, buffers.data());
}

void CRenderRHI_DX11::HSSetConstantBuffers(u32 StartSlot, u32 NumBuffers, IBuffer* const* ppConstantBuffers)
{
	// #TODO: TO SLOW

	ID3D11DeviceContext* pImmediateContext = (ID3D11DeviceContext*)m_pHWRenderContext;
	R_ASSERT(pImmediateContext);

	xr_vector< ID3D11Buffer* > buffers;

	for (int i = 0; i < NumBuffers; i++)
	{
		CD3D11Buffer* pAPIBuffer = (CD3D11Buffer*)ppConstantBuffers[i];
		buffers.push_back(pAPIBuffer ? pAPIBuffer->GetD3DObject() : NULL);
	}

	pImmediateContext->HSSetConstantBuffers(StartSlot, NumBuffers, buffers.data());
}

void CRenderRHI_DX11::CSSetConstantBuffers(u32 StartSlot, u32 NumBuffers, IBuffer* const* ppConstantBuffers)
{	// #TODO: TO SLOW

	ID3D11DeviceContext* pImmediateContext = (ID3D11DeviceContext*)m_pHWRenderContext;
	R_ASSERT(pImmediateContext);

	xr_vector< ID3D11Buffer* > buffers;

	for (int i = 0; i < NumBuffers; i++)
	{
		CD3D11Buffer* pAPIBuffer = (CD3D11Buffer*)ppConstantBuffers[i];
		buffers.push_back(pAPIBuffer ? pAPIBuffer->GetD3DObject() : NULL);
	}

	pImmediateContext->CSSetConstantBuffers(StartSlot, NumBuffers, buffers.data());
}

void CRenderRHI_DX11::DSSetConstantBuffers(u32 StartSlot, u32 NumBuffers, IBuffer* const* ppConstantBuffers)
{	// #TODO: TO SLOW

	ID3D11DeviceContext* pImmediateContext = (ID3D11DeviceContext*)m_pHWRenderContext;
	R_ASSERT(pImmediateContext);

	xr_vector< ID3D11Buffer* > buffers;

	for (int i = 0; i < NumBuffers; i++)
	{
		CD3D11Buffer* pAPIBuffer = (CD3D11Buffer*)ppConstantBuffers[i];
		buffers.push_back(pAPIBuffer ? pAPIBuffer->GetD3DObject() : NULL);
	}

	pImmediateContext->DSSetConstantBuffers(StartSlot, NumBuffers, buffers.data());
}

void CRenderRHI_DX11::GSSetConstantBuffers(u32 StartSlot, u32 NumBuffers, IBuffer* const* ppConstantBuffers)
{
	// #TODO: TO SLOW

	ID3D11DeviceContext* pImmediateContext = (ID3D11DeviceContext*)m_pHWRenderContext;
	R_ASSERT(pImmediateContext);

	xr_vector< ID3D11Buffer* > buffers;

	for (int i = 0; i < NumBuffers; i++)
	{
		CD3D11Buffer* pAPIBuffer = (CD3D11Buffer*)ppConstantBuffers[i];
		buffers.push_back(pAPIBuffer ? pAPIBuffer->GetD3DObject() : NULL);
	}

	pImmediateContext->GSSetConstantBuffers(StartSlot, NumBuffers, buffers.data());
}

void CRenderRHI_DX11::ClearRenderTargetView(IRenderTargetView* pRenderTargetView, const float ColorRGBA[4])
{
	CD3D11RenderTargetView* pD3D11RenderTargetView = (CD3D11RenderTargetView*)pRenderTargetView;
	GetDeviceContext()->ClearRenderTargetView(pD3D11RenderTargetView->GetRenderTargetView(), ColorRGBA);
}

void CRenderRHI_DX11::ClearDepthStencilView(IDepthStencilView* pDepthStencilView, u32 ClearFlags, float Depth, u8 Stencil)
{
	CD3D11DepthStencilView* pD3D11DepthStencilView = (CD3D11DepthStencilView*)pDepthStencilView;
	GetDeviceContext()->ClearDepthStencilView(pD3D11DepthStencilView->GetDepthStencilView(), ClearFlags, Depth, Stencil);
}

// Note: maximum is 8 render targets.
void CRenderRHI_DX11::SetRenderTargets(u32 NumViews, IRenderTargetView* const* ppRenderTargetViews, IDepthStencilView* pDepthStencilView)
{
	// #TODO: RHI - Shit
	ID3D11RenderTargetView* renderTargetViews[8];
	//std::array<ID3D11RenderTargetView*, 8> renderTargetViews;
	for (int i = 0; i < NumViews; i++) {
		CD3D11RenderTargetView* pD3D11RenderTargetView = (CD3D11RenderTargetView*)ppRenderTargetViews[i];
		renderTargetViews[i] = pD3D11RenderTargetView ? pD3D11RenderTargetView->GetRenderTargetView() : nullptr;
	}

	CD3D11DepthStencilView* pD3D11DepthStencilView = (CD3D11DepthStencilView*)pDepthStencilView;

	// #TODO: RHI - Hack with depth stencil installation
	GetDeviceContext()->OMSetRenderTargets(
		NumViews,
		&renderTargetViews[0],
		pD3D11DepthStencilView ? pD3D11DepthStencilView->GetDepthStencilView() : nullptr);
}

void CRenderRHI_DX11::SetShader(IRenderShader* pDom)
{
	switch (pDom->Type)
	{
		case EShaderType::Compute:
		{
			ID3D11ComputeShader* D3DShader = (ID3D11ComputeShader*)pDom->Pointer;
			GetDeviceContext()->CSSetShader(D3DShader, 0, 0);
			break;
		}
		case EShaderType::Hull:
		{
			ID3D11HullShader* D3DShader = (ID3D11HullShader*)pDom->Pointer;
			GetDeviceContext()->HSSetShader(D3DShader, 0, 0);
			break;
		}
		case EShaderType::Domain:
		{
			ID3D11DomainShader* D3DShader = (ID3D11DomainShader*)pDom->Pointer;
			GetDeviceContext()->DSSetShader(D3DShader, 0, 0);
			break;
		}
		case EShaderType::Pixel:
		{
			ID3D11PixelShader* D3DShader = (ID3D11PixelShader*)pDom->Pointer;
			GetDeviceContext()->PSSetShader(D3DShader, 0, 0);
			break;
		}
		case EShaderType::Vertex:
		{
			ID3D11VertexShader* D3DShader = (ID3D11VertexShader*)pDom->Pointer;
			GetDeviceContext()->VSSetShader(D3DShader, 0, 0);
			break;
		}
		case EShaderType::Geometry:
		{
			ID3D11GeometryShader* D3DShader = (ID3D11GeometryShader*)pDom->Pointer;
			GetDeviceContext()->GSSetShader(D3DShader, 0, 0);
			break;
		}
	}
}

void CRenderRHI_DX11::CopyTexture1D(IRHIResource* pDstResource, IRHIResource* pSrcResource)
{
	CHECK_RESOURCED_DIMENSION(pDstResource, RESOURCE_DIMENSION_TEXTURE1D);
	CHECK_RESOURCED_DIMENSION(pSrcResource, RESOURCE_DIMENSION_TEXTURE1D);

	CD3D11Texture1D* pRHIDstResource = (CD3D11Texture1D*)pDstResource;
	CD3D11Texture1D* pRHISrcResource = (CD3D11Texture1D*)pSrcResource;

	GetDeviceContext()->CopyResource(
		pRHIDstResource ? (ID3D11Resource*)pRHIDstResource->GetD3D11Texture() : 0,
		pRHISrcResource ? (ID3D11Resource*)pRHISrcResource->GetD3D11Texture() : 0);
}

void CRenderRHI_DX11::CopyTexture2D(IRHIResource* pDstResource, IRHIResource* pSrcResource)
{
	CHECK_RESOURCED_DIMENSION(pDstResource, RESOURCE_DIMENSION_TEXTURE2D);
	CHECK_RESOURCED_DIMENSION(pSrcResource, RESOURCE_DIMENSION_TEXTURE2D);

	CD3D11Texture2D* pRHIDstResource = (CD3D11Texture2D*)pDstResource;
	CD3D11Texture2D* pRHISrcResource = (CD3D11Texture2D*)pSrcResource;

	GetDeviceContext()->CopyResource(
		pRHIDstResource ? (ID3D11Resource*)pRHIDstResource->GetD3D11Texture() : 0,
		pRHISrcResource ? (ID3D11Resource*)pRHISrcResource->GetD3D11Texture() : 0);
}

void CRenderRHI_DX11::CopyTexture3D(IRHIResource* pDstResource, IRHIResource* pSrcResource)
{
	CHECK_RESOURCED_DIMENSION(pDstResource, RESOURCE_DIMENSION_TEXTURE3D);
	CHECK_RESOURCED_DIMENSION(pSrcResource, RESOURCE_DIMENSION_TEXTURE3D);

	CD3D11Texture3D* pRHIDstResource = (CD3D11Texture3D*)pDstResource;
	CD3D11Texture3D* pRHISrcResource = (CD3D11Texture3D*)pSrcResource;

	GetDeviceContext()->CopyResource(
		pRHIDstResource ? (ID3D11Resource*)pRHIDstResource->GetD3D11Texture() : 0,
		pRHISrcResource ? (ID3D11Resource*)pRHISrcResource->GetD3D11Texture() : 0);
}

void CRenderRHI_DX11::Draw(u32 VertexCount, u32 StartVertexLocation)
{
	m_pHWRenderContext->Draw(VertexCount, StartVertexLocation);
}

void CRenderRHI_DX11::DrawIndexed(u32 IndexCount, u32 StartIndexLocation, s32 BaseVertexLocation)
{
	m_pHWRenderContext->DrawIndexed(IndexCount, StartIndexLocation, BaseVertexLocation);
}

void CRenderRHI_DX11::IASetPrimitiveTopology(u32 Topology)
{
	m_pHWRenderContext->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)Topology);
}

void CRenderRHI_DX11::IASetInputLayout(void* Layout)
{
	m_pHWRenderContext->IASetInputLayout((ID3D11InputLayout*)Layout);
}

ID3D11Device* CRenderRHI_DX11::GetDevice()
{
	return (ID3D11Device*)m_pHWRenderDevice;
}

ID3D11DeviceContext* CRenderRHI_DX11::GetDeviceContext()
{
	return (ID3D11DeviceContext*)m_pHWRenderContext;
}
