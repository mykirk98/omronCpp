#pragma once

class CSaveMultipleImagesFileBase
{
public:
	CSaveMultipleImagesFileBase(void);
	virtual ~CSaveMultipleImagesFileBase(void);


	/*!
	Add new frame to AVI file.
	\param[in]	pIStDevice	IStDevice pointer that is a image souce of the avi file.
	\return bool	 Return false if a configuration was canceled or if an exception occurred.
	*/
	bool Open(StApi::IStDevice *pIStDevice, StApi::IStImageDisplayWnd *pIStImageDisplayWnd);

	/*!
	Add new frame to AVI file.
	\param[in]	pIStStreamBuffer	IStStreamBuffer pointer of new frame.
	\return bool	 Return true if AVI file already closed.
	*/
	virtual bool RegisterIStStreamBuffer(StApi::IStStreamBuffer *pIStStreamBuffer) = 0;
protected:
	virtual bool mOpen(StApi::IStDevice *pIStDevice, StApi::IStImageDisplayWnd *pIStImageDisplayWnd) = 0;



	StApi::CIStPixelFormatConverterPtr m_pIStPixelFormatConverter;
};


class CAVIFile : public CSaveMultipleImagesFileBase
{
public:
	CAVIFile(void);
	~CAVIFile(void);

	bool RegisterIStStreamBuffer(StApi::IStStreamBuffer *pIStStreamBuffer);
protected:
	bool mOpen(StApi::IStDevice *pIStDevice, StApi::IStImageDisplayWnd *pIStImageDisplayWnd);

	StApi::CIStVideoFilerPtr m_pIStVideoFiler;
	StApi::CIStImageBufferPtr m_pIStImageBuffer;

	uint64_t m_iTimestampOffset;
	bool m_isFirstFrame;
	double m_dblCameraFrameRate;
	bool m_isNeedToConvBeforeReg;
};

class CStillImageFiles : public CSaveMultipleImagesFileBase
{
public:
	CStillImageFiles(void);
	~CStillImageFiles(void);

	bool RegisterIStStreamBuffer(StApi::IStStreamBuffer *pIStStreamBuffer);

	static unsigned int WINAPI sThreadStart(void* pParam)
	{
		return(static_cast<CStillImageFiles*>(pParam)->mWork());
	}
	unsigned int mWork();
	void CreateBuffer(size_t nCount);
	void DeleteAllBuffer();
	void Start();
	void Stop();
protected:
	bool mOpen(StApi::IStDevice *pIStDevice, StApi::IStImageDisplayWnd *pIStImageDisplayWnd);

	GenApi::CLock m_LockList;
	CEvent m_eventIsEmptyBuffer;
	CEvent m_eventIsFullBuffer;
	CEvent m_eventIsStopRequest;
	HANDLE m_hThread;

	StApi::EStStillImageFileFormat_t m_eSIFF;
	int m_nSaveAImagePer;
	size_t m_nToRegImageCount;
	size_t m_nSavedImageCount;
	int m_nMaximumImageFileCount;

	CString m_strFilePath;
	CString m_strFileNamePattern;


	StApi::CIStStillImageFilerPtr m_IStStillImageFiler;

	std::vector<StApi::IStImageBufferReleasable*> m_vecEmptyBufferList;
	std::vector<StApi::IStImageBufferReleasable*> m_vecFullBufferList;
};

