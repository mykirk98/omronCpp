#include "stdafx.h"
#include "CameraSideFFC.h"

using namespace StApi;
using namespace GenApi;
CCameraSideFFC::CCameraSideFFC(StApi::IStDevice *pIStDevice) : 
	m_pIStDevice(pIStDevice), 
	m_pIStImageAveragingFilter(CreateIStFilter(StFilterType_ImageAveraging)),
	m_pIStImageBuffer(CreateIStImageBuffer()),
	m_dblBlackLevelVaule(0),
	m_nFrameCount(50), 
	m_nRcvedFrameCount(0),
	m_dblRate(1),
	m_pINodeMap(pIStDevice->GetRemoteIStPort()->GetINodeMap()),
	m_pIIntegerFFCMeshWidth(m_pINodeMap->GetNode("FFCMeshWidth")),
	m_pIIntegerFFCMeshHeight(m_pINodeMap->GetNode("FFCMeshHeight")),
	m_pIBooleanFFCEnable(m_pINodeMap->GetNode("FFCEnable")),
	m_pIIntegerFFCIndex(m_pINodeMap->GetNode("FFCIndex")),
	m_pIIntegerFFCValue(m_pINodeMap->GetNode("FFCValue")),
	m_pIRegisterFFCValueAll(m_pINodeMap->GetNode("FFCValueAll")),
	m_pIEnumerationDeviceRegistersEndianness(m_pINodeMap->GetNode("DeviceRegistersEndianness")),
	m_pIEnumerationFFCMode(m_pINodeMap->GetNode("FFCMode")),
	m_pIEnumerationFFCSensorSelector(m_pINodeMap->GetNode("FFCSensorSelector"))
{
	CIntegerPtr pIIntegerWidthMax(m_pINodeMap->GetNode("WidthMax"));
	m_nImageWidthMax = (size_t)pIIntegerWidthMax->GetValue();

	CIntegerPtr pIIntegerHeightMax(m_pINodeMap->GetNode("HeightMax"));
	m_nImageHeightMax = (size_t)pIIntegerHeightMax->GetValue();


	const size_t nMeshWidth = (size_t)m_pIIntegerFFCMeshWidth->GetValue();
	const size_t nMeshHeight = (size_t)m_pIIntegerFFCMeshHeight->GetValue();

	m_nAreaCountX = (size_t)((m_nImageWidthMax + nMeshWidth - 1) / nMeshWidth);
	m_nAreaCountY = (size_t)((m_nImageHeightMax + nMeshHeight - 1) / nMeshHeight);
	m_nAreaCount = m_nAreaCountX * m_nAreaCountY;

	m_pIBooleanFFCEnable->SetValue(false);

	if (IsReadable(m_pIIntegerFFCValue))
	{
		m_nMaxGainValue = m_pIIntegerFFCValue->GetMax();
	}
	else
	{
		
		GenApi::CIntegerPtr pIIntegerFFCValueMax(m_pINodeMap->GetNode("FFCValueMaxReg"));
		if (IsReadable(pIIntegerFFCValueMax))
		{
			m_nMaxGainValue = pIIntegerFFCValueMax->GetValue();
		}
		else
		{
			const int64_t nEachSize = m_pIRegisterFFCValueAll->GetLength() / (m_nAreaCountX + 1) / (m_nAreaCountY + 1);
			if (nEachSize < 2)
			{
				m_nMaxGainValue = 255;
			}
			else
			{
				m_nMaxGainValue = 65535;
			}
		}
	}

	CFloatPtr pIFloat_BlackLevel(m_pINodeMap->GetNode("BlackLevel"));
	if (IsReadable(pIFloat_BlackLevel))
	{
		m_dblBlackLevelVaule = pIFloat_BlackLevel->GetValue();
	}
}

CCameraSideFFC::~CCameraSideFFC()
{
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CCameraSideFFC::IsSupported(CNodeMapPtr pINodeMap)
{
	try
	{
		CEnumerationPtr pIEnumeration_FFCType(pINodeMap->GetNode("FFCType"));
		CEnumerationPtr pIEnumeration_FFCSelector(pINodeMap->GetNode("FFCSelector"));

		if(IsImplemented(pIEnumeration_FFCType) && IsImplemented(pIEnumeration_FFCSelector))
		{
			return(
				(pIEnumeration_FFCType->GetCurrentEntry()->GetSymbolic().compare("Mesh") == 0) &&
				(pIEnumeration_FFCSelector->GetCurrentEntry()->GetSymbolic().compare("Gain") == 0)
				);
		}
	}
	catch (...)
	{
	}
	return(false);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CCameraSideFFC::OnIStImage(StApi::IStImage *pIStImage)
{
	try
	{
		if (m_nRcvedFrameCount < m_nFrameCount)
		{
			m_pIStImageAveragingFilter->Filter(pIStImage);

			if (++m_nRcvedFrameCount == m_nFrameCount)
			{
				const EStPixelFormatNamingConvention_t ePFNC = pIStImage->GetImagePixelFormat();
				const StApi::IStPixelFormatInfo *pIStPixelFormatInfo = GetIStPixelFormatInfo(ePFNC);
				const uint32_t nTargetBitCount = 16;
				m_dblRate = pow(2.0, (int)(nTargetBitCount - pIStPixelFormatInfo->GetEachComponentValidBitCount()));
				m_pIStImageAveragingFilter->GetAveragedImage(m_pIStImageBuffer, nTargetBitCount);
				m_objRcvImageDoneEvent.SetEvent();
			}
		}
	}
	catch (...)
	{
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CCameraSideFFC::Wait(uint32_t waitms)
{
	if (WaitForSingleObject(m_objRcvImageDoneEvent, waitms) != WAIT_OBJECT_0)
	{
		throw RUNTIME_EXCEPTION("Timeout");
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CCameraSideFFC::CalculateAndSend()
{
	const IStImage *pIStImage =  m_pIStImageBuffer->GetIStImage();
	const EStPixelFormatNamingConvention_t ePFNC = pIStImage->GetImagePixelFormat();
	const StApi::IStPixelFormatInfo *pIStPixelFormatInfo = GetIStPixelFormatInfo(ePFNC);

	if (pIStPixelFormatInfo->GetEachComponentValidBitCount() <= 8)
	{
		if (m_nMaxGainValue < 256)
		{
			mCalculateAndSend<uint8_t, uint8_t>(pIStPixelFormatInfo);
		}
		else
		{
			mCalculateAndSend<uint8_t, uint16_t>(pIStPixelFormatInfo);
		}
	}
	else
	{
		if (m_nMaxGainValue < 256)
		{
			mCalculateAndSend<uint16_t, uint8_t>(pIStPixelFormatInfo);
		}
		else
		{
			mCalculateAndSend<uint16_t, uint16_t>(pIStPixelFormatInfo);
		}
	}
}



