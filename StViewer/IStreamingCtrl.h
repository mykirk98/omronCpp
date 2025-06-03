#pragma once

#include <windows.h>
#include <stdint.h>

STAPI_INTERFACE IStreamingCtrl
{
	/*!
	Return true if image acquisition is running.
	*/
	virtual bool IsAcquisitionRunning() const = 0;

	/*!
	Start image acquisition.
	*/
	virtual void StartImageAcquisition() = 0;

	/*!
	Stop image acquisition.
	*/
	virtual void StopImageAcquisition() = 0;
};