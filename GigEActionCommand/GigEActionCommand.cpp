/*!
\file GigEActionCommand.cpp
\brief 
 
 This sample shows how to use GigE Action command.
 The following points will be demonstrated in this sample code:
 - Initialize StApi
 - Connect to GigE camera
 - Set and send action command

 For more information, please refer to the help document of StApi.
 
*/

// If you want to use the GUI features, please remove the comment.
//#define ENABLED_ST_PREVIEW_GUI
//#define ENABLED_ST_NODEMAP_GUI
#define ENABLED_SCHEDULED_ACTION_COMMAND

// Include files for using StApi.
#include <StApi_TL.h>
#ifdef ENABLED_ST_NODEMAP_GUI
#include <StApi_GUI.h>
#else
#ifdef ENABLED_ST_PREVIEW_GUI
#include <StApi_GUI.h>
#endif
#endif
#include <iomanip>

//Namespace for using StApi.
using namespace StApi;

//Namespace for using GenApi.
using namespace GenApi;

//Namespace for using cout
using namespace std;

const uint32_t m_nDeviceKey = 0x12345678;
const uint32_t m_nGroupKey = 0x00000001;
const uint32_t m_nGroupMask = 0xFFFFFFFF;

#ifdef ENABLED_SCHEDULED_ACTION_COMMAND
CCommandPtr m_ICommand_TimestampLatch;
CIntegerPtr m_IInteger_TimestampLatchValue;
const uint64_t m_nReservationTime = 5000000000;	//[ns]
#endif //ENABLED_SCHEDULED_ACTION_COMMAND

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
std::string formatThousandsSeparators(uint64_t nValue)
{
	std::stringstream  ss;
	std::vector<uint16_t> vecNums;
	do
	{
		vecNums.push_back((uint16_t)(nValue % 1000));
		nValue /= 1000;
	} while (0 < nValue);

	std::vector<uint16_t>::reverse_iterator itr = vecNums.rbegin();
	ss << *itr;
	while(++itr != vecNums.rend())
	{
		ss << "," << std::setfill('0') << std::setw(3) << *itr;
	}
	return std::string(ss.str());
}
//-----------------------------------------------------------------------------
//Adjust GevSCPD (Only for same configuration cameras.)
//-----------------------------------------------------------------------------
void AdjustGevSCPD(CIStDevicePtrArray &pIStDeviceList)
{
	//Get packet size.
	CIntegerPtr pIIntegerGevSCPSPacketSize(pIStDeviceList[0]->GetRemoteIStPort()->GetINodeMap()->GetNode("GevSCPSPacketSize"));
	if (!pIIntegerGevSCPSPacketSize.IsValid()) return;

	CIntegerPtr pIIntegerTimestampLatchValue(pIStDeviceList[0]->GetRemoteIStPort()->GetINodeMap()->GetNode("TimestampLatchValue"));
	if (!pIIntegerTimestampLatchValue.IsValid()) return;

	const int64_t nMaximumFrameSize((pIIntegerGevSCPSPacketSize->GetValue() + 38) * 8);

	const size_t nCount = pIStDeviceList.GetSize();
	const double nEffectiveRate = 0.80;	//80%
	int64_t nTimestampUnit = pIIntegerTimestampLatchValue->GetInc();
	if (nTimestampUnit == 0)
	{
		nTimestampUnit = 40;
	}
	const int64_t nEachPacketTimeNs = (int64_t)(nMaximumFrameSize * (nCount - 1) / nEffectiveRate);
	const int64_t nSCPDValue = nEachPacketTimeNs / nTimestampUnit;

	for (size_t i = 0; i < nCount; ++i)
	{
		cout << "Device" << i << endl;
		CNodeMapPtr pINodeMap(pIStDeviceList[i]->GetRemoteIStPort()->GetINodeMap());
		CIntegerPtr pIntegerGevSCPD(pINodeMap->GetNode("GevSCPD"));
		pIntegerGevSCPD->SetValue(nSCPDValue);
		cout << "  GevSCPD = " << std::dec << nSCPDValue << endl;

		CIntegerPtr pIntegerGevSCFTD(pINodeMap->GetNode("GevSCFTD"));
		if (IsWritable(pIntegerGevSCFTD))
		{
			int64_t nSCFTDValue = 0;
			if (i != 0)
			{
				nSCFTDValue = (int64_t)(nMaximumFrameSize * i / nEffectiveRate);
			}
			pIntegerGevSCFTD->SetValue(nSCFTDValue);
			cout << "  GevSCFTD = " << nSCFTDValue << endl;
		}
	}
}

#ifdef ENABLED_SCHEDULED_ACTION_COMMAND
//-----------------------------------------------------------------------------
//Set device ptp parameters.
//-----------------------------------------------------------------------------
void SetDevicePtpSetting(CIStDevicePtrArray &pIStDeviceList)
{
	size_t nCountOfSupportedPtpCamera = 0;
	const size_t nCount = pIStDeviceList.GetSize();
	for (size_t i = 0; i < nCount; ++i)
	{
		CNodeMapPtr pINodeMap(pIStDeviceList[i]->GetRemoteIStPort()->GetINodeMap());
		CBooleanPtr pIBoolean_PtpEnable(pINodeMap->GetNode("PtpEnable"));
		if (IsWritable(pIBoolean_PtpEnable))
		{
			pIBoolean_PtpEnable->SetValue(true);
			//cout << "  PtpEnable = true" << endl;
			++nCountOfSupportedPtpCamera;
		}
	}

	if (nCountOfSupportedPtpCamera < 2)
	{
		return;
	}

	//Wait for synchronization 
	Sleep(5000);

	bool isSynchronized = false;

	do
	{
		isSynchronized = true;
		for (size_t i = 0; i < nCount; ++i)
		{
			CNodeMapPtr pINodeMap(pIStDeviceList[i]->GetRemoteIStPort()->GetINodeMap());
			CCommandPtr pICommand_PtpDataSetLatch(pINodeMap->GetNode("PtpDataSetLatch"));
			if (IsWritable(pICommand_PtpDataSetLatch))
			{
				pICommand_PtpDataSetLatch->Execute();
				CEnumerationPtr pIEnumeration_PtpStatus(pINodeMap->GetNode("PtpStatus"));
				if (IsReadable(pIEnumeration_PtpStatus))
				{
					CEnumEntryPtr pIEnumEntry_PtpStatus(pIEnumeration_PtpStatus->GetCurrentEntry());
					if (
						(pIEnumEntry_PtpStatus->GetSymbolic().compare("Master") != 0) &&
						(pIEnumEntry_PtpStatus->GetSymbolic().compare("Slave") != 0)
					)
					{
						isSynchronized = false;
					}
					else
					{
						if (!m_ICommand_TimestampLatch.IsValid())
						{
							m_ICommand_TimestampLatch = pINodeMap->GetNode("TimestampLatch");
							m_IInteger_TimestampLatchValue = pINodeMap->GetNode("TimestampLatchValue");
						}
					}
				}
			}
		}
	} while (!isSynchronized);
}
#endif //ENABLED_SCHEDULED_ACTION_COMMAND
//-----------------------------------------------------------------------------
//Set device action command parameters.
//-----------------------------------------------------------------------------
void SetDeviceActionCommandParam(IStDevice *pIStDevice)
{
	CNodeMapPtr pINodeMap(pIStDevice->GetRemoteIStPort()->GetINodeMap());
	CEnumerationPtr pIEnumeration_TriggerSelector(pINodeMap->GetNode("TriggerSelector"));
	CEnumEntryPtr pIEnumEntry_FrameStart(pIEnumeration_TriggerSelector->GetEntryByName("FrameStart"));
	pIEnumeration_TriggerSelector->SetIntValue(pIEnumEntry_FrameStart->GetValue());
	cout << "  TriggerSelector = FrameStart" << endl;

	CEnumerationPtr pIEnumeration_TriggerMode(pINodeMap->GetNode("TriggerMode"));
	CEnumEntryPtr pIEnumEntry_On(pIEnumeration_TriggerMode->GetEntryByName("On"));
	pIEnumeration_TriggerMode->SetIntValue(pIEnumEntry_On->GetValue());
	cout << "  TriggerMode = On" << endl;

	CEnumerationPtr pIEnumeration_TriggerSource(pINodeMap->GetNode("TriggerSource"));
	const char *pszTriggerSourceNames[] = { "Action0", "Action1" };
	for (size_t i = 0; i < _countof(pszTriggerSourceNames); ++i)
	{
		CEnumEntryPtr pIEnumEntry_Action0(pIEnumeration_TriggerSource->GetEntryByName(pszTriggerSourceNames[i]));
		if (pIEnumEntry_Action0.IsValid())
		{
			pIEnumeration_TriggerSource->SetIntValue(pIEnumEntry_Action0->GetValue());
			cout << "  TriggerSource = " << pszTriggerSourceNames[i] << endl;
			break;
		}
	}

	CIntegerPtr pIInteger_ActionDeviceKey(pINodeMap->GetNode("ActionDeviceKey"));
	pIInteger_ActionDeviceKey->SetValue(m_nDeviceKey);
	cout << "  ActionDeviceKey = " << hex << showbase << m_nDeviceKey << endl;

	CIntegerPtr pIInteger_ActionSelector(pINodeMap->GetNode("ActionSelector"));
	pIInteger_ActionSelector->SetValue(pIInteger_ActionSelector->GetMin());
	cout << "  ActionSelector = " << pIInteger_ActionSelector->GetMin() << endl;

	CIntegerPtr pIInteger_ActionGroupKey(pINodeMap->GetNode("ActionGroupKey"));
	pIInteger_ActionGroupKey->SetValue(m_nGroupKey);
	cout << "  ActionGroupKey = " << hex << showbase << m_nGroupKey << endl;

	CIntegerPtr pIInteger_ActionGroupMask(pINodeMap->GetNode("ActionGroupMask"));
	pIInteger_ActionGroupMask->SetValue(m_nGroupMask);
	cout << "  ActionGroupMask = " << hex << showbase << m_nGroupMask << endl;
}
//-----------------------------------------------------------------------------
//Set host scheduled action command parameters.
//-----------------------------------------------------------------------------
void SetHostScheduledActionCommandParam(IStInterface* pIStInterface, bool bScheduledTimeEnable, uint64_t nScheduledTime)
{
	CNodeMapPtr pINodeMap(pIStInterface->GetIStPort()->GetINodeMap());
	CBooleanPtr pIBoolean_ActionScheduledTimeEnable(pINodeMap->GetNode("ActionScheduledTimeEnable"));
	pIBoolean_ActionScheduledTimeEnable->SetValue(bScheduledTimeEnable);
	//cout << "  ActionScheduledTimeEnable = " << bScheduledTimeEnable << endl;

	if (bScheduledTimeEnable)
	{
		CIntegerPtr pIInteger_ActionScheduledTime(pINodeMap->GetNode("ActionScheduledTime"));
		pIInteger_ActionScheduledTime->SetValue(nScheduledTime);
		//cout << "  ActionScheduledTime = " << dec << nScheduledTime << endl;
	}
}

//-----------------------------------------------------------------------------
//Set host action command parameters.
//-----------------------------------------------------------------------------
void SetHostActionCommandParam(IStInterface* pIStInterface)
{
	CNodeMapPtr pINodeMap(pIStInterface->GetIStPort()->GetINodeMap());

	CEnumerationPtr pEnumeration_EventSelector(pINodeMap->GetNode("EventSelector"));
	CEnumerationPtr pEnumeration_EventNotification(pINodeMap->GetNode("EventNotification"));

	const char *pszEventNames[] = {"ActionCommand", "ActionCommandAcknowledge"};

	for (size_t i = 0; i < _countof(pszEventNames); ++i)
	{
		pEnumeration_EventSelector->SetIntValue(pEnumeration_EventSelector->GetEntryByName(pszEventNames[i])->GetValue());
		cout << "  EventSelector = " << pszEventNames[i] << endl;
		pEnumeration_EventNotification->SetIntValue(pEnumeration_EventNotification->GetEntryByName("On")->GetValue());
		cout << "  EventNotification = On" << endl;
	}

	CIntegerPtr pIInteger_ActionDeviceKey(pINodeMap->GetNode("ActionDeviceKey"));
	pIInteger_ActionDeviceKey->SetValue(m_nDeviceKey);
	cout << "  ActionDeviceKey = " << hex << showbase << m_nDeviceKey << endl;

	CIntegerPtr pIInteger_ActionGroupKey(pINodeMap->GetNode("ActionGroupKey"));
	pIInteger_ActionGroupKey->SetValue(m_nGroupKey);
	cout << "  ActionGroupKey = " << hex << showbase << m_nGroupKey << endl;

	CIntegerPtr pIInteger_ActionGroupMask(pINodeMap->GetNode("ActionGroupMask"));
	pIInteger_ActionGroupMask->SetValue(m_nGroupMask);
	cout << "  ActionGroupMask = " << hex << showbase << m_nGroupMask << endl;

	SetHostScheduledActionCommandParam(pIStInterface, false, 0);
}
#ifdef ENABLED_ST_NODEMAP_GUI
#define InterfaceVector_t vector<IStInterface*>
//-----------------------------------------------------------------------------
// GUI for sending action commands.
//-----------------------------------------------------------------------------
void SendActionCommand(InterfaceVector_t &vecInterface)
{
	// Create an NodeMap display window.
	CIStNodeMapDisplayWndPtr pIStNodeMapDisplayWnd(StApi::CreateIStWnd(StWindowType_NodeMapDisplay));

	for (InterfaceVector_t::iterator itr = vecInterface.begin(); itr != vecInterface.end(); ++itr)
	{
		IStInterface *pIStInterface = *itr;
		GenICam::gcstring strInterfaceName(pIStInterface->GetIStInterfaceInfo()->GetDisplayName());

		// Register the node to NodeMap window.
		pIStNodeMapDisplayWnd->RegisterINode((*itr)->GetIStPort()->GetINodeMap()->GetNode("ActionControl"), strInterfaceName);
		pIStNodeMapDisplayWnd->RegisterINode((*itr)->GetIStPort()->GetINodeMap()->GetNode("EventControl"), strInterfaceName);
	}

	// Display the window.
	pIStNodeMapDisplayWnd->Show(NULL, StWindowMode_Modal);
}
#else
#define InterfaceVector_t vector<CActionCommandInterface*>
typedef IStInterface*	ActionCommandCallbackUserParam_t;

//-----------------------------------------------------------------------------
// Class for action commande event.
//-----------------------------------------------------------------------------
class CActionCommandEvent
{
public:
	explicit CActionCommandEvent(IStInterface *pIStInterface) : m_pIStInterface(pIStInterface)
	{
		CNodeMapPtr pINodeMap(pIStInterface->GetIStPort()->GetINodeMap());
		m_pIInteger_EventActionCommand = pINodeMap->GetNode("EventActionCommand");
		m_pIInteger_EventActionCommandRequestID = pINodeMap->GetNode("EventActionCommandRequestID");
		m_objCIStRegisteredCallbackPtr_OnCommandSent = RegisterCallback(m_pIInteger_EventActionCommand->GetNode(), *this, &CActionCommandEvent::OnCommandSent, pIStInterface, cbPostOutsideLock);

		m_pIInteger_EventActionCommandAcknowledge = pINodeMap->GetNode("EventActionCommandAcknowledge");
		m_pIInteger_EventActionCommandAcknowledgeSourceIPAddress = pINodeMap->GetNode("EventActionCommandAcknowledgeSourceIPAddress");
		m_pIEnumeration_EventActionCommandAcknowledgeStatus = pINodeMap->GetNode("EventActionCommandAcknowledgeStatus");
		m_pIInteger_EventActionCommandAcknowledgeAcknowledgeID = pINodeMap->GetNode("EventActionCommandAcknowledgeAcknowledgeID");
		m_objCIStRegisteredCallbackPtr_OnAcknowledgeRcv = RegisterCallback(m_pIInteger_EventActionCommandAcknowledge->GetNode(), *this, &CActionCommandEvent::OnAcknowledgeRcv, pIStInterface, cbPostOutsideLock);

		m_pIInteger_GevActionDestinationIPAddress = pINodeMap->GetNode("GevActionDestinationIPAddress");
	};
	~CActionCommandEvent()
	{
	};
	void OnCommandSent(INode* /*pINode*/, ActionCommandCallbackUserParam_t /*pIStInterface*/)
	{
		stringstream ss;
		ss << "Sent action command[";
		if (IsReadable(m_pIInteger_EventActionCommandRequestID))
		{
			ss << dec << m_pIInteger_EventActionCommandRequestID->GetValue() << "]:";
		}

		if (IsReadable(m_pIInteger_GevActionDestinationIPAddress))
		{
			ss << m_pIInteger_GevActionDestinationIPAddress->ToString();
		}
		ss << endl;
		cout << ss.str();
	};
	void OnAcknowledgeRcv(INode* /*pINode*/, ActionCommandCallbackUserParam_t /*pIStInterface*/)
	{
		stringstream ss;
		ss << "Rcv action command[";
		if (IsReadable(m_pIInteger_EventActionCommandAcknowledgeAcknowledgeID))
		{
			ss << dec << m_pIInteger_EventActionCommandAcknowledgeAcknowledgeID->GetValue() << "]:";
		}
		if (IsReadable(m_pIInteger_EventActionCommandAcknowledgeSourceIPAddress))
		{
			ss << m_pIInteger_EventActionCommandAcknowledgeSourceIPAddress->ToString();
		}
		if (IsReadable(m_pIEnumeration_EventActionCommandAcknowledgeStatus))
		{
			ss << "(" << m_pIEnumeration_EventActionCommandAcknowledgeStatus->GetCurrentEntry()->GetNode()->GetDisplayName() << ")";
		}
		ss << endl;
		cout << ss.str();
	};
protected:
	IStInterface *m_pIStInterface;
	CIntegerPtr m_pIInteger_EventActionCommand;
	CIntegerPtr m_pIInteger_EventActionCommandRequestID;
	StApi::CIStRegisteredCallbackPtr m_objCIStRegisteredCallbackPtr_OnCommandSent;

	CIntegerPtr m_pIInteger_EventActionCommandAcknowledge;
	CIntegerPtr m_pIInteger_EventActionCommandAcknowledgeSourceIPAddress;
	CEnumerationPtr m_pIEnumeration_EventActionCommandAcknowledgeStatus;
	CIntegerPtr m_pIInteger_EventActionCommandAcknowledgeAcknowledgeID;
	StApi::CIStRegisteredCallbackPtr m_objCIStRegisteredCallbackPtr_OnAcknowledgeRcv;

	CIntegerPtr m_pIInteger_GevActionDestinationIPAddress;
};
//-----------------------------------------------------------------------------
// Class for Action Command interface.
//-----------------------------------------------------------------------------
class CActionCommandInterface
{
public:
	explicit CActionCommandInterface(IStInterface *pIStInterface) :
		m_objCActionCommandEvent(pIStInterface), 
		m_pIStInterface(pIStInterface)
	{
		CNodeMapPtr pINodeMap(pIStInterface->GetIStPort()->GetINodeMap());
		m_pICommand_ActionCommand = pINodeMap->GetNode("ActionCommand");
	};
	virtual ~CActionCommandInterface() {};

	IStInterface *GetIStInterface() {	return(m_pIStInterface);	};
	void Execute()
	{
		m_pICommand_ActionCommand->Execute();
	}
protected:
	CActionCommandEvent m_objCActionCommandEvent;
	IStInterface *m_pIStInterface;
	CCommandPtr m_pICommand_ActionCommand;
};

//-----------------------------------------------------------------------------
// Console function for sending action command.
//-----------------------------------------------------------------------------
void SendActionCommand(InterfaceVector_t &vecInterface)
{


	for (;;)
	{
		// Display a choice of setting function
#ifdef ENABLED_SCHEDULED_ACTION_COMMAND
		cout << "Scheduled Action Command reserves the trigger after " << (m_nReservationTime / 1000000000) << " seconds." << endl;
		cout << "To improve the timing accuracy of Scheduled Action Commands, " << endl;
		cout << "you must use a switch that conforms to the IEEE 1588 standard." << endl;
		cout << "Input (0:Action Command, 2:Scheduled Action Command, 1:Exit) : ";
#else
		cout << "Input (0:Action Command, 1:Exit) : ";
#endif //ENABLED_SCHEDULED_ACTION_COMMAND

		size_t nInput = 0;	//1:exit

		// Waiting for input.
		cin >> nInput;
		while (cin.get() != '\n');
#ifdef ENABLED_SCHEDULED_ACTION_COMMAND
		if ((nInput == 0) || (nInput == 2))
#else
		if (nInput == 0)
#endif //ENABLED_SCHEDULED_ACTION_COMMAND
		{
#ifdef ENABLED_SCHEDULED_ACTION_COMMAND
			uint64_t nTimestamp = 0;
			const bool bScheduledTimeEnable = (nInput == 2);
			if (bScheduledTimeEnable)
			{
				if (IsWritable(m_ICommand_TimestampLatch))
				{
					m_ICommand_TimestampLatch->Execute();
					if (IsReadable(m_IInteger_TimestampLatchValue))
					{
						nTimestamp = m_IInteger_TimestampLatchValue->GetValue();
						nTimestamp += m_nReservationTime;
						cout << "Scheduled Time (Frame start trigger) =" << formatThousandsSeparators(nTimestamp) << "[ns]" << endl;
					}
				}
			}
			for (InterfaceVector_t::iterator itr = vecInterface.begin(); itr != vecInterface.end(); ++itr)
			{
				SetHostScheduledActionCommandParam((*itr)->GetIStInterface(), bScheduledTimeEnable, nTimestamp);
			}
#endif //ENABLED_SCHEDULED_ACTION_COMMAND
			//Send action command.
			for (InterfaceVector_t::iterator itr = vecInterface.begin(); itr != vecInterface.end(); ++itr)
			{
				CActionCommandInterface *pCActionCommandInterface = *itr;
				pCActionCommandInterface->Execute();
			}
#ifdef ENABLED_SCHEDULED_ACTION_COMMAND
			if (bScheduledTimeEnable)
			{
				Sleep(m_nReservationTime / 1000000);
			}
#endif //ENABLED_SCHEDULED_ACTION_COMMAND
			Sleep(200);
		}
		else if (nInput == 1)
		{
			break;
		}
	}
}
#endif //ENABLED_ST_NODEMAP_GUI

#ifdef ENABLED_ST_PREVIEW_GUI
typedef IStImageDisplayWnd* UserParam_t;
#else
typedef void*	UserParam_t;
#endif


//-----------------------------------------------------------------------------
// Function for handling callback action
//-----------------------------------------------------------------------------
void __stdcall OnStCallbackCFunction(IStCallbackParamBase *pIStCallbackParamBase, UserParam_t pvContext)
{
	// Check callback type.
	// We only handle NewBuffer event in here.
	if (pIStCallbackParamBase->GetCallbackType() == StCallbackType_GenTLEvent_DataStreamNewBuffer)
	{
		// In case of receiving a NewBuffer events:
		// Convert received callback parameter into IStCallbackParamGenTLEventNewBuffer for acquiring additional information.
		IStCallbackParamGenTLEventNewBuffer *pIStCallbackParamGenTLEventNewBuffer = dynamic_cast<IStCallbackParamGenTLEventNewBuffer*>(pIStCallbackParamBase);

		try
		{
			// Get the IStDataStream interface pointer from the received callback parameter.
			IStDataStream *pIStDataStream = pIStCallbackParamGenTLEventNewBuffer->GetIStDataStream();

			// Retrieve the buffer pointer of image data for that callback indicated there is a buffer received.
			CIStStreamBufferPtr pIStStreamBuffer(pIStDataStream->RetrieveBuffer(0));

			// Check if the acquired data contains image data.
			if (pIStStreamBuffer->GetIStStreamBufferInfo()->IsImagePresent())
			{

				// If yes, we create a IStImage object for further image handling.
				IStImage *pIStImage = pIStStreamBuffer->GetIStImage();

#ifdef ENABLED_ST_PREVIEW_GUI
				// Check if display window is visible.
				if (!pvContext->IsVisible())
				{
					// Set the position and size of the window.
					pvContext->SetPosition(0, 0, static_cast<int32_t>(pIStImage->GetImageWidth()), static_cast<int32_t>(pIStImage->GetImageHeight()));

					// Create a new thread to display the window.
					pvContext->Show(NULL, StWindowMode_ModalessOnNewThread);
				}

				// Register the image to be displayed.
				// This will have a copy of the image data and original buffer can be released if necessary and original buffer can be released if necessary.
				pvContext->RegisterIStImage(pIStImage);
#endif
				// Display the information of the acquired image data.
				GenApi::CIntegerPtr pIInteger_GevDeviceIPAddress(pIStDataStream->GetIStDevice()->GetLocalIStPort()->GetINodeMap()->GetNode("GevDeviceIPAddress"));
				stringstream ss;
				ss
					<< "IPAddress=" << pIInteger_GevDeviceIPAddress->ToString()
					<< " BlockId=" << pIStStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()
					<< " Size:" << pIStImage->GetImageWidth() << " x " << pIStImage->GetImageHeight()
					<< " Timestamp (Exposure End)=" << formatThousandsSeparators(pIStStreamBuffer->GetIStStreamBufferInfo()->GetTimestampNS()) << "[ns]" << endl;
				cout << ss.str();
			}
			else
			{
				// If the acquired data contains no image data.
				cout << "Image data does not exist." << endl;
			}
		}
		catch (const GenICam::GenericException &e)
		{
			// If any exception occurred, display the description of the error here.
			cerr << endl << "An exception occurred." << endl << e.GetDescription() << endl;
		}
	}
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int main(int /* argc */, char ** /* argv */)
{
	InterfaceVector_t vecInterface;

	try
	{
		// Initialize StApi before using.
		CStApiAutoInit objStApiAutoInit;

		// Create a system object for device scan and connection.
		CIStSystemPtr pIStSystem(CreateIStSystem(StSystemVendor_Default, StInterfaceType_GigEVision));

		// Check GigE interface for devices.
		// If there is no camera, throw exception.
		for (size_t i = 0; i < pIStSystem->GetInterfaceCount(); i++)
		{
			IStInterface *pIStInterface = pIStSystem->GetIStInterface(i);

			try
			{
				// Displays the DisplayName of the interface.
				GenApi::CIntegerPtr pIInteger_GevInterfaceSubnetIPAddress(pIStInterface->GetIStPort()->GetINodeMap()->GetNode("GevInterfaceSubnetIPAddress"));
				cout << "Interface" << dec << i << "=" << pIStInterface->GetIStInterfaceInfo()->GetDisplayName() << "[" << pIInteger_GevInterfaceSubnetIPAddress->ToString() << "]" << endl;

				SetHostActionCommandParam(pIStInterface);

#ifdef ENABLED_ST_NODEMAP_GUI
				vecInterface.push_back(pIStInterface);
#else
				vecInterface.push_back(new CActionCommandInterface(pIStInterface));
#endif
				//Start Event Acquisition Thread
				pIStInterface->StartEventAcquisitionThread();
			}
			catch (const GenICam::GenericException &e)
			{
				// Display a description of the error.

				cerr << endl << "An exception occurred." << endl << e.GetDescription() << endl;
			}
		}
		if (vecInterface.empty())
		{
			throw RUNTIME_EXCEPTION("There is no interface.");
		}

		// Create a camera device list object to store all the cameras.
		CIStDevicePtrArray pIStDeviceList;

#ifdef ENABLED_ST_PREVIEW_GUI
		// If using GUI for display, create a display window here.
		CIStImageDisplayWndPtrArray pIStWndList;
#endif

		// Create a DataStream list object to store all the data stream object related to the cameras.
		CIStDataStreamPtrArray pIStDataStreamList;

		// Here we try to connect to all possible device with a do-while loop.
		for (;;)
		{
			IStDeviceReleasable *pIStDeviceReleasable = NULL;

			try
			{
				// Create a camera device object and connect to first detected device.
				pIStDeviceReleasable = pIStSystem->CreateFirstIStDevice();
			}
			catch (...)
			{
				break;
			}

			// Add the camera into device object list for later usage.
			pIStDeviceList.Register(pIStDeviceReleasable);

			// Displays the DisplayName of the device.
			GenApi::CIntegerPtr pIInteger_GevDeviceIPAddress(pIStDeviceReleasable->GetLocalIStPort()->GetINodeMap()->GetNode("GevDeviceIPAddress"));
			cout << "Device" << dec << pIStDeviceList.GetSize() << "=" << pIStDeviceReleasable->GetIStDeviceInfo()->GetDisplayName() << "[" << pIInteger_GevDeviceIPAddress->ToString() << "]" << endl;


			//Set action command parameter
			try
			{
				SetDeviceActionCommandParam(pIStDeviceReleasable);
			}
			catch (const GenICam::GenericException &e)
			{
				// Display a description of the error.

				cerr << endl << "An exception occurred." << endl << e.GetDescription() << endl;
			}


			// Create a DataStream object for handling image stream data then add into DataStream list for later usage.
			pIStDataStreamList.Register(pIStDeviceReleasable->CreateIStDataStream(0));

			IStDataStream *pIStDataStream = pIStDataStreamList[pIStDataStreamList.GetSize() - 1];

#ifdef ENABLED_ST_PREVIEW_GUI
			// Create an image display window object, to get the IStWndReleasable interface pointer.
			pIStWndList.Register(CreateIStWnd(StWindowType_ImageDisplay));
			IStImageDisplayWnd *pIStImageDisplayWnd = pIStWndList[pIStWndList.GetSize() - 1];
			RegisterCallback(pIStDataStream, &OnStCallbackCFunction, (UserParam_t)pIStImageDisplayWnd);
#else
			RegisterCallback(pIStDataStream, &OnStCallbackCFunction, (UserParam_t)NULL);
#endif
		}

		if (pIStDeviceList.GetSize() != 0)
		{
#ifdef ENABLED_SCHEDULED_ACTION_COMMAND
			SetDevicePtpSetting(pIStDeviceList);
#endif //ENABLED_SCHEDULED_ACTION_COMMAND

			// Start the image acquisition of the host side.
			pIStDataStreamList.StartAcquisition();

			// Start the image acquisition of the camera side.
			pIStDeviceList.AcquisitionStart();

			// Adjust GevSCPD;
			AdjustGevSCPD(pIStDeviceList);

			SendActionCommand(vecInterface);


			// Stop the image acquisition of the camera side.
			pIStDeviceList.AcquisitionStop();

			// Stop the image acquisition of the host side.
			pIStDataStreamList.StopAcquisition();
		}

		for(InterfaceVector_t::iterator itr = vecInterface.begin(); itr != vecInterface.end(); ++itr)
		{
#ifdef ENABLED_ST_NODEMAP_GUI
			//Stop Event Acquisition Thread
			(*itr)->StopEventAcquisitionThread();
#else	//ENABLED_ST_NODEMAP_GUI
			CActionCommandInterface *pCActionCommandInterface(*itr);

			//Stop Event Acquisition Thread
			pCActionCommandInterface->GetIStInterface()->StopEventAcquisitionThread();

			delete pCActionCommandInterface;
#endif	//ENABLED_ST_NODEMAP_GUI
		}
		vecInterface.clear();
	}
    catch (const GenICam::GenericException &e)
    {
		// Display a description of the error.
		cerr << endl << "An exception occurred." << endl << e.GetDescription() << endl;
    }

	return(0);
}
