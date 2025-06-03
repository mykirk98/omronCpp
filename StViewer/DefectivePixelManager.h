#pragma once
#include <map>

#define LIST_COL_STATUS 0
#define LIST_COL_COLOR 1
#define LIST_COL_X 2
#define LIST_COL_Y 3
#define LIST_COL_EVALUATION 4
#define LIST_COL_REFERENCE 5
#define LIST_COL_DIFFERENCE 6

#define ITEM_TYPE_MAIN_OR_R 0
#define ITEM_TYPE_G 1
#define ITEM_TYPE_B 2

#if 1900 <= _MSC_VER	//VS2015-
#define ENABLED_TUPLE
#endif
#ifdef ENABLED_TUPLE
typedef std::tuple<int, uint16_t, uint16_t> EachDefectPixelPosition_t;
#else
typedef std::pair<int, std::pair<uint16_t, uint16_t>> EachDefectPixelPosition_t;
#endif //ENABLED_TUPLE

//Interface used when displaying the defective pixel list
STAPI_INTERFACE IDefectivePixelListCtrl
{
	virtual void AddDefectivePixel(uint8_t nColor, LPCTSTR szColor, StApi::PSStDefectivePixelInformation_t pInfo, int32_t nRegistered = -1) = 0;
	virtual void AddDefectivePixel(uint8_t nColor, LPCTSTR szColor, uint16_t x, uint16_t y, int32_t nRegistered = -1) = 0;
};


class CDefectivePixelManager
{
public:
	CDefectivePixelManager(GenApi::INodeMap *pINodeMap, StApi::IStImageDisplayWnd *pIStImageDisplayWnd);
	~CDefectivePixelManager();

public:
	static bool IsSupported(GenApi::INodeMap *pINodemap);


	//Comparison method used to sort the defective pixel list display
	int Compare(int iColumn, EachDefectPixelPosition_t *pPosition);

	//Method for setting defective pixel selection information
	void SetSelectedPixelInformation(const std::vector<EachDefectPixelPosition_t> &vecPxelList);

	//Method for registering defective pixels to the camera
	bool RegisterSelectedPixel(std::vector<EachDefectPixelPosition_t> &vecPxelList);

	//Method to Unregister Defective Pixels in Camera
	void DeregisterSelectedPixel(std::vector<EachDefectPixelPosition_t> &vecPxelList);

	//Method for updating the defective pixel list display
	void UpdateDefectivePixelList(IDefectivePixelListCtrl *pIDefectivePixelListCtrl);
public:
	GenApi::INodeMap *GetINodeMapForDefectivePixelDetectionFilter()
	{
		return(m_pIStDefectivePixelDetectionFilter->GetINodeMap());
	}
	void DetectDefectivePixel(StApi::IStImage *pIStImage);

	bool GetHighlight() const;
	void SetHighlight(bool value);
public:
	size_t GetDetectedDefectivePixelCount();
	void GetRegisteredDefectivePixelList();
	void ClearDetectedPixelList();
protected:
	const GenApi::CNodeMapPtr m_pINodeMap_RemoteDevice;
	StApi::IStImageDisplayWnd * const m_pIStImageDisplayWnd;
	StApi::CIStRegisteredCallbackPtr m_pIStRegisteredCallback_Overlay;
protected:
	const GenApi::CBooleanPtr m_pIBoolean_PixelCorrectionAllEnabled;
	const GenApi::CEnumerationPtr m_pIEnumeration_PixelCorrectionSelector;
	const GenApi::CIntegerPtr m_pIInteger_PixelCorrectionIndex;
	const GenApi::CBooleanPtr m_pIBoolean_PixelCorrectionEnabled;
	const GenApi::CIntegerPtr m_pIInteger_PixelCorrectionX;
	const GenApi::CIntegerPtr m_pIInteger_PixelCorrectionY;
	const GenApi::CCommandPtr m_pICommand_DeviceRegistersStreamingStart;
	const GenApi::CCommandPtr m_pICommand_DeviceRegistersStreamingEnd;
	std::vector<int64_t> m_vecPixelCorrectionSelectorAvailableValueList;
protected:
	StApi::CIStDefectivePixelDetectionFilterPtr m_pIStDefectivePixelDetectionFilter;
	const GenApi::CNodeMapPtr m_pINodeMap_DefectivePixelDetectionFilter;
	const GenApi::CEnumerationPtr m_pIEnumeration_ExpectedPixelFormat;
	const GenApi::CIntegerPtr m_pIInteger_MaximumPixelCountToDetect;
protected:

	void OnStCallbackForOverlay(StApi::IStCallbackParamBase *pIStCallbackParamBase, void* pvContext);

	GenApi::CLock m_objLock;

	//Registered defective pixel information
	std::vector<std::map<std::pair<uint16_t, uint16_t>, size_t>> m_vecmapRegisteredDefectivePixelIndex;
	typedef struct _SRegisteredDefectivePixel_t
	{
		uint16_t x;
		uint16_t y;
		bool isEnable;
	}SRegisteredDefectivePixel_t, *PSRegisteredDefectivePixel_t;
	std::vector<std::vector<SRegisteredDefectivePixel_t>> m_vecvecRegisteredDefectivePixel;

	//Detected defective pixels information
	std::vector <std::map<std::pair<uint16_t, uint16_t>, size_t>> m_vecmapDetectedDefectivePixelIndex;
	std::vector<std::vector<StApi::SStDefectivePixelInformation_t>> m_vecvecDefectivePixelInfomation;
	GenICam::gcstring_vector m_vecPixelColorName;

	//Selected defective pixels information
	std::vector<EachDefectPixelPosition_t> m_vecSelectedPixelInformation;

};

