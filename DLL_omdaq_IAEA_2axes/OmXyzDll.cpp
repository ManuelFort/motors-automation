
/* ---------------------------------------------------------------------------

 This file contains the main code (OmXyzDll.cpp) to build a DLL that controls a
 motorized stage with 2 translation degrees of freedom. the motion along each
 axis is assured by a stepper motor with a step of 2.54 microns. The control of
 this stage is assured by a Arduino Nano.

 All the comments were made by me. This file was built using the source file
 provided with OMDAQ-3 as a starting point. Some of the comments are just
 small modifications to what what Geoff Grime originally commented.

 Any questions can be sent to manelfortunato94@gmail.com


 Manuel Fortunato, June 2021
 ---------------------------------------------------------------------------*/



#pragma hdrstop
#include <time.h>
#include<math.h>
#include <cmath>
#include <stdio.h>
#include <stdlib>
#define XYZDLL_EXPORTS 1
#include "OmXyzDll.h"
#include "rs232.h"
#include <cstring>
#include <string>
#include <sstream>

#define XYZDLL_EXPORTS 1
#include "OmXyzDll.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)

// ______Global variables
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
#define nOptions 3
char OptionText[nOptions][32];
bool optionsCopied = false;


/*Global variables are useful since they can be accessed and modified by any
 function.
 Besides the global variables included originally in the code provided
 the following global variables were added. */

//Global variables to store RS232 communication parameters
char modo[4];
int taxabaud;
int port_nmr;


using namespace std;

/******************************* Adminstration routines *******************************/


/*XyzCapabilityMask returns a DWORD mask that describes the basic functionality
 of the hardware and allows OMDAQ to make the user interface.
 The return value is assembled from the capability constants
 defined in OmXyzDll_StatusBits.h */
 /*>>>>>> THIS MUST BE DEFINED <<<<<<<*/
XYZ_DLL DWORD _CALLSTYLE_ XyzCapabilityMask() {


	/*The flag XYZCAP_XYZ3 declares that the stage is capable of translation
	along the 3 Cartesian axes (linear axes). Due to a shortcoming in OMDAQ-3
	this must always be	declared and REQUIRES that:

  1) The procedures related to moving (XyzMoveToPosition(...)) or setting up
  the position (XyzSetCurrentPosition(...)) along the linear axes must do
  nothing for the unavailable axes (and always return true).
  2) The procedure to obtain the position of the stage along the linear axes
  (XyzGetPosition(...)) must export 0 for the unavailable axes (and always
  return true).
  3) When the status of the stage is queried through the function
  XyzAxisStatus(-1, Dword *AxisStatus) the status mask must be ORed with the
  appropriate ST_AX_INPOSITION flag for each of the unavailable linear axis,
  and with the flag ST_ALL_XYZ_MOTORS_ON. This informs OMDAQ that the
  "missing motors" are always in position and ON.

  IMPORTANT: The status lights in the XYZ control panel should be green and/or
  blue but NOT red.*/


  return (XYZCAP_XYZ3);

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

/* XyzDescription fills a char string that describes the XYZ stage
 nChar is the length of the supplied buffer (typically 80 characters)*/
XYZ_DLL bool _CALLSTYLE_ XyzDescription(char *statusText, int nChar) {
  strncpy(statusText, "XYZ stage controlled by user-supplied DLL", nChar);
  return true;
}

/* XyzHwDescription is meant to fill a char string that decsribes the current
setup (COM ports, card slot numbers etc.) nChar is the length of the supplied
buffer. (typically 80 characters). */
XYZ_DLL bool _CALLSTYLE_ XyzHwDescription(char *statusText, int nChar) {
  strncpy(statusText, "COM45 9600baud", nChar);
  return true;
}


/* XyzAuthor returns the author credits and copyrights etc.
 nChar is the length of the supplied buffer.  (typically 80 characters) */
XYZ_DLL bool _CALLSTYLE_ XyzAuthor(char *statusText, int nChar) {
  strncpy(statusText,
	  "DLL written by A. Coder (c), ACME Software Inc., 2020", nChar);
  return true;
}



// ---------Procedures for optional parameters ----------
/* When OMDAQ-3 is executed there is a window that pops up asking for
parameters. These parameters are passed as strings to the
XyzInitialise(char **options, int szOptions) function through the char
**options argument.

In order to create the window interface OMDAQ needs to know the number of
parameters and the name of each one. These are obtained using the
XyzOptionCount and XyzOptionHeader functions.

XyzOptionValue establishes the default parameter values. This is used to
provide sensible starting values for the parameters to assist the user in
 setting up a new stage.
 */
XYZ_DLL int _CALLSTYLE_ XyzOptionCount() {
  return nOptions;
}

/*In OMDAQ-3 there is the possibility to specify a path to an optional
parameters file through Tools->Hardware options->Xyz_stage->Edit->Configuration
I believe that the function XyzSetParameterFileName provides this path to the
DLL through the cText pointer argument so that the parameters can be retrieved.
*/
XYZ_DLL bool _CALLSTYLE_ XyzSetParameterFileName(wchar_t *cText, int nChar) {
  // IniFile = UnicodeString(&cText[0]);
  return true;
}


/*In OMDAQ-3 there is the possibility to specify what I believe to be a
diferent path for the DLL to be located in (it is located by default in
the instalation folder). This path field can be accessed through
Tools->Hardware options->Xyz_stage->Edit->Configuration. However I don't think
that this functionality is working correctly (OMDAQ-3.2.4.1009).
The XyzSetDllFolder function is probably meant to get this new path
to the DLL folder through the statusText pointer argument.
*/
XYZ_DLL bool _CALLSTYLE_ XyzSetDLLfolder(wchar_t *statusText, int nChar) {
  return true;
}

/* XyzOptionHeader sets up the name of the parameters in the parameters
window interface. */
XYZ_DLL bool _CALLSTYLE_ XyzOptionHeader(int nHdr, char * optionsHdr,
	int szOptionsHdr) {
  bool ok = false;
  //Added parameters to set up RS232 communication
  //Also had to change the macro #define nOptions 2 to #define nOptions 3
  char * initHdrs[nOptions] = {"COM", "Baud", "Mode"};
  if ((nHdr >= 0) && (nHdr < nOptions)) {
	strncpy(optionsHdr, initHdrs[nHdr], szOptionsHdr);
	ok = true;
  }
  return ok;
}


/* XyzOptionValue sets the default parameter values in the parameters window
interface to assist the user in setting up a new stage. Should return false if
nHdr is out of range. */
XYZ_DLL bool _CALLSTYLE_ XyzOptionValue(int nHdr, char * optionVal,
	int szOptionVal) {
  bool ok = false;

  //Added initial sensible values for RS232 communication
  //the COM port number is the only field that may have to be changed
  char * initVals[nOptions] = {"5", "9600", "8N1"};
  if ((nHdr >= 0) && (nHdr < nOptions)) {
	if (!optionsCopied) {
	  strncpy(&OptionText[nHdr][0], initVals[nHdr], 32*sizeof(char));
	}
	strncpy(optionVal, &OptionText[nHdr][0], szOptionVal);
	ok = true;
  }
  return ok;
}


/****************************** End of administration routines *******************************/




/****************************** Initialisation routines *******************************/

// ---------------------------------------------------------------------------
XYZ_DLL bool _CALLSTYLE_ XyzInitialise(char **options, int szOptions) {
  /* Initialisation code here

   This function obtains the parameter values from the parameters window
   interface through the char **options argument. Namely the RS232
   communication parameters and the step of each avaliable axis. Global
   variables are defined with these parameters so that they can be used by
   every function in the DLL.

   The RS232 channel is opened.

	*/




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






  //in mm
  PosStep[0]=0.00254;
  PosStep[1]=0.00254;
  PosStep[2]=0;




  //Getting RS232 parameters from OMDAQ-3 parameters window

  if(options[0]!=NULL){
  port_nmr=atoi(options[0])-1;
  }

  if(options[1]!=NULL) {
  taxabaud=atoi(options[1]);
  }


  if(options[2]!=NULL) {
  std::strcpy(modo,options[2]);
  }


	//Openning communications port.
	if(RS232_OpenComport(port_nmr, taxabaud, modo))
  {
	return(0);
  }


  DllPowerOn = true;
  return true;
}

// ---------------------------------------------------------------------------
/* The XyzShutDown() function is meant to perform a Full shutdown - stop stage
  if it's moving, power down, free comms links and free resources.

   OMDAQ saves the position at shutdown ready for the next startup.
   Returns false if it fails. */
XYZ_DLL bool _CALLSTYLE_ XyzShutDown() {

  /*This was not a needed funcionality
	so this function was not used. */

  return true;
}

/*--------------------------------------------------------------------------*/
 /* XyzSetCurrentPosition(double * NewPosition) is meant to inform the DLL of
the values of the positions of the stage along the linear axes, according to
OMDAQ. NewPosition is a pointer to a double[3] array which contains the
values of the absolute positions for the 3 linear axes. This function is used
when the current position is known by OMDAQ but not by the DLL.
This is only (I believe) in 2 situations:
	1 - When the program is initialised. The position is the position
	in which the stage was in when OMDAQ was last shut down.
	2 - When the reference frame is changed in OMDAQ. That is, a new set of
	coordinates is assigned to the current position of the stage.


Note that this procedure is not required for stages with hardware zero markers,
in which case just return a true.

.*/
XYZ_DLL bool _CALLSTYLE_ XyzSetCurrentPosition(double * NewPosition) {

  /*In this function the current position of the stage is set to correspond
  to the position received in the argument of the
  function - double * NewPosition.
  There's no motion involved, this function just sets new frame of reference
  */


  for (int i = 0; i < 2; ++i) {
	CurrentDllPosition[i] = NewPosition[i];
	DemandPosition[i] = NewPosition[i];

  }

  //The 3rd axis is not used so it's position is always 0
  CurrentDllPosition[2]=0;
  DemandPosition[2]=0;
  return true;
}

/* XyzSetCurrentAngle(...) is the analogous function to XyzSetCurrentPosition
but for the angles. However since there are no rotation degrees of freedom for
this stage this function is not important. */
XYZ_DLL bool _CALLSTYLE_ XyzSetCurrentAngle(double * NewAngle) {


  for (int i = 0; i < 3; ++i) {
	CurrentDllAngle[i] = NewAngle[i];
	DemandAngle[i] = NewAngle[i];
  }
  return true;
}


/********************************* End of initialisation routines ********************************/






/********************************** Routines to set up motion parameters *******************************/

/* XyzSetSpeed(...) and XyzSetAccel(...) set the LINEAR speed and acceleration
per axis, respectively. The acceleration is assumed to be the same in the accel
and decel phases.  Units are  mm/sec and mm/sec2.
NewAccel and NewSpeed are pointers to double[3] arrays containing the new
values for each axis.
At present OMDAQ only allows a single acceleration value for all axes (possibly
the first argument of NewAccel, NewAccel[0]?). */
XYZ_DLL bool _CALLSTYLE_ XyzSetAccel(double * NewAccel) {
	/*This was not a needed funcionality
	so this function was not used. */
  return true;
}

XYZ_DLL bool _CALLSTYLE_ XyzSetSpeed(double * NewSpeed) {

	/*This was not a needed funcionality
	so this function was not used.*/

  for (int i = 0; i < 3; ++i) {
	LinSpeed[i] = NewSpeed[i];
  }

  return true;
}

/* XyzSetRotSpeed(...) and XyzSetRotAccel set the ROTATIONAL speed and
acceleration per axis, respectively. The acceleration is assumed to be the same
in the accel and decel phases.  Units are  deg/sec and deg/sec^2.
 NewAccel and NewSpeed are pointers to double[3] arrays containing the new
 values for each axis.   */
XYZ_DLL bool _CALLSTYLE_ XyzSetRotAccel(double * NewAccel) {

	/*This was not a needed funcionality
	so this function was not used.*/

  return true;
}

XYZ_DLL bool _CALLSTYLE_ XyzSetRotSpeed(double * NewSpeed) {

	/*This was not a needed funcionality
	so this function was not used.*/

  for (int i = 0; i < 3; ++i) {
	RotSpeed[i] = NewSpeed[i];
  }
  return true;
}

/* XyzPowerOn(...) is meant to control the power of the stage (on or off).
 If Enabled = true (false) the power should be turned ON (OFF) for all axes.
 It should leave the controller active and reporting.
 Returns true for success. */
XYZ_DLL bool _CALLSTYLE_ XyzPowerOn(bool Enabled) {

	/*This was not a needed funcionality
	so this function was not used.*/

  DllPowerOn = Enabled;
  return true;
}


/********************************** End of routines to set up motion parameters *******************************/







/********************************** Routines for motion command *******************************************/


/*To understand the format of the strings sent to the Arduino Nano please
 read section 2 - "Communication with
Nano Arduino" of the user manual of the Universal DLL ("Universal DLL.pdf").
This stage is a particular case with 2 translation axes of the general case
 described in "Universal DLL.pdf".
*/


/* XyzMoveToPosition(...) is meant to move the stage along the linear
(Cartesian) axes to the absolute position values supplied in the arguments.
Arguments are pointers to double[3] containing the new values.
The routines are expected to return immediately - waiting for the stage to
reach it's position is handled by OMDAQ alone and not by the DLL
*/
XYZ_DLL bool _CALLSTYLE_ XyzMoveToPosition(double * NewPosition) {



    	/*
	In this function I just create a string with the motion order for the
	Arduino Nano. For each of the two motion axes, taking into
	account the current position and the new one, I provide 2 numbers to the
	order string: the length of the motion in number number of steps and the
	sense of motion (0 or 1).

	*/



  for (int i = 0; i < 3; ++i) {
	DemandPosition[i] = NewPosition[i];
  }

  string dir[2];
  string nsteps[2];
  string order_aux;
  string order_aux2;


  for (int i = 0; i < 2; ++i) {
		if (NewPosition[i]>CurrentDllPosition[i]) {
			/*finding number of steps for the desired position;
			turning the double variable with no. of steps into a string
			and erasing the decimal part in the string since the order for the
			NanoArduino can only have an integer number of steps*/
			nsteps[i]=std::to_string((NewPosition[i]-CurrentDllPosition[i])/PosStep[i]);
			std::string::size_type k = nsteps[i].find(".");
			nsteps[i].erase(k, std::string::npos);
			dir[i]="1";
		}
		else {
			nsteps[i]=std::to_string((CurrentDllPosition[i]-NewPosition[i])/PosStep[i]);
			std::string::size_type k = nsteps[i].find(".");
			nsteps[i].erase(k, std::string::npos);
			dir[i]="0";
		}
  }

  //Setting up order and sending it
  order_aux=dir[0]+" "+nsteps[0]+" "+dir[1]+" "+nsteps[1]+"\n";

  const char* order=order_aux.data();




  RS232_cputs(port_nmr, order);



  tLin = clock();
  return true;
}


/* XyzMoveToAngle(...) is the analogous function to XyzMoveToPosition
but for the angles. However since there are no rotation degrees of freedom for
this stage this function is not important. */
XYZ_DLL bool _CALLSTYLE_ XyzMoveToAngle(double * NewAngle) {

  tRot = clock();
  return true;
}

// XyzHalt() is meant to perform an immediate halt (emergency stop, so no
// deceleration) on all axes
XYZ_DLL bool _CALLSTYLE_ XyzHalt() {

	/*This was not a needed funcionality
	so this function was not used.*/

  return true;
}


/********************************** End of routines for motion command *******************************************/





/********************************** Routines for stage status reporting **********************************************/


/* XyzGetPosition(...) provides the current value of the
position of the stage to OMDAQ-3. This value is exported
in the argument, which is a pointer to a double[3]. The library
receives the required position for the stage by the user, through OMDAQ, and
exports the actual position reached, to OMDAQ. Of course that there is no
feedback from the stage to get the actual physical position. But this
function provides the expected reached position taking into account that the
motion is made by stepper motors.
*/
XYZ_DLL bool _CALLSTYLE_ XyzGetPosition(double * CurrentPosition) {
  clock_t tNow = clock();

    /*

  For each axis, this function exports the approximation
  to the required position that the stepper motor that performs the motion
  is capable of.

  If the required position is greater (smaller) than the current position,
  the motor's final position is smaller (greater) than the required position.
  This is due to the motion order  that is sent in the function
  XyzMoveToPosition(...) - the number of steps for each motion is rounded down.

  */

  /*Step slightly smaller than real step just because of the (finite)
  precision of doubles; This is a small trick to assure the correct behaviour
  for the specific cases where the actual position that the motor achieves can
   match exactly the required position by the user.
  */
  double posstep[2];
  posstep[0]=PosStep[0]-0.01*PosStep[0];
  posstep[1]=PosStep[1]-0.01*PosStep[1];

  for(int i=0; i<2; ++i) {

	  if(DemandPosition[i]>CurrentDllPosition[i]){
	  /*This "while" gets the closest approximation that the motor is capable
	  of to the required position, by default.
		*/
		while ((DemandPosition[i]-CurrentDllPosition[i]) >= posstep[i] ) {
			CurrentDllPosition[i]+= PosStep[i];
	  }
  }
  else if(DemandPosition[i]<CurrentDllPosition[i]){
		/*This "while" gets the closest approximation that the motor is capable
		of to the required position, by excess.*/
		while ((CurrentDllPosition[i] - DemandPosition[i]) >= posstep[i] ) {
			CurrentDllPosition[i]-= PosStep[i];
	  }

  }

  }

  CurrentPosition[0] =CurrentDllPosition[0];
  CurrentPosition[1]=CurrentDllPosition[1];
  //3rd axis' position is always 0
  CurrentPosition[2]=0;


  tLin = tNow;
  return true;
}

/*XyzGetAngle(...) is the analogous function to XyzGetPosition(...) but for the
rotation axes. Since there are no rotation axes in this stage this function is
not important. */
XYZ_DLL bool _CALLSTYLE_ XyzGetAngle(double * CurrentAngle) {
  clock_t tNow = clock();


  tRot = tNow;
  return true;
}



/*
 GetMotorTemp is meant to return the temperature in degrees of all axes/motors.
 MotorTemp is a pointer to a double array.
 If iAxis = -1 MotorTemp is an array big enough to hold all motor temperatures.
 If iAxis >= 0 the temperature of iAxis is put into the first element of the
 array.
 Of course that there must be a temperature sensor in the motors for this
 function to be useful.
*/

XYZ_DLL bool _CALLSTYLE_ XyzGetMotorTemp(double *MotorTemp, int iAxis) {

	/*This was not a needed funcionality
	so this function was not used.*/

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


/* XyzStageStatus(...) informs OMDAQ-3 of what is happening with the whole
stage by calling the function XyzAxisStatus(int iAxis, DWORD * AxisStatus)
with iAxis=-1. Please read the description of the XyzAxisStatus(...) function
below.
*/
XYZ_DLL DRVSTAT _CALLSTYLE_ XyzStageStatus(DWORD * AxisStatus) {
  return XyzAxisStatus(-1, AxisStatus);
}


/* XyzAxisStatus(...) informs OMDAQ-3 of what is happening with the stage by
returning the DRVSTAT (UINT64) status mask built from the mask constants
defined in OmXyzDll_StatusBits.h. Each mask constant that is ORed with the
status mask activates one particular bit of the status mask to inform something
to OMDAQ. For example:

ST_AX1_INPOSITION = 100000000000
ST_AX2_INPOSITION = 10000000000000000000
ST_AX3_INPOSITION = 1000000000000000000000000000

status = status | ST_AX1_INPOSITION | ST_AX2_INPOSITION | ST_AX3_INPOSITION

status is then equal to 1000000010000000100000000000
*/

/*
If iAxis = -1 the status of all axes is returned and, in theory, if 0<=iAxis<=5
only the status of axis number iAxis is returned. However the code, as provided
with OMDAQ-3 (3.2.4.1009), does NOT work with 0<=iAxis<=5 and XyzAxisStatus
is always called from OMDAQ-3 with iAxis=-1. Geoff Grime is probably expecting
to use this feature with some future version of OMDAQ-3.
*/

/*
I don't understand what the pointer argument AxisStatus is for in
XyzAxisStatus(int iAxis, DWORD * AxisStatus)! According to Geoff Grime:

"Optionally the program may ask for more details in the AxisStatus DWORDs by
passing a non-NULL pointer to AxisStatus.  This is DWORD[3] or DWORD[6]
depending on the capabilities of the stage. If iAxis = -1 this is an array big
enough to hold all axis status. If iAxis >= 0 the status of iAxis is put into
the first element of the array. Note that for single axis calls only the single
axis segments of status are filled so this must be managed in the calling
program."

Maybe an experiment for a future version of OMDAQ-3 ? I don't think that this
is used for now.

*/

XYZ_DLL DRVSTAT _CALLSTYLE_ XyzAxisStatus(int iAxis, DWORD * AxisStatus) {

  /*In the original code provided with OMDAQ-3 this
  function had several conditional expressions to check if the stage had
  reached hardware limits. However this is not stricly necessary so I removed
  them.
  This function mainly checks if the stage is in movement or not and sets the
  status unsigned integer word accordingly.
  */

  /*
  NOTE:
  Due to a shortcoming in OMDAQ-3 the function XyzCapabilityMask(...) must
  always declare that the stage is capable of translation along the 3 Cartesian
  axes. This REQUIRES that:

    3) When the status of the stage is queried through the function
  XyzAxisStatus(-1, Dword *AxisStatus) the status mask must be ORed with the
  appropriate ST_AX_INPOSITION mask for each of the unavailable linear axis,
  and with the mask ST_ALL_XYZ_MOTORS_ON. This informs OMDAQ that the
  "missing motors" are always in position and ON.

  IMPORTANT: The status lights in the XYZ control panel should be green and/or
  blue but NOT red.

  */



  DRVSTAT status = 0;
  int iMin = 0;
  int iMax = 6;
  if (iAxis >= 0) {
	iMin = iAxis;
	iMax = iAxis;
  }

  double posstep[2];
  posstep[0]=PosStep[0]-0.01*PosStep[0];
  posstep[1]=PosStep[1]-0.01*PosStep[1];

  for (int i = iMin; i < iMax; ++i) {

  if (i < 2) {
	  if (fabs(CurrentDllPosition[i] - DemandPosition[i]) >= posstep[i]) {
		switch (i) {
		case 0:
		  status = status | ST_AX1_MOVING;
		  break;
		case 1:
		  status = status | ST_AX2_MOVING;
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

		}
	  }
  }

  else if(i==2){
	 status = status | ST_AX3_INPOSITION;
  }




  }





  if (DllPowerOn) {
	status |= (ST_ALL_XYZ_MOTORS_ON );
  }
  return status;
}


/********************************** End of routines for stage status reporting **********************************************/





/********************************** Routines for handling errors **********************************************/



/*
 XyzFaultAck() is called after StageStatus reports a fault by ORing the
 status variable in the XyzAxisStatus(...) function with the mask constants
 with the terminations "POSLIM", "NEGLIM" or "HWFAULT" on any axis.
 XyzFaultAck() should be used to clear these faults (e.g. backing off from
 limit switches).
 Return values have the followinhg meanings:
 XyzFltAckOK    0    - Fault has been cleared OK (as far as I can tell)
 XyzFltAckFatal 1    - Fault cannot be cleared and the stage is dead
 (in which case OMDAQ will try to do a tidy shutdown)
 XyzFtlAckRetry 2    - I may be able to clear the fault if you try again,
*/
XYZ_DLL int _CALLSTYLE_ XyzFaultAck() {

	/*This was not a needed funcionality
	so this function was not used.*/

  return XyzFltAckOK;
}


/*
 XyzLastFaultText(...) is meant to return a text description of the last
 hardware fault encountered.
 The existence of a fault must be signalled in the status flag mask
 of the XyzAxisStatus(...) function.
 nChar is the length of the supplied buffer (typically 80 characters).

 return true for success.
*/
XYZ_DLL bool _CALLSTYLE_ XyzLastFaultText(char *statusText, int nChar) {

	/*This was not a needed funcionality
	so this function was not used.*/

  strcpy(statusText, "Fault?  What fault?");
  return true;
}
//
// *************************************************************************
