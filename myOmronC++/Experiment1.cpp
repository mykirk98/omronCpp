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
		// �ý��� ��ü ���� (��ġ �˻� �� ����)
		m_pSystem = CreateIStSystem();

		// ù ���� ��ġ ���� �� ����
		m_pDevice = m_pSystem->CreateFirstIStDevice();

		// ��ġ ���� ���
		std::cout << "Device: " << m_pDevice->GetIStDeviceInfo()->GetDisplayName() << std::endl;

		// ī�޶� ������ ���� ���� ��������
		GenApi::CNodeMapPtr pINodeMap(m_pDevice->GetRemoteIStPort()->GetINodeMap());
		// Ʈ���Ÿ�� ����
		SetTriggerMode(pINodeMap, TRIGGER_SELECTOR_FRAME_START, TRIGGER_MODE_ON, TRIGGER_SOURCE_SOFTWARE);
		pICommandTriggerSoftware = pINodeMap->GetNode(TRIGGER_SOFTWARE);

		// �̹��� ��Ʈ�� �����͸� ó���ϱ� ���� �����ͽ�Ʈ�� ��ü ����
		m_pDataStream = m_pDevice->CreateIStDataStream(0);

		// ������ ��Ʈ�� �ݹ� ���� (this �����͸� pvContext�� ����)
		RegisterCallback(m_pDataStream, &Experiment1::OnStCallbackMethod, this);
		//RegisterCallback(m_pDataStream, &CameraWorker_CB::OnStCallbackFunction, nullptr);	// nullptr�� �ѱ� ���, �ݹ� �Լ����� this �����͸� ����� �� ����
		// NOTE: this�� �ѱ�� ���� : �ݹ��� �߻����� ��, � ��ü�� ��� �Լ��� ó���� �������� �˷��ֱ� ����

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
		// ȣ��Ʈ(PC) �� �̹��� ȹ�� ����
		m_pDataStream->StartAcquisition();

		// ī�޶� �� �̹��� ȹ�� ����
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
		// ī�޶� �� �̹��� ȹ�� ����
		m_pDevice->AcquisitionStop();

		// ȣ��Ʈ(PC) �� �̹��� ȹ�� ����
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
		// pvContext�� �ѱ� this �����͸� �ٽ� ĳ�����Ͽ� ��� �Լ� ȣ��
		static_cast<Experiment1*>(pvContext)->OnCallback(pIStCallbackParamBase);
		// static_cast : C++���� �� ��ȯ�� �� �� ����ϴ� ������, ������ Ÿ�ӿ� ��ȯ
		// <> : ���ø��� ����Ͽ� Ÿ���� ����
	}
}

// ��� �ݹ� ó�� �Լ�
void Experiment1::OnCallback(IStCallbackParamBase* pCallbackParam)
{
	try
	{
		// �ݹ� �Ķ������ Ÿ�� Ȯ��
		if (pCallbackParam->GetCallbackType() == StCallbackType_GenTLEvent_DataStreamNewBuffer)
		{
			IStCallbackParamGenTLEventNewBuffer* pNewBufferParam = dynamic_cast<IStCallbackParamGenTLEventNewBuffer*>(pCallbackParam);
			// NOTE: dynamic_cast�� ����� ���� : �������� Ȱ���Ͽ� IStCallbackParamBase���� �Ļ��� 
			//									IStCallbackParamGenTLEventNewBuffer Ÿ������ �����ϰ� �ٿ�ĳ�����ϱ� ����
			// NOTE: static_cast���� ������ : dynamic_cast�� ��Ÿ�ӿ� Ÿ�� üũ�� �����Ͽ� ������ ��� nullptr�� ��ȯ

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
		// IEnumeration �������̽� ������ ��������
		GenApi::CEnumerationPtr pIEnumeration(pInodeMap->GetNode(szEnumerationName));

		// ������ �̸��� IEnumEntry �������̽� ������ ��������
		GenApi::CEnumEntryPtr pIEnumEntry(pIEnumeration->GetEntryByName(szValueName));

		// IEnumEntry �������̽� �����͸� ����Ͽ� ���� �� ��������
		// IEnumeration �������̽� �����͸� ����Ͽ� ���� ������Ʈ
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
		// TriggerSelector ��� ����
		SetEnumeration(pINodeMap, TRIGGER_SELECTOR, triggerSelector);
		// TriggerMode ��� ����
		SetEnumeration(pINodeMap, TRIGGER_MODE, triggerMode);
		// TriggerSource ��� ����
		SetEnumeration(pINodeMap, TRIGGER_SOURCE, triggerSource);
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "Setting trigger mode failed: " << e.GetDescription() << std::endl;
	}
}

// ��� ���� (main.cpp���� ȣ��)
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