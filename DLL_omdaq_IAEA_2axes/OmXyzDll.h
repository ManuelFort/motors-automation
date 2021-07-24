///--------------------------------------------------------------------------
// OMXYZDLL.H
// Declarations of functions exported from OMXYZDLL.DLL.
//
// >>>>>>>>>>>>>> This file must not be changed <<<<<<<<<<<<<<<<<<<<<<<<<<<<
// ---------------------------------------------------------------------------
// Revision History
// ================
// 5th June 2013:   beta version.
// 7th June 2013:   trapped the extern "C" so that it only applies to C++ compilers
// 2nd June 2016:   added a character string argument to Initialise to pass in optional parameters
// added the "optionalDataHeader"
// 5th Feb 2019:	  Added the _CALLSTYLE_ define to define the calling convention used by
// the DLL. This is because other compilers may not recognise the
// __fastcall used by VCL
// 18th April 2019:  Removed the XyzDisplayDecimals routine.  This was overwriting the value
// managed by OMDAQ.
// 18th April 2019:  Removed the comment in XyzSetRotAccel.  This call IS now used by OMDAQ
//
// 12th June 2019:   Added gfalgs and code to respond to motior temperature sensors and
// over-temperatuire faults.
// Flags added:   XYZCAP_TEMPSENSOR
// ST_AX1_OVERTEMP  (and AX2, AX3)
// ST_RO1_OVERTEMP  (and RO2, RO3)
// Calls added:   XYZ_DLL bool _CALLSTYLE_ XyzGetMotorTemp (double *MotorTemp)
//
// 20th August 2019:  Modified getMotorTemp to allow single axis reading
// XYZ_DLL bool _CALLSTYLE_ XyzGetMotorTemp (double *MotorTemp, int iAxis)
// Did the same thing for XyzStageStatus (to help with
// Steprocker stages which give up their information in
// single calls.
// XYZ_DLL DRVSTAT _CALLSTYLE_ XyzStageStatus(iAxis = -1,
// DWORD * AxisStatus = NULL);
// The original version still works and calls this with iAxis =-1.
//
// -----------------------------------------------------------------------------
//
// Virtually all calls return a boolean - true for success, false for failure
// Any routines that are not relevant to the hardware must be defined,
// but should just return true.
//
// Position units are mm.
// Angle units are degrees.
// Velocity and acceleration are per second and per second^2
//
// -------------------------------------------------------------------------------

//
// The define  XYZDLL_EXPORTS must be declared in the file creating the
// DLL routines before this file is included.
//
// This sets the correct definition (import or export) for the procedure definitions.
//
// The procedures are declared with extern "C" to avoid name mangling in the
// library files.
// OMDAQ uses Embarcadero C++ Builder 2010 as the devlopment platform.
//

#ifndef OmXyzDllH
#define OmXyzDllH
#include "windows.h"

// Define _CALLSTYLE_ to determine the calling convention for the DLL ----------
// routines
//
// #define _CALLSTYLE_    __pascal
#define _CALLSTYLE_    __cdecl
// #define _CALLSTYLE_    __fastcall
//
// -----------------------------------------------------------------------------

#ifdef XYZDLL_EXPORTS
#define XYZ_DLL __declspec(dllexport)
#else
#define XYZ_DLL __declspec(dllimport)
#endif

// Include the status bit definitions
#include  "OmXyzDll_StatusBits.h"

#ifdef __cplusplus
extern "C"
{ // define as extern "C" in C++ compilers to give a slight chance of cross-platform operation...!
#endif

  // Adminstration and information routines +++++++++++++++++++++++++++++++++++++++
  //
  // These allow the DLL to get the parameter filename and the DDL folder from OMDAQ
  XYZ_DLL bool _CALLSTYLE_ XyzSetParameterFileName(wchar_t *cText, int nChar);

  XYZ_DLL bool _CALLSTYLE_ XyzSetDLLfolder(wchar_t *statusText, int nChar);
  // -----------------------------------------------------------------------------
  //
  // XyzCapabilityMask returns a DWORD mask that describes the basic functionality
  // of the hardware and allows OMDAQ to make the user interface.
  // The return value is assembled by ORing the capability constants
  // defined in the header.
  // >>>>>> THIS MUST BE DEFINED <<<<<<<
  XYZ_DLL DWORD _CALLSTYLE_ XyzCapabilityMask();
  //
  // XyzDllVersion returns the version number of the DLL file.
  // This information is displayed in the Show Configuration command in OMDAQ
  XYZ_DLL bool _CALLSTYLE_ XyzDllVersion(int * majorVersion, int * minorVersion,
	  int * buildNumber);
  //
  // XyzDescription fills a char string that describes the XYZ stage
  // This information is displayed in the Show Configuration command in OMDAQ
  // nChar is the length of the supplied buffer (typically 80 characters)
  XYZ_DLL bool _CALLSTYLE_ XyzDescription(char *statusText, int nChar);
  //
  // XyzHwDescription fills a char string that decsribes the current setup
  // This information is displayed in the Show Configuration command in OMDAQ
  // (COM ports, card slot numbers etc.)
  // nChar is the length of the supplied buffer. (typically 80 characters)
  XYZ_DLL bool _CALLSTYLE_ XyzHwDescription(char *statusText, int nChar);
  //
  // XyzAuthor returns the author credits and copyrights etc.
  // This information is displayed in the credits in the About command in OMDAQ
  // nChar is the length of the supplied buffer.  (typically 80 characters)
  XYZ_DLL bool _CALLSTYLE_ XyzAuthor(char *statusText, int nChar);
  //
  // XyzDisplayDecimals retunrs the number of decimal places to display in the readouts
  // of position in millimetres and angle in degreees (the same value is used for both
  // displays).
  // >>>>>>>>>>>>>  NOTE:  This is no longer used.  OMDAQ sets the display resolution
  // XYZ_DLL int _CALLSTYLE_ XyzDisplayDecimals ();
  //
  // ---------Procedures for optional parameters -----------------------------------
  // (See the header of XyzOptionCount in the source file for more information
  //
  // returns the expected number of optional parameters
  XYZ_DLL int _CALLSTYLE_ XyzOptionCount();
  //
  // XyzOptionHeader returns a description of the optional parameter nHdr.  This is used in the
  // user interface for setting up the stage BEFORE the initialisation routine is called.
  // Should return false if nHdr is out of range.
  XYZ_DLL bool _CALLSTYLE_ XyzOptionHeader(int nHdr, char * optionsHdr,
	  int szOptionsHdr);
  //
  // XyzOptionValue returns the current value of an option.  This is used primarily on
  // initialisation.  Use this to provide sensible starting values for the parameters
  // to assist th euser in setting up a new stage.
  // Should return false if nHdr is out of range.
  XYZ_DLL bool _CALLSTYLE_ XyzOptionValue(int nHdr, char * optionValue,
	  int szOptionValue);
  //
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // Initialisation routines
  //
  //
  XYZ_DLL bool _CALLSTYLE_ XyzInitialise(char ** options = NULL,
	  int szOptions = 0);
  // Initialisation code here
  // This should include e.g.:  allocation of resources, setting up comms link,
  // starting the controller, finding the home marker, setting up any hardware parameters
  // such as motor current, motor steps and scaling, encoder steps and scaling, etc.
  // Speed and Acceleration are set by OMDAQ.
  //
  // If the stages have home markers, OMDAQ will move the stage to the postiton at last shutdown,
  // othewise, OMDAQ defines the stage position after initialisation to be the position at last shutdown.
  // Return false if it fails (IMPORTANT!!)
  //
  // If the stage needs optional parameters which can be set up at runtime (e.g. COM port
  // number, bauds, etc.) then these can be passed in as character strings in the options arguments.
  // These are set by the user in the "Miscellaneous" tab of the XYZ setup dialog.  If you use optionStrings,
  // define the procedures

  XYZ_DLL bool _CALLSTYLE_ XyzShutDown();
  // Full shutdown code here  - stop stage if it's moving,
  // power down, free comms links and free resources.
  //
  // OMDAQ saves the position at shutdown ready for the next startup.
  // return false if it fails.

  // These procedures initialise the values of the position or angle readouts to the supplied values.
  // NewPosition and NewAngle are pointers to double[3] arrays which contain on entry the
  // new values of the absolute postions (mm) or angles (deg) for axes 0..2
  // Is not required for stages with hardware zero markers, in which case just return true.
  //
  // OMDAQ uses XyzSetCurrentPosition (or ...Angle)(0,0,0) as a synonym for Set Home.
  // If your stage requires a different call to define the current position as HOME,
  // you should trap the situation when (x == y == z == 0).
  XYZ_DLL bool _CALLSTYLE_ XyzSetCurrentPosition(double * NewPosition);
  XYZ_DLL bool _CALLSTYLE_ XyzSetCurrentAngle(double * NewAngle);
  //
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  // Motion parameters *****************************************************************************
  // These set the LINEAR speed and acceleration per axis (assumed to be the same in the
  // accel and decel phases.  Units are  mm/sec and mm/sec2
  // NewAccel and NewSpeed are pointers to double[3] arrays containing the new values for each axis.
  // At present OMDAQ only defines a single accel value for all axes, so the Accel routines will
  // be called with x = y = z = Accel.
  XYZ_DLL bool _CALLSTYLE_ XyzSetAccel(double * NewAccel);
  XYZ_DLL bool _CALLSTYLE_ XyzSetSpeed(double * NewSpeed);

  // These set the ROTATIONAL speed and acceleration per axis (assumed to be the same in the
  // accel and decel phases.  Units are  deg/sec and deg/sec2
  // NewAccel and NewSpeed are pointers to double[3] arrays containing the new values for each axis.
  XYZ_DLL bool _CALLSTYLE_ XyzSetRotAccel(double * NewAccel);
  // This comment is no longer valid:  OMDAQ DOES set rotary acceleration.
  ///*  At present OMDAQ does not define rotational acceleration, so this call is not used.
  // This must be set up during initialisation */
  //
  XYZ_DLL bool _CALLSTYLE_ XyzSetRotSpeed(double * NewSpeed);

  // Power On-off.  Turns the power to all axes ON (Enabled = true) or OFF (Enabled = false)
  // Leaves the controller active and reporting.
  // returns true for success.
  XYZ_DLL bool _CALLSTYLE_ XyzPowerOn(bool Enabled);
  //
  // *************************************************************************************************

  // Motion commands. +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // Move the position or angle to the absolute values supplied in the arguments.
  // Arguments are pointers to double[3] containing the new values.
  // The routines are expected to return immediately - waiting for position is handled by OMDAQ
  //
  XYZ_DLL bool _CALLSTYLE_ XyzMoveToPosition(double * NewPosition);
  XYZ_DLL bool _CALLSTYLE_ XyzMoveToAngle(double * NewAngle);

  // XyzHalt performs an immediate halt (emergency stop, so no deceleration) on all axes
  XYZ_DLL bool _CALLSTYLE_ XyzHalt();
  //
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  // Status reporting +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // GetPosition and GetAngle read back the current values into the arguments, which are pointers
  // to double[3].
  XYZ_DLL bool _CALLSTYLE_ XyzGetPosition(double * CurrentPosition);
  XYZ_DLL bool _CALLSTYLE_ XyzGetAngle(double * CurrentAngle);
  // GetMotorTemp returns the temperature in degrees of all axes.
  // MotorTemp is a pointer to a double.
  // if iAxis = -1 this it's an array big enough to hold all motor temps.
  // If iAxis >= 0 the temp of iAxis is put into th efirst element of the array.
  XYZ_DLL bool _CALLSTYLE_ XyzGetMotorTemp(double *MotorTemp, int iAxis = -1);
  //
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  // Status and Error handling commands.  *************************************
  //
  // StageStatus returns the DRVSTAT (UINT64) status mask built
  // from the mask constants defined in OmXyzDll_StatusBits.h.  Optionally the program may ask
  // for more details in the AxisStatus DWORDs by passing a non-NULL pointer to
  // AxisStstus.  This is DWORD[3] or DWORD[6] depending on the capabilities of the stage.
  // if iAxis = -1 this it's an array big enough to hold all axis status.
  // If iAxis >= 0 the temp of iAxis is put into the first element of the array.
  // Note that for single axis calls only the single axis segments of status are filled
  // so this must be managed in the calling program,
  XYZ_DLL DRVSTAT _CALLSTYLE_ XyzStageStatus(DWORD * AxisStatus = NULL);
  XYZ_DLL DRVSTAT _CALLSTYLE_ XyzAxisStatus(int iAxis = -1,
	  DWORD * AxisStatus = NULL);
  //
  // XyzFaultAck is called after StageStatus reports a fault - defined as a POSLIM, NEGLIM or HWFAULT
  // on any axis.  This should be used to clear faults (e.g. backing off from limit switches).
  // Return values have the followinhg meanings:
  // XyzFltAckOK    0    // Fault has been cleared OK (as far as I can tell)
  // XyzFltAckFatal 1    // Fault cannot be cleared and the stage is dead
  // (in which case OMDAQ will try to do a tidy shutdown)
  // XyzFtlAckRetry 2    // I may be able to clear the fault if you try again
  XYZ_DLL int _CALLSTYLE_ XyzFaultAck();
  //
  // This call returns a text description of the last HWFAULT encountered
  // The existence of a fault must be signalled in the StageStatus flag mask.
  // nChar is the length of the supplied buffer.    (typically 80 characters)
  //
  // return true for success.
  XYZ_DLL bool _CALLSTYLE_ XyzLastFaultText(char *statusText, int nChar);
  //
  // *************************************************************************
#ifdef __cplusplus
} // End of extern "C"
#endif

#endif
