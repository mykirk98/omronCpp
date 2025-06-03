#include "stdafx.h"
#include "DefectivePixelManager.h"

using namespace StApi;
using namespace GenApi;


bool CDefectivePixelManager::IsSupported(GenApi::INodeMap *pINodemap)
{
	CBooleanPtr pIBoolean_PixelCorrectionAllEnabled(pINodemap->GetNode("PixelCorrectionAllEnabled"));
	CIntegerPtr pIInteger_PixelCorrectionIndex(pINodemap->GetNode("PixelCorrectionIndex"));
	CBooleanPtr pIBoolean_PixelCorrectionEnabled(pINodemap->GetNode("PixelCorrectionEnabled"));
	CIntegerPtr pIInteger_PixelCorrectionX(pINodemap->GetNode("PixelCorrectionX"));
	CIntegerPtr pIInteger_PixelCorrectionY(pINodemap->GetNode("PixelCorrectionY"));

	return(
		IsImplemented(pIBoolean_PixelCorrectionAllEnabled) && 
		IsImplemented(pIInteger_PixelCorrectionIndex) &&
		IsImplemented(pIBoolean_PixelCorrectionEnabled) &&
		IsImplemented(pIInteger_PixelCorrectionX) &&
		IsImplemented(pIInteger_PixelCorrectionY)
		);

}

CDefectivePixelManager::CDefectivePixelManager(GenApi::INodeMap *pINodeMap, StApi::IStImageDisplayWnd *pIStImageDisplayWnd) :
	m_pINodeMap_RemoteDevice(pINodeMap),
	m_pIStImageDisplayWnd(pIStImageDisplayWnd),
	m_pIBoolean_PixelCorrectionAllEnabled(m_pINodeMap_RemoteDevice->GetNode("PixelCorrectionAllEnabled")),
	m_pIEnumeration_PixelCorrectionSelector(m_pINodeMap_RemoteDevice->GetNode("PixelCorrectionSelector")),
	m_pIInteger_PixelCorrectionIndex(m_pINodeMap_RemoteDevice->GetNode("PixelCorrectionIndex")),
	m_pIBoolean_PixelCorrectionEnabled(m_pINodeMap_RemoteDevice->GetNode("PixelCorrectionEnabled")),
	m_pIInteger_PixelCorrectionX(m_pINodeMap_RemoteDevice->GetNode("PixelCorrectionX")),
	m_pIInteger_PixelCorrectionY(m_pINodeMap_RemoteDevice->GetNode("PixelCorrectionY")),
	m_pICommand_DeviceRegistersStreamingStart(m_pINodeMap_RemoteDevice->GetNode("DeviceRegistersStreamingStart")),
	m_pICommand_DeviceRegistersStreamingEnd(m_pINodeMap_RemoteDevice->GetNode("DeviceRegistersStreamingEnd")),
	m_pIStDefectivePixelDetectionFilter(StApi::CreateIStFilter(StApi::StFilterType_DefectivePixelDetection)),
	m_pINodeMap_DefectivePixelDetectionFilter(m_pIStDefectivePixelDetectionFilter->GetINodeMap()),
	m_pIEnumeration_ExpectedPixelFormat(m_pINodeMap_DefectivePixelDetectionFilter->GetNode("ExpectedPixelFormat")),
	m_pIInteger_MaximumPixelCountToDetect(m_pINodeMap_DefectivePixelDetectionFilter->GetNode("MaximumPixelCountToDetect"))
{
	try
	{
		if (IsReadable(m_pIInteger_PixelCorrectionIndex))
		{
			m_pIInteger_MaximumPixelCountToDetect->SetValue(m_pIInteger_PixelCorrectionIndex->GetMax() - m_pIInteger_PixelCorrectionIndex->GetMin() + 1);
		}

		if (m_pIEnumeration_PixelCorrectionSelector.IsValid())
		{
			NodeList_t nodeList;
			m_pIEnumeration_PixelCorrectionSelector->GetEntries(nodeList);
			for (size_t i = 0; i < nodeList.size(); ++i)
			{
				if (IsAvailable(nodeList[i]))
				{
					CEnumEntryPtr pEnumEntry(nodeList[i]);
					m_vecPixelCorrectionSelectorAvailableValueList.push_back(pEnumEntry->GetValue());
					m_vecPixelColorName.push_back(pEnumEntry->GetSymbolic());
				}
			}
		}
		else
		{
			m_vecPixelColorName.push_back("Raw");
		}
	}
	catch (...)
	{
	}
}


CDefectivePixelManager::~CDefectivePixelManager()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelManager::OnStCallbackForOverlay(IStCallbackParamBase *pIStCallbackParamBase, void* /*pvContext*/)
{
	if (pIStCallbackParamBase->GetCallbackType() != StCallbackType_StApiGUIEvent_DisplayImageWndDrawing)
	{
		return;
	}


	StApi::IStCallbackParamStApiGUIEventDrawing *pIStCallbackParamStApiGUIEventDrawing = dynamic_cast<StApi::IStCallbackParamStApiGUIEventDrawing*>(pIStCallbackParamBase);
	HDC hDC = pIStCallbackParamStApiGUIEventDrawing->GetDC();

	const size_t nROIOffsetX = pIStCallbackParamStApiGUIEventDrawing->GetROIOffsetX();
	const size_t nROIOffsetY = pIStCallbackParamStApiGUIEventDrawing->GetROIOffsetY();
	const size_t nROIWidth = pIStCallbackParamStApiGUIEventDrawing->GetROIWidth();
	const size_t nROIHeight = pIStCallbackParamStApiGUIEventDrawing->GetROIHeight();
	const size_t nDisplayWidth = pIStCallbackParamStApiGUIEventDrawing->GetDisplayWidth();
	const size_t nDisplayHeight = pIStCallbackParamStApiGUIEventDrawing->GetDisplayHeight();

	const double dblMagnificationH = nDisplayWidth / (double)nROIWidth;
	const double dblMagnificationV = nDisplayHeight / (double)nROIHeight;


	const int32_t nSize = 4;
	GenApi::AutoLock objAuto(m_objLock);

	for (size_t nColor = 0; nColor < m_vecmapDetectedDefectivePixelIndex.size(); ++nColor)
	{
		std::map<std::pair<uint16_t, uint16_t>, size_t> *pmapPixelIndex[] = { &m_vecmapDetectedDefectivePixelIndex[nColor],  &m_vecmapRegisteredDefectivePixelIndex[nColor] };
		const COLORREF pColor[] = { RGB(255, 0, 0) , RGB(0, 255, 0) };

		for (size_t i = 0; i < _countof(pmapPixelIndex); ++i)
		{
			HPEN hPen = reinterpret_cast<HPEN>(CreatePen(PS_SOLID, 1, pColor[i]));
			HPEN hOldPen = reinterpret_cast<HPEN>(SelectObject(hDC, hPen));
			HBRUSH hBrush = reinterpret_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
			HBRUSH hOldBrush = reinterpret_cast<HBRUSH>(SelectObject(hDC, hBrush));
			for (std::map<std::pair<uint16_t, uint16_t>, size_t>::const_iterator itr = pmapPixelIndex[i]->begin(); itr != pmapPixelIndex[i]->end(); ++itr)
			{
				const size_t x = itr->first.first;
				const size_t y = itr->first.second;

				const int32_t nLeft = (int32_t)x - nSize;
				const int32_t nRight = (int32_t)x + nSize;
				const int32_t nTop = (int32_t)y - nSize;
				const int32_t nBottom = (int32_t)y + nSize;

				Ellipse(hDC,
					static_cast<LONG>((nLeft - nROIOffsetX) * dblMagnificationH), static_cast<LONG>((nTop - nROIOffsetY) * dblMagnificationV),
					static_cast<LONG>((nRight - nROIOffsetX) * dblMagnificationH), static_cast<LONG>((nBottom - nROIOffsetY) * dblMagnificationV));
			}
			SelectObject(hDC, hOldBrush);
			SelectObject(hDC, hOldPen);
			DeleteObject(hPen);
		}
	}

	{
		HPEN hPen = reinterpret_cast<HPEN>(CreatePen(PS_SOLID, 1, RGB(255, 0, 255)));
		HPEN hOldPen = reinterpret_cast<HPEN>(SelectObject(hDC, hPen));
		HBRUSH hBrush = reinterpret_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
		HBRUSH hOldBrush = reinterpret_cast<HBRUSH>(SelectObject(hDC, hBrush));
		for (std::vector<EachDefectPixelPosition_t>::const_iterator itr = m_vecSelectedPixelInformation.begin(); itr != m_vecSelectedPixelInformation.end(); ++itr)
		{
#ifdef ENABLED_TUPLE
			const size_t x = std::get<1>(*itr);
			const size_t y = std::get<2>(*itr);
#else
			const size_t x = itr->second.first;
			const size_t y = itr->second.second;
#endif //ENABLED_TUPLE

			const int32_t nLeft = (int32_t)x - nSize;
			const int32_t nRight = (int32_t)x + nSize;
			const int32_t nTop = (int32_t)y - nSize;
			const int32_t nBottom = (int32_t)y + nSize;

			Ellipse(hDC,
				static_cast<LONG>((nLeft - nROIOffsetX) * dblMagnificationH), static_cast<LONG>((nTop - nROIOffsetY) * dblMagnificationV),
				static_cast<LONG>((nRight - nROIOffsetX) * dblMagnificationH), static_cast<LONG>((nBottom - nROIOffsetY) * dblMagnificationV));
		}
		SelectObject(hDC, hOldBrush);
		SelectObject(hDC, hOldPen);
		DeleteObject(hPen);

	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelManager::DetectDefectivePixel(StApi::IStImage *pIStImage)
{
	bool isRGB = false;
	switch (pIStImage->GetImagePixelFormat())
	{
	case(StPFNC_RGB8):
	case(StPFNC_RGBa8):
	case(StPFNC_BGR8):
	case(StPFNC_BGRa8):
	case(StPFNC_RGB10):
	case(StPFNC_RGBa10):
	case(StPFNC_BGR10):
	case(StPFNC_BGRa10):
	case(StPFNC_RGB12):
	case(StPFNC_RGBa12):
	case(StPFNC_BGR12):
	case(StPFNC_BGRa12):
	case(StPFNC_RGB14):
	case(StPFNC_RGBa14):
	case(StPFNC_BGR14):
	case(StPFNC_BGRa14):
	case(StPFNC_RGB16):
	case(StPFNC_RGBa16):
	case(StPFNC_BGR16):
	case(StPFNC_BGRa16):
		isRGB = true;
		break;
	}
	const size_t nColorCount = isRGB ? 3 : 1;
	m_vecvecDefectivePixelInfomation.resize(nColorCount);
	m_vecmapDetectedDefectivePixelIndex.resize(nColorCount);

	StApi::CIStImageBufferPtr pIStImageBuffer(isRGB ? CreateIStImageBuffer() : NULL);
	for (size_t nColor = 0; nColor < nColorCount; ++nColor)
	{
		if (isRGB)
		{
			const EStPixelComponent_t pePC[] = { StPixelComponent_R , StPixelComponent_G, StPixelComponent_B};
			pIStImage->ExportComponentData(pePC[nColor], pIStImageBuffer);
		}

		StApi::IStImage *pIStImageCurColor = isRGB ? pIStImageBuffer->GetIStImage() : pIStImage;

		m_pIEnumeration_ExpectedPixelFormat->SetIntValue(pIStImageCurColor->GetImagePixelFormat());
		m_pIStDefectivePixelDetectionFilter->Filter(pIStImageCurColor);

		GenApi::AutoLock objAuto(m_objLock);

		EStDefectivePixelDetectionStatus_t eStatus;
		size_t nCount;
		m_pIStDefectivePixelDetectionFilter->GetDetectionResult(&eStatus, &nCount, NULL);

		m_vecmapDetectedDefectivePixelIndex[nColor].clear();

		if (
			(eStatus == StDefectivePixelDetectionStatus_Succeeded) ||
			(eStatus == StDefectivePixelDetectionStatus_TooManyDefectivePixelDetectedFailed)
			)
		{
			if (0 < nCount)
			{
				m_vecvecDefectivePixelInfomation[nColor].resize(nCount);
				m_pIStDefectivePixelDetectionFilter->GetDetectionResult(&eStatus, &nCount, &m_vecvecDefectivePixelInfomation[nColor][0]);

				for (size_t i = 0; i < nCount; ++i)
				{
					PSStDefectivePixelInformation_t pDPI = &m_vecvecDefectivePixelInfomation[nColor][i];
					std::pair<size_t, size_t> pairPos = std::make_pair(pDPI->x, pDPI->y);
					m_vecmapDetectedDefectivePixelIndex[nColor].insert(std::make_pair(pairPos, i));
				}
			}
		}
		m_pIStDefectivePixelDetectionFilter->ClearDetectionResult();
	}
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelManager::GetRegisteredDefectivePixelList()
{
	const size_t nColorCount = m_vecPixelColorName.size();
	int64_t nInitialSelectorValue = 0;
	if (m_pIEnumeration_PixelCorrectionSelector.IsValid())
	{
		nInitialSelectorValue = m_pIEnumeration_PixelCorrectionSelector->GetIntValue();
	}

	GenApi::AutoLock objAuto(m_objLock);
	m_vecmapRegisteredDefectivePixelIndex.resize(nColorCount);
	m_vecvecRegisteredDefectivePixel.resize(nColorCount);

	const bool bEnabledAll = m_pIBoolean_PixelCorrectionAllEnabled->GetValue();
	m_pIBoolean_PixelCorrectionAllEnabled->SetValue(true);

	const int64_t nInitialIndex = m_pIInteger_PixelCorrectionIndex->GetValue();
	for (size_t nColor = 0; nColor < nColorCount; ++nColor)
	{
		if (m_pIEnumeration_PixelCorrectionSelector.IsValid())
		{
			m_pIEnumeration_PixelCorrectionSelector->SetIntValue(m_vecPixelCorrectionSelectorAvailableValueList[nColor]);
		}
		m_vecmapRegisteredDefectivePixelIndex[nColor].clear();
		m_vecvecRegisteredDefectivePixel[nColor].clear();
		const size_t nCount = (size_t)(m_pIInteger_PixelCorrectionIndex->GetMax() + 1);
		m_vecvecRegisteredDefectivePixel[nColor].resize(nCount);
		for (size_t i = 0; i < nCount; ++i)
		{
			PSRegisteredDefectivePixel_t pRDP = &m_vecvecRegisteredDefectivePixel[nColor][i];
			m_pIBoolean_PixelCorrectionEnabled->GetValue();
			m_pIInteger_PixelCorrectionIndex->SetValue(i);
			pRDP->isEnable = m_pIBoolean_PixelCorrectionEnabled->GetValue();
			if (pRDP->isEnable)
			{
				pRDP->x = (size_t)(m_pIInteger_PixelCorrectionX->GetValue());
				pRDP->y = (size_t)(m_pIInteger_PixelCorrectionY->GetValue());
				m_vecmapRegisteredDefectivePixelIndex[nColor].insert(std::make_pair(std::make_pair(pRDP->x, pRDP->y), i));
			}
		}
	}

	if (m_pIEnumeration_PixelCorrectionSelector.IsValid())
	{
		m_pIEnumeration_PixelCorrectionSelector->SetIntValue(nInitialSelectorValue);
	}
	m_pIInteger_PixelCorrectionIndex->SetValue(nInitialIndex);
	if (!bEnabledAll)
	{
		m_pIBoolean_PixelCorrectionAllEnabled->SetValue(false);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelManager::ClearDetectedPixelList()
{
	GenApi::AutoLock objAuto(m_objLock);
	m_pIStDefectivePixelDetectionFilter->ClearDetectionResult();
	for (size_t i = 0; i < m_vecvecDefectivePixelInfomation.size(); ++i)
	{
		m_vecvecDefectivePixelInfomation[i].clear();
	}
	for (size_t i = 0; i < m_vecmapDetectedDefectivePixelIndex.size(); ++i)
	{
		m_vecmapDetectedDefectivePixelIndex[i].clear();
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
size_t CDefectivePixelManager::GetDetectedDefectivePixelCount()
{

	GenApi::AutoLock objAuto(m_objLock);
	size_t nCount = 0;
	for (size_t i = 0; i < m_vecvecDefectivePixelInfomation.size(); ++i)
	{
		nCount += m_vecvecDefectivePixelInfomation[i].size();
	}
	return(nCount);
}


//-----------------------------------------------------------------------------
//Comparison method used to sort the defective pixel list display
//-----------------------------------------------------------------------------
int CDefectivePixelManager::Compare(int iColumn, EachDefectPixelPosition_t *pPosition)
{
	size_t nColor0 = std::get<0>(pPosition[0]);
	size_t nColor1 = std::get<0>(pPosition[1]);
	
#ifdef ENABLED_TUPLE
	std::pair<uint16_t, uint16_t> pairPos0 = std::make_pair(std::get<1>(pPosition[0]), std::get<2>(pPosition[0]));
	std::pair<uint16_t, uint16_t> pairPos1 = std::make_pair(std::get<1>(pPosition[1]), std::get<2>(pPosition[1]));
#else
	std::pair<uint16_t, uint16_t> pairPos0 = pPosition[0].second;
	std::pair<uint16_t, uint16_t> pairPos1 = pPosition[1].second;
#endif //ENABLED_TUPLE

	if (iColumn == LIST_COL_STATUS)
	{
		std::map<std::pair<uint16_t, uint16_t>, size_t>::iterator itr0 = m_vecmapRegisteredDefectivePixelIndex[nColor0].find(pairPos0);
		std::map<std::pair<uint16_t, uint16_t>, size_t>::iterator itr1 = m_vecmapRegisteredDefectivePixelIndex[nColor1].find(pairPos1);
		if ((itr0 == m_vecmapRegisteredDefectivePixelIndex[nColor0].end()))
		{
			if ((itr1 == m_vecmapRegisteredDefectivePixelIndex[nColor1].end()))
			{
				return(0);
			}
			else
			{
				return(-1);
			}
		}
		else
		{
			if ((itr1 == m_vecmapRegisteredDefectivePixelIndex[nColor1].end()))
			{
				return(1);
			}
			else
			{
				return((itr0->second < itr1->second) ? 1 : -1);
			}
		}
	}
	else if (iColumn == LIST_COL_COLOR)
	{
		if (nColor0 == nColor1)
		{
			return(0);
		}
		else
		{
			return((nColor0 < nColor1) ? 1 : -1);
		}
	}
	else if (iColumn == LIST_COL_X)
	{
		if (pairPos0.first == pairPos1.first)
		{

			if (pairPos0.second == pairPos1.second)
			{
				return(0);
			}
			else
			{
				return((pairPos0.second < pairPos1.second) ? 1 : -1);
			}
		}
		else
		{
			return((pairPos0.first < pairPos1.first) ? 1 : -1);
		}
	}
	else if (iColumn == LIST_COL_Y)
	{
		if (pairPos0.second == pairPos1.second)
		{

			if (pairPos0.first == pairPos1.first)
			{
				return(0);
			}
			else
			{
				return((pairPos0.first < pairPos1.first) ? 1 : -1);
			}
		}
		else
		{
			return((pairPos0.second < pairPos1.second) ? 1 : -1);
		}
	}
	else
	{
		std::map<std::pair<uint16_t, uint16_t>, size_t>::iterator itr0 = m_vecmapDetectedDefectivePixelIndex[nColor0].find(pairPos0);
		std::map<std::pair<uint16_t, uint16_t>, size_t>::iterator itr1 = m_vecmapDetectedDefectivePixelIndex[nColor1].find(pairPos1);
		if ((itr0 == m_vecmapDetectedDefectivePixelIndex[nColor0].end()))
		{
			if ((itr1 == m_vecmapDetectedDefectivePixelIndex[nColor1].end()))
			{
				return(0);
			}
			else
			{
				return(1);
			}
		}
		else
		{
			if ((itr1 == m_vecmapDetectedDefectivePixelIndex[nColor1].end()))
			{
				return(-1);
			}
			else
			{
				PSStDefectivePixelInformation_t pDPI0 = &m_vecvecDefectivePixelInfomation[nColor0][itr0->second];
				PSStDefectivePixelInformation_t pDPI1 = &m_vecvecDefectivePixelInfomation[nColor1][itr1->second];

				switch (iColumn)
				{
				case(LIST_COL_EVALUATION):
					if (pDPI0->dblEvaluationValue == pDPI1->dblEvaluationValue)
					{
						return(0);
					}
					else
					{
						return((pDPI0->dblEvaluationValue < pDPI1->dblEvaluationValue) ? 1 : -1);
					}
					break;
				case(LIST_COL_REFERENCE):
					if (pDPI0->dblReferenceValue == pDPI1->dblReferenceValue)
					{
						return(0);
					}
					else
					{
						return((pDPI0->dblReferenceValue < pDPI1->dblReferenceValue) ? 1 : -1);
					}
					break;
				case(LIST_COL_DIFFERENCE):
					if (pDPI0->dblDeltaRatio == pDPI1->dblDeltaRatio)
					{
						return(0);
					}
					else
					{
						return((pDPI0->dblDeltaRatio < pDPI1->dblDeltaRatio) ? 1 : -1);
					}
					break;
				}
			}
		}


	}
	return(0);
}


//-----------------------------------------------------------------------------
//Method for registering defective pixels to the camera
//-----------------------------------------------------------------------------
bool CDefectivePixelManager::RegisterSelectedPixel(std::vector<EachDefectPixelPosition_t> &vecPxelList)
{
	// Acquire the registered defective pixel list.
	GetRegisteredDefectivePixelList();

	GenApi::AutoLock objAuto(m_objLock);


	const bool bEnabledAll = m_pIBoolean_PixelCorrectionAllEnabled->GetValue();
	m_pIBoolean_PixelCorrectionAllEnabled->SetValue(true);
	const int64_t nInitialIndex = m_pIInteger_PixelCorrectionIndex->GetValue();

	int64_t nInitialSelectorValue = 0;
	if (m_pIEnumeration_PixelCorrectionSelector.IsValid())
	{
		nInitialSelectorValue = m_pIEnumeration_PixelCorrectionSelector->GetIntValue();
	}
	bool bOverflow = false;
	std::vector<size_t> vecRegIndex;
	vecRegIndex.resize(m_vecvecRegisteredDefectivePixel.size());
	memset(&vecRegIndex[0], 0, sizeof(vecRegIndex[0]) * vecRegIndex.size());

	if (m_pICommand_DeviceRegistersStreamingStart.IsValid())
	{
		m_pICommand_DeviceRegistersStreamingStart->Execute();
	}
	for (std::vector<EachDefectPixelPosition_t>::const_iterator itr = vecPxelList.begin(); itr != vecPxelList.end(); ++itr)
	{
		//
		const uint8_t nColor = std::get<0>(*itr);
#ifdef ENABLED_TUPLE
		std::pair<uint16_t, uint16_t> sPos = std::make_pair(std::get<1>(*itr), std::get<2>(*itr));
#else
		std::pair<uint16_t, uint16_t> sPos = itr->second;
#endif //ENABLED_TUPLE
		if (m_vecmapRegisteredDefectivePixelIndex[nColor].find(sPos) == m_vecmapRegisteredDefectivePixelIndex[nColor].end())
		{
			if (m_pIEnumeration_PixelCorrectionSelector.IsValid())
			{
				m_pIEnumeration_PixelCorrectionSelector->SetIntValue(m_vecPixelCorrectionSelectorAvailableValueList[nColor]);
			}

			for (; vecRegIndex[nColor] < m_vecvecRegisteredDefectivePixel[nColor].size(); ++vecRegIndex[nColor])
			{
				if (!m_vecvecRegisteredDefectivePixel[nColor][vecRegIndex[nColor]].isEnable)
				{
					m_pIInteger_PixelCorrectionIndex->SetValue(vecRegIndex[nColor]);
					m_pIBoolean_PixelCorrectionEnabled->SetValue(true);
					m_pIInteger_PixelCorrectionX->SetValue(sPos.first);
					m_pIInteger_PixelCorrectionY->SetValue(sPos.second);

					m_vecvecRegisteredDefectivePixel[nColor][vecRegIndex[nColor]].isEnable = true;
					m_vecvecRegisteredDefectivePixel[nColor][vecRegIndex[nColor]].x = sPos.first;
					m_vecvecRegisteredDefectivePixel[nColor][vecRegIndex[nColor]].y = sPos.second;
					m_vecmapRegisteredDefectivePixelIndex[nColor].insert(std::make_pair(sPos, vecRegIndex[nColor]));
					break;
				}
			}
			if (m_vecvecRegisteredDefectivePixel[nColor].size() <= vecRegIndex[nColor])
			{
				bOverflow = true;
				break;
			}

		}
	}
	if (m_pICommand_DeviceRegistersStreamingEnd.IsValid())
	{
		m_pICommand_DeviceRegistersStreamingEnd->Execute();
	}

	if (m_pIEnumeration_PixelCorrectionSelector.IsValid())
	{
		m_pIEnumeration_PixelCorrectionSelector->SetIntValue(nInitialSelectorValue);
	}
	m_pIInteger_PixelCorrectionIndex->SetValue(nInitialIndex);
	if (!bEnabledAll)
	{
		m_pIBoolean_PixelCorrectionAllEnabled->SetValue(false);
	}
	return(bOverflow);
}
//-----------------------------------------------------------------------------
//Method to Unregister Defective Pixels in Camera
//-----------------------------------------------------------------------------
void CDefectivePixelManager::DeregisterSelectedPixel(std::vector<EachDefectPixelPosition_t> &vecPxelList)
{
	// Acquire the registered defective pixel list.
	GetRegisteredDefectivePixelList();

	const bool bEnabledAll = m_pIBoolean_PixelCorrectionAllEnabled->GetValue();
	m_pIBoolean_PixelCorrectionAllEnabled->SetValue(true);
	const int64_t nInitialIndex = m_pIInteger_PixelCorrectionIndex->GetValue();

	int64_t nInitialSelectorValue = 0;
	if (m_pIEnumeration_PixelCorrectionSelector.IsValid())
	{
		nInitialSelectorValue = m_pIEnumeration_PixelCorrectionSelector->GetIntValue();
	}
	if (m_pICommand_DeviceRegistersStreamingStart.IsValid())
	{
		m_pICommand_DeviceRegistersStreamingStart->Execute();
	}
	for (std::vector<EachDefectPixelPosition_t>::const_iterator itr = vecPxelList.begin(); itr != vecPxelList.end(); ++itr)
	{
		//
		const uint8_t nColor = std::get<0>(*itr);
		if (m_pIEnumeration_PixelCorrectionSelector.IsValid())
		{
			m_pIEnumeration_PixelCorrectionSelector->SetIntValue(m_vecPixelCorrectionSelectorAvailableValueList[nColor]);
		}
#ifdef ENABLED_TUPLE
		std::pair<uint16_t, uint16_t> sPos = std::make_pair(std::get<1>(*itr), std::get<2>(*itr));
#else
		std::pair<uint16_t, uint16_t> sPos = itr->second;
#endif //ENABLED_TUPLE
		std::map<std::pair<uint16_t, uint16_t>, size_t>::iterator itrReg = m_vecmapRegisteredDefectivePixelIndex[nColor].find(sPos);
		if (itrReg != m_vecmapRegisteredDefectivePixelIndex[nColor].end())
		{
			const size_t nRegIndex = itrReg->second;

			m_pIInteger_PixelCorrectionIndex->SetValue(nRegIndex);
			m_pIBoolean_PixelCorrectionEnabled->SetValue(false);

			m_vecvecRegisteredDefectivePixel[nColor][nRegIndex].isEnable = false;

			m_vecmapRegisteredDefectivePixelIndex[nColor].erase(itrReg);
		}
	}
	if (m_pICommand_DeviceRegistersStreamingEnd.IsValid())
	{
		m_pICommand_DeviceRegistersStreamingEnd->Execute();
	}

	if (m_pIEnumeration_PixelCorrectionSelector.IsValid())
	{
		m_pIEnumeration_PixelCorrectionSelector->SetIntValue(nInitialSelectorValue);
	}
	m_pIInteger_PixelCorrectionIndex->SetValue(nInitialIndex);
	if (!bEnabledAll)
	{
		m_pIBoolean_PixelCorrectionAllEnabled->SetValue(false);
	}
}
//-----------------------------------------------------------------------------
//Method for setting defective pixel selection information
//-----------------------------------------------------------------------------
void CDefectivePixelManager::SetSelectedPixelInformation(const std::vector<EachDefectPixelPosition_t> &vecPxelList)
{
	GenApi::AutoLock objAuto(m_objLock);
	m_vecSelectedPixelInformation.clear();
	m_vecSelectedPixelInformation = vecPxelList;
}
//-----------------------------------------------------------------------------
//Method for updating the defective pixel list display
//-----------------------------------------------------------------------------
void CDefectivePixelManager::UpdateDefectivePixelList(IDefectivePixelListCtrl *pIDefectivePixelListCtrl)
{
	GenApi::AutoLock objAuto(m_objLock);

	const size_t nColorCount = m_vecPixelColorName.size();

	for (size_t nColor = 0; nColor < nColorCount; ++nColor)
	{
		std::map<std::pair<uint16_t, uint16_t>, size_t> mapAddedItem;
		if (nColor < m_vecvecDefectivePixelInfomation.size())
		{
			for (int i = 0; i < (int)m_vecvecDefectivePixelInfomation[nColor].size(); ++i)
			{
				PSStDefectivePixelInformation_t pInfo = &m_vecvecDefectivePixelInfomation[nColor][i];

				std::pair<uint16_t, uint16_t> pairPos = std::make_pair(pInfo->x, pInfo->y);


				int32_t nRegistered = -1;
				if (nColor < m_vecmapRegisteredDefectivePixelIndex.size())
				{
					std::map<std::pair<uint16_t, uint16_t>, size_t>::iterator itr = m_vecmapRegisteredDefectivePixelIndex[nColor].find(pairPos);
					if (m_vecmapRegisteredDefectivePixelIndex[nColor].end() != itr)
					{
						nRegistered = (int32_t)itr->second;
					}
				}
				pIDefectivePixelListCtrl->AddDefectivePixel(nColor, GCSTRING_2_LPCTSTR(m_vecPixelColorName[nColor]), pInfo, nRegistered);
				mapAddedItem.insert(std::make_pair(pairPos, i));
			}
		}

		if (nColor < m_vecmapRegisteredDefectivePixelIndex.size())
		{
			for (size_t i = 0; i < m_vecvecRegisteredDefectivePixel[nColor].size(); ++i)
			{
				PSRegisteredDefectivePixel_t pRDP = &m_vecvecRegisteredDefectivePixel[nColor][i];
				if (pRDP->isEnable)
				{
					std::pair<uint16_t, uint16_t> pairPos = std::make_pair(pRDP->x, pRDP->y);
					std::map<std::pair<uint16_t, uint16_t>, size_t>::iterator itr = mapAddedItem.find(pairPos);
					if (itr == mapAddedItem.end())
					{
						pIDefectivePixelListCtrl->AddDefectivePixel(nColor, GCSTRING_2_LPCTSTR(m_vecPixelColorName[nColor]), pRDP->x, pRDP->y, (int32_t)i);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool CDefectivePixelManager::GetHighlight() const
{
	return(m_pIStRegisteredCallback_Overlay.IsValid());
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CDefectivePixelManager::SetHighlight(bool value)
{
	if (value)
	{
		if (!m_pIStRegisteredCallback_Overlay.IsValid())
		{
			m_pIStRegisteredCallback_Overlay.Reset(RegisterCallback(m_pIStImageDisplayWnd, *this, &CDefectivePixelManager::OnStCallbackForOverlay, (void*)NULL));
		}
	}
	else
	{
		m_pIStRegisteredCallback_Overlay.Reset(NULL);
	}
}


