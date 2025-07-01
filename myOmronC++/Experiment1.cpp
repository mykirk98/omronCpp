#include "Experiment1.h"

Experiment1::Experiment1()
	: pICommandTriggerSoftware(nullptr)
{
}

Experiment1::~Experiment1()
{
	stopAcquisition();
}

bool Experiment1::initialize()
{
	try
	{
		// Create a camera device object and connect to the first detected device.
		m_pSystem = CreateIStSystem();

		m_pDevice = m_pSystem->CreateFirstIStDevice();

		std::cout << "Device: " << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << std::endl;

		GenApi::CNodeMapPtr pINodeMap(m_pDevice->GetRemoteIStPort()->GetINodeMap());

		SetTriggerMode(pINodeMap, TRIGGER_SELECTOR_FRAME_START, TRIGGER_MODE_ON, TRIGGER_SOURCE_SOFTWARE);
		pICommandTriggerSoftware = pINodeMap->GetNode(TRIGGER_SOFTWARE);

		m_pDataStream = m_pDevice->CreateIStDataStream(0);


		RegisterCallback(m_pDataStream, &Experiment1::OnStCallbackMethod, this);

		return true;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Initialization error: " << e.GetDescription() << std::endl;
		return false;
	}
}

void Experiment1::startAcquisition()
{
	try
	{
		m_pDataStream->StartAcquisition();

		m_pDevice->AcquisitionStart();
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Start acquisition error: " << e.GetDescription() << std::endl;
	}
}

void Experiment1::stopAcquisition()
{
	try
	{
		m_pDevice->AcquisitionStop();

		m_pDataStream->StopAcquisition();
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Stop acquisition error: " << e.GetDescription() << std::endl;
	}
}

void Experiment1::OnStCallbackMethod(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext)
{
	if (pvContext)
	{
		static_cast<Experiment1*>(pvContext)->OnCallback(pIStCallbackParamBase);
	}
}

void Experiment1::OnCallback(IStCallbackParamBase* pCallbackParam)
{
	try
	{
		if (pCallbackParam->GetCallbackType() == StCallbackType_GenTLEvent_DataStreamNewBuffer)
		{
			IStCallbackParamGenTLEventNewBuffer* pNewBufferParam = dynamic_cast<IStCallbackParamGenTLEventNewBuffer*>(pCallbackParam);
			
			IStDataStream* pDataStream = pNewBufferParam->GetIStDataStream();

			CIStStreamBufferPtr pStreamBuffer(pDataStream->RetrieveBuffer(0));

			if (pStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{
				IStImage* pImage = pStreamBuffer->GetIStImage();

				const uint64_t frameID = pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID();
				PrintFrameInfo(pImage, pStreamBuffer);

				//std::string targetDir = "C:\\Users\\USER\\Pictures\\";//NOTE: HOME PC DIRECTORY
				std::string targetDir = "C:\\Users\\mykir\\Work\\Experiments\\";	//NOTE: LAB PC DIRECTORY
				ConvertAndSaveImage<BMP>(pImage, true, targetDir, frameID);
				//ConvertAndSaveImage<BMP>(m_pImage, true, targetDir, m_frameID);
			}
			else
			{
				std::cout << "No image present in the buffer." << std::endl;
			}
		}
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Callback Exception: " << e.GetDescription() << std::endl;
	}
}

void Experiment1::SetEnumeration(GenApi::INodeMap* pInodeMap, const char* szEnumerationName, const char* szValueName)
{
	try
	{
		GenApi::CEnumerationPtr pIEnumeration(pInodeMap->GetNode(szEnumerationName));

		GenApi::CEnumEntryPtr pIEnumEntry(pIEnumeration->GetEntryByName(szValueName));

		pIEnumeration->SetIntValue(pIEnumEntry->GetValue());
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Setting enumeration failed: " << e.GetDescription() << std::endl;
	}
}

void Experiment1::SetTriggerMode(GenApi::CNodeMapPtr& pINodeMap, const char* triggerSelector, const char* triggerMode, const char* triggerSource)
{
	try
	{
		SetEnumeration(pINodeMap, TRIGGER_SELECTOR, triggerSelector);
		SetEnumeration(pINodeMap, TRIGGER_MODE, triggerMode);
		SetEnumeration(pINodeMap, TRIGGER_SOURCE, triggerSource);
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Setting trigger mode failed: " << e.GetDescription() << std::endl;
	}
}

/*
int main()
{
	CameraWorkerCB cameraWorker;
	if (cameraWorker.initialize())
	{
		cameraWorker.startAcquisition();

		while (true)
		{
			std::cout << "0: Generate trigger" << std::endl;
			std::cout << "Else: Exit" << std::endl;
			std::cout << "Select: ";

			size_t nindex;
			std::cin >> nindex;
			if (nindex == 0)
			{
				cameraWorker.pICommandTriggerSoftware->Execute();
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		std::cerr << "Camera initialization failed." << std::endl;
	}
	return 0;
}
*/