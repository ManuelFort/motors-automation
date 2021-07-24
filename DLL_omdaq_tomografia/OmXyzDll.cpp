// ---------------------------------------------------------------------------
//
// This file contains dummy procedure bodies as examples and for testing.
//
// The code in these procedures simulates an XYZ-2R stage
//
//
// ---------------------------------------------------------------------------
#pragma hdrstop
#include <math.h>
#include <time.h>

#define XYZDLL_EXPORTS 1
#include "OmXyzDll.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)

// ______Local variables for testing____________________________
//
double CurrentDllPosition[3];
double CurrentDllAngle[3];
double DemandPosition[3];
double DemandAngle[3];
double PosStep[3];
double AngleStep[3];
double LinSpeed[3];
double RotSpeed[3];
clock_t tLin;
clock_t tRot;
bool DllPowerOn;
#define nOptions 2
char OptionText[nOptions][32];
bool optionsCopied = false;
//
// _____________________________________________________________

// Adminstration routines ++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// XyzCapabilityMask returns a DWORD mask that describes the basic functionality
// of the hardware and allows OMDAQ to make the user interface.
// The return value is assembled from the capability constants
// defined in OmXyzDll_StatusBits.h
// >>>>>> THIS MUST BE DEFINED <<<<<<<
XYZ_DLL DWORD _CALLSTYLE_ XyzCapabilityMask() {
  return (XYZCAP_XYZ3 | XYZCAP_ROT3 | XYZCAP_POWER_ONOFF | XYZCAP_TEMPSENSOR);
  // | XYZCAP_TEMPSENSOR);
  // This defines an XYZ + 3R stage with Power Off capability, (temperature sensors) and
  // no home switch
}

//
// XyzDllVersion returns the version numbers of the DLL file.
XYZ_DLL bool _CALLSTYLE_ XyzDllVersion(int * majorVersion, int * minorVersion,
	int * buildNumber) {
  *majorVersion = 1;
  *minorVersion = 0;
  *buildNumber = 12;
  return true;
}

//
// XyzDescription fills a char string that describes the XYZ stage
// nChar is the length of the supplied buffer (typically 80 characters)
//
XYZ_DLL bool _CALLSTYLE_ XyzDescription(char *statusText, int nChar) {
  strncpy(statusText, "XYZ stage controlled by user-supplied DLL", nChar);
  return true;
}

// XyzHwDescription fills a char string that decsribes the current setup
// (COM ports, card slot numbers etc.)
// nChar is the length of the supplied buffer. (typically 80 characters)
//
XYZ_DLL bool _CALLSTYLE_ XyzHwDescription(char *statusText, int nChar) {
  strncpy(statusText, "COM45 9600baud", nChar);
  return true;
}

//
// XyzAuthor returns the author credits and copyrights etc.
// nChar is the length of the supplied buffer.  (typically 80 characters)
XYZ_DLL bool _CALLSTYLE_ XyzAuthor(char *statusText, int nChar) {
  strncpy(statusText,
	  "DLL written by A. Coder (c), ACME Software Inc., 2020", nChar);
  return true;
}

//
// XyzDisplayDecimals returns the number of decimal places to disply inthe readouts
// of position in millimetres and angle in degreees (the same value is used for both
// displays.
// >>>>>>>>>>>>>  NOTE:  This is no longer used.  OMDAQ sets the display resolution
// XYZ_DLL int _CALLSTYLE_ XyzDisplayDecimals() {
// return 3;
// }
// -----------------------------------------------------

// ---------Procedures for optional parameters -----------------------------------
// Any DLL parameters that neeed to be specified at runtime can be passed as
// strings to the XyzInitialise(...) procedure through the options argument.
// In order to manage the user interface OMDAQ needs to know the number of
// and a description of each one.  These are obtained using the XyzOptionCount
// and XyzOptionHeader procedures. XyzOptionValue returns the current value
// of an option. This is used primarily BEFORE XyzInitialise() is called to
// provide sensible starting values for the parameters to assist the user in
// setting up a new stage.
//
XYZ_DLL int _CALLSTYLE_ XyzOptionCount() {
  return nOptions;
}

// These allow the DLL to get the parameter filename and the DDL folder from OMDAQ
XYZ_DLL bool _CALLSTYLE_ XyzSetParameterFileName(wchar_t *cText, int nChar) {
  // IniFile = UnicodeString(&cText[0]);
  return true;
}

XYZ_DLL bool _CALLSTYLE_ XyzSetDLLfolder(wchar_t *statusText, int nChar) {
  return true;
}

//
// XyzOptionHeader returns a SHORT description of the optional parameter nHdr.  This is used in the
// user interface for setting up the stage BEFORE the initialisation routine is called.
// Should return false if nHdr is out of range.
XYZ_DLL bool _CALLSTYLE_ XyzOptionHeader(int nHdr, char * optionsHdr,
	int szOptionsHdr) {
  bool ok = false;
  char * initHdrs[nOptions] = {"COM", "Baud"}; // For example...
  if ((nHdr >= 0) && (nHdr < nOptions)) {
	strncpy(optionsHdr, initHdrs[nHdr], szOptionsHdr);
	ok = true;
  }
  return ok;
}

//
// XyzOptionValue returns the current value of an option.  This is used primarily BEFORE
// XyzInitialise() is called to provide sensible starting values for the parameters
// to assist the user in setting up a new stage.
// Should return false if nHdr is out of range.
XYZ_DLL bool _CALLSTYLE_ XyzOptionValue(int nHdr, char * optionVal,
	int szOptionVal) {
  bool ok = false;
  char * initVals[nOptions] = {"COM4", "9600"}; // For example...
  if ((nHdr >= 0) && (nHdr < nOptions)) {
	if (!optionsCopied) {
	  strncpy(&OptionText[nHdr][0], initVals[nHdr], 32*sizeof(char));
	}
	strncpy(optionVal, &OptionText[nHdr][0], szOptionVal);
	ok = true;
  }
  return ok;
}
//
// End of administration routines ++++++++++++++++++++++++++++++++++++++++++++

// Initialisation routines +++++++++++++++++++++++++++++++++++++++++++++++++++

// ---------------------------------------------------------------------------
XYZ_DLL bool _CALLSTYLE_ XyzInitialise(char **options, int szOptions) {
  // Initialisation code here
  // This should include e.g.:  allocation of resources, setting up comms link,
  // starting the controller, finding the home marker, setting up any hardware parameters
  // such as motor current, motor steps and scaling, encoder steps and scaling, etc.
  // Speed and Acceleration are set by OMDAQ.
  //
  // If the stages have home markers, OMDAQ will move the stage to the position at last shutdown,
  // othewise, OMDAQ defines the stage position after initialisation to be the position at last shutdown.
  // Return false if it fails (IMPORTANT!!)
  //
  // If the stage needs optional parameters which can be set up at runtime (e.g. COM port
  // number, bauds, etc.) then these can be passed in as character strings in the options arguments.
  // These are set by the user in the "Miscellaneous" tab of the XYZ setup dialog

  if (szOptions != nOptions) {
	return false;
  }

  for (int i = 0; i < szOptions; ++i) {
	ZeroMemory(&OptionText[i][0], 32*nOptions*sizeof(char));
	strcpy(&OptionText[i][0], options[i]);
  }
  optionsCopied = true;

  for (int i = 0; i < 3; ++i) {
	CurrentDllPosition[i] = 0;
	CurrentDllAngle[i] = 0;
  }
  DllPowerOn = true;
  return true;
}

// ---------------------------------------------------------------------------
XYZ_DLL bool _CALLSTYLE_ XyzShutDown() {
  // Full shutdown code here  - stop stage if it's moving,
  // power down, free comms links and free resources.
  //
  // OMDAQ saves the position at shutdown ready for the next startup.
  // return false if it fails.
  return true;
}

// --------------------------------------------------------------------------
// These procedures initialise the values of the position or angle readouts to the supplied values.
// NewPosition and NewAngle are pointers to double[3] arrays which contain on entry the
// new values of the absolute postions (mm) or angles (deg) for axes 0..2
// Is not required for stages with hardware zero markers, in which case just return true.
XYZ_DLL bool _CALLSTYLE_ XyzSetCurrentPosition(double * NewPosition) {
  for (int i = 0; i < 3; ++i) {
	CurrentDllPosition[i] = NewPosition[i];
	DemandPosition[i] = NewPosition[i];
	PosStep[i] = 0;
  }
  return true;
}

XYZ_DLL bool _CALLSTYLE_ XyzSetCurrentAngle(double * NewAngle) {
  for (int i = 0; i < 3; ++i) {
	CurrentDllAngle[i] = NewAngle[i];
	DemandAngle[i] = NewAngle[i];
	AngleStep[i] = 0;
  }
  return true;
}
//
// End of initialisation ++++++++++++++++++++++++++++++++++++++++++++++++++++

// Motion parameters +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// These set the LINEAR speed and acceleration per axis (assumed to be the same in the
// accel and decel phases.  Units are  mm/sec and mm/sec2
// NewAccel and NewSpeed are pointers to double[3] arrays containing the new values for each axis.
// At present OMDAQ only allows a single accel value for all axes.
XYZ_DLL bool _CALLSTYLE_ XyzSetAccel(double * NewAccel) {
  return true;
}

XYZ_DLL bool _CALLSTYLE_ XyzSetSpeed(double * NewSpeed) {
  for (int i = 0; i < 3; ++i) {
	LinSpeed[i] = NewSpeed[i];
  }
  return true;
}

// These set the ROTATIONAL speed and acceleration per axis (assumed to be the same in the
// accel and decel phases.  Units are  deg/sec and deg/sec2
// NewAccel and NewSpeed are pointers to double[3] arrays containing the new values for each axis.
XYZ_DLL bool _CALLSTYLE_ XyzSetRotAccel(double * NewAccel) {
  // This comment is no longer valid:  OMDAQ DOES set rotary acceleration.
  ///*  At present OMDAQ does not define rotational acceleration, so this call is not used.
  // This must be set up during initialisation */
  //
  return true;
}

XYZ_DLL bool _CALLSTYLE_ XyzSetRotSpeed(double * NewSpeed) {
  for (int i = 0; i < 3; ++i) {
	RotSpeed[i] = NewSpeed[i];
  }
  return true;
}

// Power On-off.  Turns the power to all axes ON (Enabled = true) or OFF (Enabled = false)
// Leaves the controller active and reporting.
// returns true for success.
XYZ_DLL bool _CALLSTYLE_ XyzPowerOn(bool Enabled) {
  DllPowerOn = Enabled;
  return true;
}
// End of motion parameters +++++++++++++++++++++++++++++++++++++++++++++++++++

// Motion commands. ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Move the position or angle to the absolute values supplied in the arguments.
// Arguments are pointers to double[3] containing the new values.
// The routines are expected to return immediately - waiting for position is handled by OMDAQ
//
XYZ_DLL bool _CALLSTYLE_ XyzMoveToPosition(double * NewPosition) {
  for (int i = 0; i < 3; ++i) {
	DemandPosition[i] = NewPosition[i];
	PosStep[i] = (NewPosition[i] > CurrentDllPosition[i]) ? 1 : -1;
  }
  tLin = clock();
  return true;
}

XYZ_DLL bool _CALLSTYLE_ XyzMoveToAngle(double * NewAngle) {
  for (int i = 0; i < 3; ++i) {
	DemandAngle[i] = NewAngle[i];
	AngleStep[i] = (NewAngle[i] > CurrentDllAngle[i]) ? 1 : -1;
  }
  tRot = clock();
  return true;
}

// XyzStop performs an immediate halt (emergency stop, so no deceleration) on all axes
XYZ_DLL bool _CALLSTYLE_ XyzHalt() {
  DllPowerOn = false;
  for (int i = 0; i < 3; ++i) {
	PosStep[i] = 0;
	AngleStep[i] = 0;
	DemandPosition[i] = CurrentDllPosition[i];
	DemandAngle[i] = CurrentDllAngle[i];
  }
  return true;
}
//
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Status reporting +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// GetPosition and GetAngle read back the current values into the arguments, which are pointers
// to double[3].
XYZ_DLL bool _CALLSTYLE_ XyzGetPosition(double * CurrentPosition) {
  clock_t tNow = clock();
  for (int i = 0; i < 3; ++i) {
	if (PosStep[i] != 0) {
	  CurrentDllPosition[i] += PosStep[i] * (tNow - tLin) * 0.001 * LinSpeed[i];
	  if (PosStep[i] > 0) {
		if (CurrentDllPosition[i] >= DemandPosition[i]) {
		  CurrentDllPosition[i] = DemandPosition[i];
		  PosStep[i] = 0;
		}
	  }
	  else {
		if (CurrentDllPosition[i] <= DemandPosition[i]) {
		  CurrentDllPosition[i] = DemandPosition[i];
		  PosStep[i] = 0;
		}
	  }
	}
	CurrentPosition[i] = CurrentDllPosition[i];
  }
  tLin = tNow;
  return true;
}

XYZ_DLL bool _CALLSTYLE_ XyzGetAngle(double * CurrentAngle) {
  clock_t tNow = clock();
  for (int i = 0; i < 3; ++i) {
	if (AngleStep[i] != 0) {
	  CurrentDllAngle[i] += AngleStep[i] * (tNow - tRot) * 0.001 * RotSpeed[i];
	  if (AngleStep[i] > 0) {
		if (CurrentDllAngle[i] >= DemandAngle[i]) {
		  CurrentDllAngle[i] = DemandAngle[i];
		  AngleStep[i] = 0;
		}
	  }
	  else {
		if (CurrentDllAngle[i] <= DemandAngle[i]) {
		  CurrentDllAngle[i] = DemandAngle[i];
		  AngleStep[i] = 0;
		}
	  }
	}
	CurrentAngle[i] = CurrentDllAngle[i];
  }
  tRot = tNow;
  return true;
}

//
// GetMotorTemp returns the temperature in degrees of all axes.
// MotorTemp is a pointer to a double array
// if iAxis = -1 this it's an array big enough to hold all motor temps.
// If iAxis >= 0 the temp of iAxis is put into th efirst element of the array.
XYZ_DLL bool _CALLSTYLE_ XyzGetMotorTemp(double *MotorTemp, int iAxis) {
  int iMin = 0;
  int iMax = 6;
  if (iAxis >= 0) {
	iMin = iAxis;
	iMax = iAxis;
  }
  for (int i = iMin; i < iMax; ++i) {
	int iDest = 0;
	if (iAxis >= 0) {
	  iDest = i;
	}
	MotorTemp[iDest] = 25 + 0.01 * random(500);
  }
}
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Status and Error handling commands.  *************************************
//
// StageStatus returns the DRVSTAT (UINT64) status mask built
// from the mask constants defined in OmXyzDll_StatusBits.h.  Optionally the program may ask
// for more details in the AxisStatus DWORDs by passing a non-NULL pointer to
// AxisiStstus.  This is DWORD[3] or DWORD[6] depending on the capabilities of the stage.
// if iAxis = -1 this it's an array big enough to hold all axis status.
// If iAxis >= 0 the temp of iAxis is put into the first element of the array.
// Note that for single axis calls only the single axis segments of status are filled
// so this must be managed in th ecalling program,
XYZ_DLL DRVSTAT _CALLSTYLE_ XyzStageStatus(DWORD * AxisStatus) {
  return XyzAxisStatus(-1, AxisStatus);
}

XYZ_DLL DRVSTAT _CALLSTYLE_ XyzAxisStatus(int iAxis, DWORD * AxisStatus) {
  DRVSTAT status = 0;
  int iMin = 0;
  int iMax = 6;
  if (iAxis >= 0) {
	iMin = iAxis;
	iMax = iAxis;
  }
  for (int i = iMin; i < iMax; ++i) {
	if (i < 3) {
	  if (fabs(CurrentDllPosition[i] - DemandPosition[i]) > 0.001) {
		switch (i) {
		case 0:
		  status = status | ST_AX1_MOVING;
		  break;
		case 1:
		  status = status | ST_AX2_MOVING;
		  break;
		case 2:
		  status = status | ST_AX3_MOVING;
		  break;
		}
	  }
	  else {
		switch (i) {
		case 0:
		  status = status | ST_AX1_INPOSITION;
		  break;
		case 1:
		  status = status | ST_AX2_INPOSITION;
		  break;
		case 2:
		  status = status | ST_AX3_INPOSITION;
		  break;
		}
	  }

	  if (CurrentDllPosition[i] < -20.0) {
		switch (i) {
		case 0:
		  status = status | ST_AX1_NEGLIM;
		  break;
		case 1:
		  status = status | ST_AX2_NEGLIM;
		  break;
		case 2:
		  status = status | ST_AX3_NEGLIM;
		  break;
		}
	  }

	  if (CurrentDllPosition[i] > 20.0) {
		switch (i) {
		case 0:
		  status = status | ST_AX1_POSLIM;
		  break;
		case 1:
		  status = status | ST_AX2_POSLIM;
		  break;
		case 2:
		  status = status | ST_AX3_POSLIM;
		  break;
		}
	  }
	}
	else {
	  if (fabs(CurrentDllAngle[i - 3] - DemandAngle[i - 3]) > 0.001) {
		switch (i - 3) {
		case 0:
		  status = status | ST_RO1_MOVING;
		  break;
		case 1:
		  status = status | ST_RO2_MOVING;
		  break;
		case 2:
		  status = status | ST_RO3_MOVING;
		  break;
		}
	  }
	  else {
		switch (i - 3) {
		case 0:
		  status = status | ST_RO1_INPOSITION;
		  break;
		case 1:
		  status = status | ST_RO2_INPOSITION;
		  break;
		case 2:
		  status = status | ST_RO3_INPOSITION;
		  break;
		}
	  }

	  if (CurrentDllAngle[i - 3] < -90.0) {
		switch (i - 3) {
		case 0:
		  status = status | ST_RO1_NEGLIM;
		  break;
		case 1:
		  status = status | ST_RO2_NEGLIM;
		  break;
		case 2:
		  status = status | ST_RO3_NEGLIM;
		  break;
		}
	  }

	  if (CurrentDllAngle[i - 3] > 90.0) {
		switch (i - 3) {
		case 0:
		  status = status | ST_RO1_POSLIM;
		  break;
		case 1:
		  status = status | ST_RO2_POSLIM;
		  break;
		case 2:
		  status = status | ST_RO3_POSLIM;
		  break;
		}
	  }
	}
  }

  if (DllPowerOn) {
	status |= (ST_ALL_XYZ_MOTORS_ON | ST_ALL_R3_MOTORS_ON);
  }
  return status;
}

//
// XyzFaultAck is called after StageStatus reports a fault - defined as a POSLIM, NEGLIM or HWFAULT
// on any axis.  This should be used to clear faults (e.g. backing off from limit switches).
// Return values have the followinhg meanings:
// XyzFltAckOK    0    // Fault has been cleared OK (as far as I can tell)
// XyzFltAckFatal 1    // Fault cannot be cleared and the stage is dead
// (in which case OMDAQ will try to do a tidy shutdown)
// XyzFtlAckRetry 2    // I may be able to clear the fault if you try again,
XYZ_DLL int _CALLSTYLE_ XyzFaultAck() {
  // Resets the limits in one go
  for (int i = 0; i < 3; ++i) {
	if (CurrentDllPosition[i] < -20) {
	  CurrentDllPosition[i] = DemandPosition[i] = -19.99;
	}
	if (CurrentDllPosition[i] > 20) {
	  CurrentDllPosition[i] = DemandPosition[i] = 19.99;
	}
	if (CurrentDllAngle[i] < -90) {
	  CurrentDllAngle[i] = DemandAngle[i] = -89.99;
	}
	if (CurrentDllAngle[i] > 90) {
	  CurrentDllAngle[i] = DemandAngle[i] = 89.99;
	}
  }
  return XyzFltAckOK;
}

//
// This call returns a text description of the last HWFAULT encountered
// The existence of a fault must be signalled in the StageStatus flag mask.
// nChar is the length of the supplied buffer.    (typically 80 characters)
//
// return true for success.
XYZ_DLL bool _CALLSTYLE_ XyzLastFaultText(char *statusText, int nChar) {
  strcpy(statusText, "Fault?  What fault?");
  return true;
}
//
// *************************************************************************
