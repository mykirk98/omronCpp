#pragma once

STAPI_INTERFACE IStImageCallback
{
	virtual void OnIStImage(StApi::IStImage *) = 0;
};

STAPI_INTERFACE IStImageCallbackRegister
{
	virtual void Add(IStImageCallback *pIStImageCallback) = 0;
	virtual void Remove(IStImageCallback *pIStImageCallback) = 0;
};


class CIStImageCallbackList : public IStImageCallback, public IStImageCallbackRegister
{
public:
	CIStImageCallbackList()
	{
	}
	virtual ~CIStImageCallbackList()
	{

	}

	void Add(IStImageCallback *pIStImageCallback) override
	{
		GenApi::AutoLock objAuto(m_objLock);
		m_vecCallbackList.push_back(pIStImageCallback);
	}

	void Remove(IStImageCallback *pIStImageCallback) override
	{
		GenApi::AutoLock objAuto(m_objLock);
		for (std::vector<IStImageCallback*>::iterator itr = m_vecCallbackList.begin(); itr != m_vecCallbackList.end(); ++itr)
		{
			if (*itr == pIStImageCallback)
			{
				m_vecCallbackList.erase(itr);
				break;
			}
		}
	}

	void OnIStImage(StApi::IStImage *pIStImage) override
	{
		GenApi::AutoLock objAuto(m_objLock);
		for (std::vector<IStImageCallback*>::iterator itr = m_vecCallbackList.begin(); itr != m_vecCallbackList.end(); ++itr)
		{
			(*itr)->OnIStImage(pIStImage);
		}
	}

protected:
	GenApi::CLock m_objLock;
	std::vector<IStImageCallback*> m_vecCallbackList;
};







