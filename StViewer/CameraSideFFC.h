#pragma once

#include <algorithm>

#include "IStImageAveragingFilter.h"
#include "IStImageCallback.h"
#define FFC_PARAM_UNIT 128
class CCameraSideFFC /*final*//* Comment out for VS2010. */ : public IStImageCallback
{
public:
	explicit CCameraSideFFC(StApi::IStDevice *pIStDevice);
	~CCameraSideFFC();
	static bool IsSupported(GenApi::CNodeMapPtr pINodeMap);
	void Wait(uint32_t);
	void OnIStImage(StApi::IStImage *pIStImage) override;
	void CalculateAndSend();
protected:
	StApi::IStDevice * const m_pIStDevice;
	StApi::CIStImageAveragingFilterPtr m_pIStImageAveragingFilter;
	StApi::CIStImageBufferPtr m_pIStImageBuffer;

	int64_t m_nMaxGainValue;
	double m_dblBlackLevelVaule;
	size_t m_nFrameCount;
	size_t m_nRcvedFrameCount;
	double m_dblRate;

	size_t m_nImageWidthMax;
	size_t m_nImageHeightMax;
	size_t m_nAreaCountX;
	size_t m_nAreaCountY;
	size_t m_nAreaCount;

	const GenApi::CNodeMapPtr m_pINodeMap;
	const GenApi::CIntegerPtr m_pIIntegerFFCMeshWidth;
	const GenApi::CIntegerPtr m_pIIntegerFFCMeshHeight;
	const GenApi::CBooleanPtr m_pIBooleanFFCEnable;
	const GenApi::CIntegerPtr m_pIIntegerFFCIndex;
	const GenApi::CIntegerPtr m_pIIntegerFFCValue;
	const GenApi::CRegisterPtr m_pIRegisterFFCValueAll;
	const GenApi::CEnumerationPtr m_pIEnumerationDeviceRegistersEndianness;
	const GenApi::CEnumerationPtr m_pIEnumerationFFCMode;
	const GenApi::CEnumerationPtr m_pIEnumerationFFCSensorSelector;

	CEvent m_objRcvImageDoneEvent;

	bool isLittleEndian() const
	{
		const uint16_t nValue = 1;
		return((*(char*)&nValue) != 0);
	}

	uint8_t SwapEndian(uint8_t nValue) const { return(nValue); }
	uint16_t SwapEndian(uint16_t nValue) const { return(((nValue & 0xFF) << 8) | (nValue >> 8)); }
	uint32_t SwapEndian(uint32_t nValue) const { return(((nValue & 0xFF) << 24) | ((nValue & 0xFF00) << 8) | ((nValue & 0xFF0000) >> 8) | (nValue >> 24)); }

	template <class PixelComponentTypt_t, class GainCorrectionType_t> void mFlatShading(std::vector<std::vector<GainCorrectionType_t>> &vecvecGainList, std::vector<std::vector<PixelComponentTypt_t>> &vecvecEachAreaMedianValue)
	{
		size_t nGainListSize = (m_nAreaCountX + 1) * (m_nAreaCountY + 1);
		if ((sizeof(GainCorrectionType_t) * nGainListSize) % 4)
		{
			nGainListSize += (4 - ((sizeof(GainCorrectionType_t) * nGainListSize) % 4)) / sizeof(GainCorrectionType_t);
		}
		const double dblBaseGain = (1 < vecvecGainList.size()) ? 0 : 1;
		for (size_t nColor = 0; nColor < vecvecGainList.size(); ++nColor)
		{
			std::vector<PixelComponentTypt_t> &vecEachAreaMedianValue = vecvecEachAreaMedianValue[nColor];
			PixelComponentTypt_t nMax = 0;
			for (size_t i = 0; i < vecEachAreaMedianValue.size(); ++i)
			{
				if (nMax < vecEachAreaMedianValue[i])
				{
					nMax = vecEachAreaMedianValue[i];
				}
			}

			const PixelComponentTypt_t nBlackLevel = (PixelComponentTypt_t)(m_dblBlackLevelVaule * m_dblRate + 0.5);
			if (nMax <= nBlackLevel)
			{
				nMax = 0;
			}
			else
			{
				nMax -= nBlackLevel;
			}


			std::vector<GainCorrectionType_t> &vecGainList = vecvecGainList[nColor];
			vecGainList.resize(nGainListSize);
			memset(&vecGainList[0], 0, sizeof(GainCorrectionType_t) * nGainListSize);

			for (size_t i = 0; i < vecEachAreaMedianValue.size(); ++i)
			{
				int64_t nGainValue = m_nMaxGainValue;
				if (nBlackLevel < vecEachAreaMedianValue[i])
				{
					double dblGain = nMax / (double)(vecEachAreaMedianValue[i] - nBlackLevel);
					dblGain -= dblBaseGain;
					nGainValue = (int64_t)(dblGain * FFC_PARAM_UNIT);
					if (m_nMaxGainValue < nGainValue) nGainValue = m_nMaxGainValue;
				}
				vecGainList[i] = (GainCorrectionType_t)nGainValue;
			}
		}
	}

	template <class PixelComponentTypt_t, class GainCorrectionType_t> void ColorShading(std::vector<std::vector<GainCorrectionType_t>> &vecvecGainList, std::vector<std::vector<PixelComponentTypt_t>> &vecvecEachAreaMedianValue)
	{
		size_t nGainListSize = (m_nAreaCountX + 1) * (m_nAreaCountY + 1);
		if ((sizeof(GainCorrectionType_t) * nGainListSize) % 4)
		{
			nGainListSize += (4 - ((sizeof(GainCorrectionType_t) * nGainListSize) % 4)) / sizeof(GainCorrectionType_t);
		}
		std::vector<PixelComponentTypt_t> &vecEachAreaMedianValueG = vecvecEachAreaMedianValue[1];
		const PixelComponentTypt_t nBlackLevel = (PixelComponentTypt_t)(m_dblBlackLevelVaule * m_dblRate + 0.5);
		
		size_t nMaxPos = 0;
		PixelComponentTypt_t nMaxG = 0;
		for (size_t i = 0; i < vecEachAreaMedianValueG.size(); ++i)
		{
			if (nMaxG < vecEachAreaMedianValueG[i])
			{
				nMaxG = vecEachAreaMedianValueG[i];
				nMaxPos = i;
			}
		}

		PixelComponentTypt_t pnMax[] = { vecvecEachAreaMedianValue[0][nMaxPos], nMaxG, vecvecEachAreaMedianValue[2][nMaxPos] };
		for (size_t nColor = 0; nColor < 3; ++nColor)
		{
			if (pnMax[nColor] <= nBlackLevel)
			{
				pnMax[nColor] = 0;
			}
			else
			{
				pnMax[nColor] -= nBlackLevel;
			}

		}

		std::vector<GainCorrectionType_t> &vecGainListR = vecvecGainList[0];
		std::vector<GainCorrectionType_t> &vecGainListG = vecvecGainList[1];
		std::vector<GainCorrectionType_t> &vecGainListB = vecvecGainList[2];
		vecGainListR.resize(nGainListSize);
		vecGainListG.resize(nGainListSize);
		vecGainListB.resize(nGainListSize);

		if (nMaxG <= 0)
		{
			for (size_t i = 0; i < vecEachAreaMedianValueG.size(); ++i)
			{
				vecGainListR[i] = vecGainListG[i] = vecGainListB[i] = FFC_PARAM_UNIT;
			}
		}
		else
		{


			const double pdblMaxRatio[] = { pnMax[0] / (double)nMaxG, 1, pnMax[2] / (double)nMaxG };
			for (size_t i = 0; i < vecEachAreaMedianValueG.size(); ++i)
			{
				double pdblCurRatio[3];
				for (size_t nColor = 0; nColor < 3; ++nColor)
				{
					pdblCurRatio[nColor] = (double)vecvecEachAreaMedianValue[nColor][i] - nBlackLevel;
				}

				if (pdblCurRatio[1] <= 0) //Green
				{
					vecGainListR[i] = vecGainListG[i] = vecGainListB[i] = FFC_PARAM_UNIT;
				}
				else
				{
					pdblCurRatio[0] /= pdblCurRatio[1];
					pdblCurRatio[2] /= pdblCurRatio[1];

					for (size_t nColor = 0; nColor < 3; ++nColor)
					{
						if (nColor == 1)
						{
							vecGainListG[i] = FFC_PARAM_UNIT;	//Green
						}
						else
						{
							const double dblGain = pdblMaxRatio[nColor] / pdblCurRatio[nColor];
							int64_t nGainValue = (int64_t)(dblGain * FFC_PARAM_UNIT);
							if (m_nMaxGainValue < nGainValue) nGainValue = m_nMaxGainValue;
							vecvecGainList[nColor][i] = (GainCorrectionType_t)nGainValue;
						}
					}
					
				}
			}
		}
	
	}

	template <class PixelComponentTypt_t, class GainCorrectionType_t> void mCalculate(const StApi::IStPixelFormatInfo *pIStPixelFormatInfo, std::vector<std::vector<GainCorrectionType_t>> &vecvecGainList)
	{
		std::vector<std::vector<PixelComponentTypt_t>> vecvecEachAreaMedianValue;
		vecvecEachAreaMedianValue.resize(vecvecGainList.size());
		for (size_t nColor = 0; nColor < vecvecGainList.size(); ++nColor)
		{
			vecvecEachAreaMedianValue[nColor].resize((m_nAreaCountX + 1) * (m_nAreaCountY + 1));
		}
		if (pIStPixelFormatInfo->IsMono())
		{
			GetMedianMono<PixelComponentTypt_t>(m_pIStImageBuffer->GetIStImage(), vecvecEachAreaMedianValue[0]);
		}
		else if (pIStPixelFormatInfo->IsBayer())
		{
			switch (pIStPixelFormatInfo->GetPixelColorFilter())
			{
			case(StApi::StPixelColorFilter_BayerRG) :
			case(StApi::StPixelColorFilter_BayerBG) :
				GetMedianBayer<PixelComponentTypt_t>(m_pIStImageBuffer->GetIStImage(), vecvecEachAreaMedianValue[0], false);
				break;
			case(StApi::StPixelColorFilter_BayerGR) :
			case(StApi::StPixelColorFilter_BayerGB) :
				GetMedianBayer<PixelComponentTypt_t>(m_pIStImageBuffer->GetIStImage(), vecvecEachAreaMedianValue[0], true);
				break;
			}
		}
		else
		{
			switch (pIStPixelFormatInfo->GetValue())
			{
			case(StApi::StPFNC_RGB8):
			case(StApi::StPFNC_RGBa8):
			case(StApi::StPFNC_RGB10):
			case(StApi::StPFNC_RGBa10):
			case(StApi::StPFNC_RGB12):
			case(StApi::StPFNC_RGBa12):
			case(StApi::StPFNC_RGB14):
			case(StApi::StPFNC_RGBa14):
			case(StApi::StPFNC_RGB16):
			case(StApi::StPFNC_RGBa16):
				GetMedianRGB<PixelComponentTypt_t>(m_pIStImageBuffer->GetIStImage(), vecvecEachAreaMedianValue, true);
				break;
			case(StApi::StPFNC_BGR8):
			case(StApi::StPFNC_BGRa8):
			case(StApi::StPFNC_BGR10):
			case(StApi::StPFNC_BGRa10):
			case(StApi::StPFNC_BGR12):
			case(StApi::StPFNC_BGRa12):
			case(StApi::StPFNC_BGR14):
			case(StApi::StPFNC_BGRa14):
			case(StApi::StPFNC_BGR16):
			case(StApi::StPFNC_BGRa16):
				GetMedianRGB<PixelComponentTypt_t>(m_pIStImageBuffer->GetIStImage(), vecvecEachAreaMedianValue, false);
				break;
			default:
				throw RUNTIME_EXCEPTION("Invalid pixel format");
				break;
			}
		}

		if (m_pIEnumerationFFCMode.IsValid() && (m_pIEnumerationFFCMode->GetCurrentEntry()->GetSymbolic().compare("ColorShading") == 0))
		{
			ColorShading(vecvecGainList, vecvecEachAreaMedianValue);
		}
		else
		{
			mFlatShading(vecvecGainList, vecvecEachAreaMedianValue);
		}

	}


	template <class PixelComponentTypt_t, class GainCorrectionType_t> void mCalculateAndSend(const StApi::IStPixelFormatInfo *pIStPixelFormatInfo)
	{
		size_t nColorCount = 1;
		if (m_pIEnumerationFFCSensorSelector.IsValid())
		{
			GenApi::NodeList_t nodeList;
			m_pIEnumerationFFCSensorSelector->GetEntries(nodeList);
			nColorCount = 0;
			for (GenApi::NodeList_t::iterator itr = nodeList.begin(); itr != nodeList.end(); ++itr)
			{
				if (IsAvailable(*itr))
				{
					++nColorCount;
				}
			}
		}

		std::vector<std::vector<GainCorrectionType_t>> vecvecGainList;
		vecvecGainList.resize(nColorCount);

		mCalculate<PixelComponentTypt_t, GainCorrectionType_t>(pIStPixelFormatInfo, vecvecGainList);
		mSend(vecvecGainList);
	}


	template <class GainCorrectionType_t>
	void mSend(std::vector<std::vector<GainCorrectionType_t>> &vecvecGainList)
	{
		m_pIBooleanFFCEnable->SetValue(false);

		for (size_t nColor = 0; nColor < vecvecGainList.size(); ++nColor)
		{
			if (m_pIEnumerationFFCSensorSelector.IsValid())
			{
				if (1 < vecvecGainList.size())
				{
					const char * const pszSensor[] = { "Red", "Green", "Blue" };
					*m_pIEnumerationFFCSensorSelector = pszSensor[nColor];
				}
			}

			std::vector<GainCorrectionType_t> &vecGainList = vecvecGainList[nColor];
			if (m_pIRegisterFFCValueAll.IsValid())
			{
				//ValueAll
				const bool isSysLE = isLittleEndian();
				bool isCamLE = true;
				if (m_pIEnumerationDeviceRegistersEndianness)
				{
					isCamLE = (m_pIEnumerationDeviceRegistersEndianness->GetCurrentEntry()->GetSymbolic().compare("Little") == 0);
				}
				if ((isSysLE && (!isCamLE)) || ((!isSysLE) && isCamLE))
				{
					//Swap endian
					for (size_t i = 0; i < vecGainList.size(); ++i)
					{
						vecGainList[i] = SwapEndian(vecGainList[i]);
					}
				}

				m_pIRegisterFFCValueAll->Set((uint8_t*)&vecGainList[0], sizeof(vecGainList[0]) * vecGainList.size());
			}
			else
			{
				for (size_t i = 0; i < vecGainList.size(); ++i)
				{
					m_pIIntegerFFCIndex->SetValue(i);
					m_pIIntegerFFCValue->SetValue(vecGainList[i]);
				}
			}
		}
		m_pIBooleanFFCEnable->SetValue(true);

	}

	#define MEDIAN_SIZE 3
	template <class X> X GetEachAreaMedianMono(X *pxTLBuffer, size_t nLinePitch)
	{
		std::vector<X> vecData;
		for (size_t y = 0; y < MEDIAN_SIZE; ++y)
		{
			for (size_t x = 0; x < MEDIAN_SIZE; ++x)
			{
				vecData.push_back(pxTLBuffer[x]);
			}
			pxTLBuffer = reinterpret_cast<X*>(reinterpret_cast<uint8_t*>(pxTLBuffer) + nLinePitch);
		}

		std::sort(vecData.begin(), vecData.end());

		const size_t nCount = vecData.size();
		if (nCount & 1)
		{
			return(vecData[nCount / 2]);
		}
		else
		{
			size_t nIndex = nCount / 2;
			return((X)((vecData[nIndex] + vecData[nIndex - 1]) / 2));
		}
	}
	template <class X> void GetMedianMono(StApi::IStImage *pIStImage, std::vector<X> &vecEachAreaMedian)
	{
		const size_t nImageLinePitch = pIStImage->GetImageLinePitch();
		const size_t nMeshWidth = (size_t)m_pIIntegerFFCMeshWidth->GetValue();
		const size_t nMeshHeight = (size_t)m_pIIntegerFFCMeshHeight->GetValue();

		size_t nAreaIndex = 0;
		size_t nOffsetY = 0;
		for (size_t nY = 0; nY < m_nAreaCountY + 1; ++nY)
		{
			X * pxSrcBufferLine = reinterpret_cast<X*>(reinterpret_cast<uint8_t*>(pIStImage->GetImageBuffer()) + nOffsetY * nImageLinePitch);

			size_t nOffsetX = 0;
			for (size_t nX = 0; nX < m_nAreaCountX + 1; ++nX)
			{
				vecEachAreaMedian[nAreaIndex] = GetEachAreaMedianMono<X>(&pxSrcBufferLine[nOffsetX], nImageLinePitch);
				++nAreaIndex;

				nOffsetX += nMeshWidth;
				if (nX + 1 == m_nAreaCountX)
				{
					nOffsetX = m_nImageWidthMax - MEDIAN_SIZE;
				}
			}

			nOffsetY += nMeshHeight;
			if (nY + 1 == m_nAreaCountY)
			{
				nOffsetY = m_nImageHeightMax - MEDIAN_SIZE;
			}
		}
	}
	template <class X> X GetEachAreaMedianBayer(X *pxTLBuffer, size_t nLinePitch, bool isGreen)	//eType 0:RG or BG, 1:GR or GB
	{
		std::vector<X> vecData;
		for (size_t y = 0; y < MEDIAN_SIZE; ++y)
		{
			if ((y & 1) == (size_t)(isGreen ? 0 : 1))
			{
				for (size_t x = 0; x < MEDIAN_SIZE; ++x)
				{
					if ((x & 1) == 0)
					{
						vecData.push_back(pxTLBuffer[x]);
					}
				}
			}
			else
			{
				for (size_t x = 0; x < MEDIAN_SIZE; ++x)
				{
					if ((x & 1) == 1)
					{
						vecData.push_back(pxTLBuffer[x]);
					}
				}
			}
			pxTLBuffer = reinterpret_cast<X*>(reinterpret_cast<uint8_t*>(pxTLBuffer) + nLinePitch);
		}

		std::sort(vecData.begin(), vecData.end());

		const size_t nCount = vecData.size();
		if (nCount & 1)
		{
			return(vecData[nCount / 2]);
		}
		else
		{
			size_t nIndex = nCount / 2;
			return((X)((vecData[nIndex] + vecData[nIndex - 1]) / 2));
		}
	}
	template <class X> void GetMedianBayer(StApi::IStImage *pIStImage, std::vector<X> &vecEachAreaMedian, bool isGreen)
	{
		const size_t nImageLinePitch = pIStImage->GetImageLinePitch();
		const size_t nMeshWidth = (size_t)m_pIIntegerFFCMeshWidth->GetValue();
		const size_t nMeshHeight = (size_t)m_pIIntegerFFCMeshHeight->GetValue();
		const bool isGreenOnEdge = (MEDIAN_SIZE & 1) ? (isGreen ? false : true) : isGreen;
		size_t nAreaIndex = 0;
		size_t nOffsetY = 0;
		for (size_t nY = 0; nY < m_nAreaCountY; ++nY)
		{
			X * pxSrcBufferLine = reinterpret_cast<X*>(reinterpret_cast<uint8_t*>(pIStImage->GetImageBuffer()) + nOffsetY * nImageLinePitch);

			size_t nOffsetX = 0;
			for (size_t nX = 0; nX < m_nAreaCountX; ++nX)
			{
				vecEachAreaMedian[nAreaIndex] = GetEachAreaMedianBayer<X>(&pxSrcBufferLine[nOffsetX], nImageLinePitch, isGreen);
				++nAreaIndex;

				nOffsetX += nMeshWidth;
				if (nX + 1 == m_nAreaCountX)
				{
					nOffsetX = m_nImageWidthMax - MEDIAN_SIZE;
				}
			}

			//Right
			vecEachAreaMedian[nAreaIndex] = GetEachAreaMedianBayer<X>(&pxSrcBufferLine[nOffsetX], nImageLinePitch, isGreenOnEdge);
			++nAreaIndex;

			nOffsetY += nMeshHeight;
			if (nY + 1 == m_nAreaCountY)
			{
				nOffsetY = m_nImageHeightMax - MEDIAN_SIZE;
			}
		}
		{
			//Bottom
			X * pxSrcBufferLine = reinterpret_cast<X*>(reinterpret_cast<uint8_t*>(pIStImage->GetImageBuffer()) + nOffsetY * nImageLinePitch);

			size_t nOffsetX = 0;
			for (size_t nX = 0; nX < m_nAreaCountX; ++nX)
			{
				vecEachAreaMedian[nAreaIndex] = GetEachAreaMedianBayer<X>(&pxSrcBufferLine[nOffsetX], nImageLinePitch, isGreenOnEdge);
				++nAreaIndex;

				nOffsetX += nMeshWidth;
				if (nX + 1 == m_nAreaCountX)
				{
					nOffsetX = m_nImageWidthMax - MEDIAN_SIZE;
				}
			}
			
			//Bottom / Right
			vecEachAreaMedian[nAreaIndex] = GetEachAreaMedianBayer<X>(&pxSrcBufferLine[nOffsetX], nImageLinePitch, isGreen);
		}
	}

	template <class X> X GetEachAreaMedianRGB(X *pxTLBuffer, size_t nLinePitch, size_t nTotalComponentCount)
	{
		std::vector<X> vecData;
		for (size_t y = 0; y < MEDIAN_SIZE; ++y)
		{
			X* pxBufferCurPos = pxTLBuffer;
			for (size_t x = 0; x < MEDIAN_SIZE; ++x)
			{
				vecData.push_back(*pxBufferCurPos);
				pxBufferCurPos += nTotalComponentCount;
			}
			pxTLBuffer = reinterpret_cast<X*>(reinterpret_cast<uint8_t*>(pxTLBuffer) + nLinePitch);
		}

		std::sort(vecData.begin(), vecData.end());

		const size_t nCount = vecData.size();
		if (nCount & 1)
		{
			return(vecData[nCount / 2]);
		}
		else
		{
			size_t nIndex = nCount / 2;
			return((X)((vecData[nIndex] + vecData[nIndex - 1]) / 2));
		}
	}
	template <class X> void GetMedianRGB(StApi::IStImage *pIStImage, std::vector<std::vector<X>> &vecvecEachAreaMedian, bool isRFirst)
	{
		const size_t nImageLinePitch = pIStImage->GetImageLinePitch();
		const size_t nMeshWidth = (size_t)m_pIIntegerFFCMeshWidth->GetValue();
		const size_t nMeshHeight = (size_t)m_pIIntegerFFCMeshHeight->GetValue();
		const StApi::IStPixelFormatInfo *pIStPixelFormatInfo = GetIStPixelFormatInfo(pIStImage->GetImagePixelFormat());
		const size_t nTotalComponentCount = pIStPixelFormatInfo->GetEachPixelTotalComponentCount();
		size_t nAreaIndex = 0;
		size_t nOffsetY = 0;
		const size_t nIndexR = isRFirst ? 0 : 2;
		const size_t nIndexB = isRFirst ? 2 : 0;

		for (size_t nY = 0; nY < m_nAreaCountY + 1; ++nY)
		{
			X * pxSrcBufferLine = reinterpret_cast<X*>(reinterpret_cast<uint8_t*>(pIStImage->GetImageBuffer()) + nOffsetY * nImageLinePitch);

			size_t nOffsetX = 0;
			for (size_t nX = 0; nX < m_nAreaCountX + 1; ++nX)
			{
				vecvecEachAreaMedian[nIndexR][nAreaIndex] = GetEachAreaMedianRGB<X>(&pxSrcBufferLine[nOffsetX], nImageLinePitch, nTotalComponentCount);
				vecvecEachAreaMedian[1][nAreaIndex] = GetEachAreaMedianRGB<X>(&pxSrcBufferLine[nOffsetX + 1], nImageLinePitch, nTotalComponentCount);
				vecvecEachAreaMedian[nIndexB][nAreaIndex] = GetEachAreaMedianRGB<X>(&pxSrcBufferLine[nOffsetX + 2], nImageLinePitch, nTotalComponentCount);
				++nAreaIndex;
				if (nX + 1 == m_nAreaCountX)
				{
					nOffsetX = (m_nImageWidthMax - MEDIAN_SIZE) * nTotalComponentCount;
				}
				else
				{
					nOffsetX += nMeshWidth * nTotalComponentCount;
				}
			}

			nOffsetY += nMeshHeight;
			if (nY + 1 == m_nAreaCountY)
			{
				nOffsetY = m_nImageHeightMax - MEDIAN_SIZE;
			}
		}
	}
};

