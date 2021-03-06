// ---------------------------------------------------------------------------

/* This file contains the main code (OmXyzDll.cpp) to build a DLL that commands
a stepper motor for rotation, to be used for STIM tomography.

 The motor board used (V8849 RS Components) has a command language simillar
to C.
 Check the instructions manual to get all the details of the commands

  All the comments were made by me. This file was built using the source file
 provided with OMDAQ-3 as a starting point. Some of the comments are just
 small modifications to what what Geoff Grime originally commented.

 Any questions can be sent to manelfortunato94@gmail.com


 Manuel Fortunato, June 2021
 ---------------------------------------------------------------------------
*/



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
#include <windows.h>

#define XYZDLL_EXPORTS 1
#include "OmXyzDll.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)

using namespace std;

// ______Global variables____________________________


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
#define nOptions 8
char OptionText[nOptions][32];
bool optionsCopied = false;


/*Global variables are useful since they can be accessed and modified by
any function. Besides the global variables included originally in the code
provided with OMDAQ-3, the following global variables were added. */
 /*Global variables to store RS232 communication parameters,
initialized with 0's. Two sets of variables are declared. One set to open the
regular communication port to communicate with the V8849 motor control board
and one set to open a second port that controls the power of the motor, to
prevent electromagnetic noise. The variables for the noise COM port
end with "N". */
char modo[4]="0", modoN[4]="0";
int taxabaud=0, taxabaudN=0;
int port_nmr=0, port_nmrN=0;

/*Global variable to store the number of steps/rotation of the motor, obtained
from the OMDAQ-3 parameters window. Although it is not likely the user may wish
to change the microstepping mode of the motor.   */
double steps_rev;


/*Global variable used to prevent RS-232 communications just to test the DLL
without the hardware. If false no RS232 orders are sent and there's no error
when linking OMDAQ-3 with the DLL without the actual RS232 connection
established */
bool COMS=true;



/******************************* Adminstration routines *******************************/




/*XyzCapabilityMask returns a DWORD mask that describes the basic functionality
 of the hardware and allows OMDAQ to make the user interface.
 The return value is assembled from the capability constants
 defined in OmXyzDll_StatusBits.h */
 /*>>>>>> THIS MUST BE DEFINED <<<<<<<*/
XYZ_DLL DWORD _CALLSTYLE_ XyzCapabilityMask() {
  return (XYZCAP_XYZ3 | XYZCAP_ROT1);


  /*This declares that the stage is capable of translation along the 3
  Cartesian axes and rotation along 1 axis. Due to a shortcoming in OMDAQ-3
  translation along the 3 Cartesian axes (linear axes) must always be declared
  and REQUIRES that:

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
  blue but NOT red.

  Of course that in this case none of the 3 linear axis is available.
  */


}


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

/* XyzHwDescription fills a char string that decsribes the current setup
 (COM ports, card slot numbers etc.)
 nChar is the length of the supplied buffer. (typically 80 characters) */
XYZ_DLL bool _CALLSTYLE_ XyzHwDescription(char *statusText, int nChar) {
  strncpy(statusText, "COM45 9600baud", nChar);
  return true;
}


/* XyzAuthor returns the author credits and copyrights etc.
 nChar is the length of the supplied buffer.  (typically 80 characters) */
XYZ_DLL bool _CALLSTYLE_ XyzAuthor(char *statusText, int nChar) {
  strncpy(statusText,
	  "DLL written by Manuel Fortunato, 2021", nChar);
  return true;
}



// ---------Procedures for optional parameters ----------
/* When OMDAQ-3 is executed there is a window that pops up asking for
parameters. These parameters are passed as strings to the
XXyzInitialise(char **options, int szOptions procedure through the options
argument.

In order to create the window interface OMDAQ needs to know the number of
parameters and the name of each one. These are obtained using the
XyzOptionCount and XyzOptionHeader procedures.

 XyzOptionValue establishes the default parameter values. This is used to
 provide sensible starting values for the parameters to assist the user in
 setting up a new stage.
 */

XYZ_DLL int _CALLSTYLE_ XyzOptionCount() {
  return nOptions;
}

/*These allow the DLL to get the parameter filename and the DDL folder
from OMDAQ
*/
XYZ_DLL bool _CALLSTYLE_ XyzSetParameterFileName(wchar_t *cText, int nChar) {

  return true;
}

XYZ_DLL bool _CALLSTYLE_ XyzSetDLLfolder(wchar_t *statusText, int nChar) {
  return true;
}


/* XyzOptionHeader sets up the name of the parameters in the parameters window
interface. */
/*Added options to set up 2 ports for RS232 communication - one to send the
motor commands to the V8849 motor control board and the other to cut the
current supply of the motor when required, to prevent electromagnetic noise.
Also added the parameter motor speed which holds the speed in deg/sec. This
can be any value just take into account that if the speed is to high the
hardware will not be able to keep up.
Although unlikely the user may wish to change the microstepping mode of the
motor so I also added the steps/rotation parameter, which hold the number of
steps that the motor performs to complete a full rotation. Unlike the speed of
the motor this parameter only informs OMDAQ of the hardware settings and is not
configurable at run time. The motor used performs 200 steps/revolution in
normal stepping mode and is usualy configured for quarter step mode where it
performs 800 steps/revolution.*/
XYZ_DLL bool _CALLSTYLE_ XyzOptionHeader(int nHdr, char * optionsHdr,
	int szOptionsHdr) {
  bool ok = false;


  char * initHdrs[nOptions] = {"COM", "Baud", "Mode", "COM (noise)", "Baud (noise)", "Mode (noise)", "Speed (?/s)", "Steps/rotation"}; // For example...
  if ((nHdr >= 0) && (nHdr < nOptions)) {
	strncpy(optionsHdr, initHdrs[nHdr], szOptionsHdr);
	ok = true;
  }
  return ok;
}

/* XyzOptionValue sets the default parameter values in the parameters window
interface to assist the user in setting up a new stage. Should return false if
nHdr is out of range. */
/*NOTES: 1) The noise COM port is always number 0 in the "tomography" computer
(the one with the NVIDIA board). 2) If the speed of the motor is too high the
hardware cannot keep up.
*/
XYZ_DLL bool _CALLSTYLE_ XyzOptionValue(int nHdr, char * optionVal,
	int szOptionVal) {
  bool ok = false;


  char * initVals[nOptions] = {"5", "9600", "8N1", "0", "9600", "8N1", "30", "800"};
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
   communication parameters, the speed and the number of steps for a full
   roation of the motor. Global variables store these parameters so that they
   can be used by every function in the DLL.

   Two RS232 channels are opened - one to send the motor commands to the
   V8849 motor control board and the other to cut the current supply of the
   motor when required, to prevent electromagnetic noise.

   Two commands are sent to the V8849 control board: one to erase any previous
   orders sent to the control board - the "new" command - and one to set the
   rotation speed of the motor - the "cvel(u)" command. For more details about
   these commands please read the manual of the V8849 control board.

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



  //Opening COM port to communicate with the V8849 motor control board.
  //Getting required parameters from the OMDAQ-3 parameters window.
  port_nmr=atoi(options[0])-1;
  taxabaud=atoi(options[1]);
  std::strcpy(modo,options[2]);

  if(COMS) {
	if(RS232_OpenComport(port_nmr, taxabaud, modo))
  {
	return(0);
  }

	//"New" order to erase any previous programs in the control board
	string new_order_s = "new\n";
	const char* new_order = new_order_s.data();
	RS232_cputs(port_nmr, new_order);

  }






  //Openning COM port to control current supply.
  //Getting required parameters from the OMDAQ-3 parameters window
  port_nmrN=atoi(options[3])-1;
  taxabaudN=atoi(options[4]);
  std::strcpy(modoN,options[5]);

  if(COMS) {


	if(RS232_OpenComport(port_nmrN, taxabaudN, modoN))
  {
	return(0);
  }

	/*Order to turn motor OFF. The voltage level of the DTR pin of the
	RS232 port controls the power of the motor.  */
	RS232_disableDTR(port_nmrN);

  }





  /*Storing rotation speed in degrees per second in a global variable.
  Value was obtained from the OMDAQ-3 parameters window */
  RotSpeed[0]=atof(options[6]);
  RotSpeed[1]=0;
  RotSpeed[2]=0;

  /*Setting the physical speed of the motor. First this value must be converted
  from degrees per second to motor steps per second */
  steps_rev=atoi(options[7]);
  double rot_speed;
  double prescale=1;

  rot_speed = RotSpeed[0]*steps_rev/360;
  rot_speed = round(rot_speed);


  /*The motor board cvel(u) function only accepts an argument u > 63 (steps
  per second). However, the physical velocity of the motor can be set below
  this threshold by using the prescale factor. This factor divides any argument
  u given to the cvel(u) function so that the actual velocity of the motor is
  set to u/prescale. For more details about this function please check the
  Programmable Stepper Motor Control Board manual (V8849 RS Components)
  NOTE: the maximum value of the prescale factor is 32767  */


  /*Checking wich prescale factor is needed so that the argument u of the
  cvel(u) function is smaller than 63.
  Multiplying (arbitrarily) the prescale factor by 5 in each iteration. */
  while(rot_speed<63 && prescale < 32767) {

	prescale = prescale * 5 ;
	prescale = round(prescale);
	rot_speed = rot_speed * 5 ;
	rot_speed = round(rot_speed);

  }


  if(rot_speed<63) {
	rot_speed = 63 ;
	RotSpeed[0] = rot_speed*prescale*steps_rev/360 ;
  }

  //Prescaling, if necessary, so that velocities lower than 63 steps/second
  //can be reached.
  if(prescale>1 && COMS){



	string prescale_s;
	prescale_s = to_string(prescale);
	string::size_type ps = prescale_s.find(".");
	prescale_s.erase(ps, string::npos);
	string order_ps = "prescale("+prescale_s+")\n";
	const char* order_ps2=order_ps.data();

	RS232_cputs(port_nmr, order_ps2);


  }


  //Sending the cvel(u) order to the V8849 control board
  string rot_speed_s;
  rot_speed_s=to_string(rot_speed);
  string::size_type dot = rot_speed_s.find(".");
  rot_speed_s.erase(dot, string::npos);
  string order_rs = "cvel("+rot_speed_s+")\n";
  const char* order_rs2=order_rs.data();

  if(COMS) {

  RS232_cputs(port_nmr, order_rs2);

  }



  //Storing the value of the motor's step in degrees
  AngleStep[0]=360/steps_rev;
  AngleStep[1]=0;
  AngleStep[2]=0;


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

Since the there are no linear positions for this stage this function doesn't
do anything.
.*/
XYZ_DLL bool _CALLSTYLE_ XyzSetCurrentPosition(double * NewPosition) {



 /*
 The function XyzCapabilityMask() must always declare that the stage is
 able to perform translation along the 3 Cartesian axes. This function must do
 nothing for the unavailable axes (all 3 in this case) and return true.

 */

  return true;
}


/*
XyzSetCurrentAngle(double * NewAngle) is meant to inform the DLL of
the values of the positions of the stage along the rotation axes, according to
OMDAQ. NewAngle is a pointer to a double[3] array which contains the
values of the absolute postions (in degrees, ?) for the 3 rotation axes.
This function is used when the current position is known by OMDAQ but not by
the DLL. This is only (I believe) in 2 situations:
	1 - When the program is initialised. The angular positions are the
	positions in which the stage was in when OMDAQ was last shut down.
	2 - When the reference frame of some of the rotation axis is changed in
	OMDAQ. That is, a new set of angular coordinates is assigned to the current
	position of the stage.

Note that this procedure is not required for stages with hardware zero markers,
in which case just return a true.
 */
XYZ_DLL bool _CALLSTYLE_ XyzSetCurrentAngle(double * NewAngle) {


  /*In this function the position of the motor is set to correspond
  to the angle received from OMDAQ3 in the argument of the function,
  NewAngle[0], both in the DLL and in the motor control board.
  To do this in the motor control board the datum(axis,val) function is used.
  This functions sets the axis number "axis" (i.e. motor number 0 or motor
  number 1) position to be equal to "val". For more details about this function
  please check the Programmable Stepper Motor Control Board manual
  (V8849 RS Components).
  */

  for (int i = 0; i < 3; ++i) {
	CurrentDllAngle[i] = NewAngle[i];
	DemandAngle[i] = NewAngle[i];
  }

  double n_angle;

  /*Converting angle from degrees to motor
   steps and storing the value in a string. */
  n_angle=NewAngle[0]*steps_rev/360;
  n_angle = round(n_angle);
  string na0=to_string(n_angle);
  string::size_type m = na0.find(".");
  na0.erase(m, string::npos);

  //Setting up const char* with the datum(axis,val) order
  string order;
  order="datum(0,"+na0+")\n";
  const char* order2=order.data();



  //Sending datum(axis,val) to the control board
  if(COMS){
  RS232_cputs(port_nmr, order2);
  }



  return true;
}




/********************************* End of initialisation routines ********************************/





/********************************** Routines to set up motion parameters *******************************/




/* XyzSetSpeed(...) and XyzSetAccel(...) set the speed and acceleration of the
linear axes, respectively. The acceleration is assumed to be the same in the
acceleration and deceleration phases. Units are  mm/sec and mm/sec2.
NewAccel and NewSpeed are pointers to double[3] arrays containing the new
values for each axis. At present OMDAQ only allows a single acceleration value
for all axes (possibly the first argument of NewAccel, NewAccel[0]?). */
XYZ_DLL bool _CALLSTYLE_ XyzSetAccel(double * NewAccel) {

	/*This was not a needed funcionality
	so this function was not used. */

  return true;
}

XYZ_DLL bool _CALLSTYLE_ XyzSetSpeed(double * NewSpeed) {

	/*This was not a needed funcionality
	so this function was not used. */

  for (int i = 0; i < 3; ++i) {
	LinSpeed[i] = NewSpeed[i];
  }
  return true;
}



/* XyzSetRotSpeed(...) and XyzSetRotAccel(...) set the speed and acceleration
for rotational axes, respectively. The acceleration is assumed to be the same
in the acceleration and deceleration phases. Units are  degrees/second (?/s)
and degrees/second^2 (?/s^2).
NewSpeed and NewAccel are pointers to double[3] arrays containing the new
values for each axis. */
XYZ_DLL bool _CALLSTYLE_ XyzSetRotAccel(double * NewAccel) {

	/*This was not a needed funcionality
	so this function was not used.*/

  return true;
}


XYZ_DLL bool _CALLSTYLE_ XyzSetRotSpeed(double * NewSpeed) {

	/*This function does not seem to be reading correctly the values of the
	speed configured in OMDAQ-3, so in this DLL the speed value is provided
	through the main parameters window, which is supplied to the
	XyzInitialise(...) function, and the motor's speed is set in this
	initialisation function. For more details see the comments in the
    XyzInitialise(...) function.
	*/


  return true;
}

/* XyzPowerOn(...) is meant to control the power of the stage (on or off), so
 that the power is controlled through a button in the stage control panel in
 OMDAQ-3. If Enabled = true (false) the power should be turned ON
 (OFF) for all axes. It should leave the controller active and reporting.
 Returns true for success. */
XYZ_DLL bool _CALLSTYLE_ XyzPowerOn(bool Enabled) {

/*
  The motor is powered on and off through a RS232 communication channel
  by controling the voltage level of the DTR pin of this port. Shutting down
  the motor before data is acquired eliminates the electromagnetic noise in the
  acquisition system. The used scheme is that the motor is always off, except
  when it must move. Therefore the motor is turned off when the program is
  initialised through the XyzInitialise(...) function, and then it is turned on
  for the motion, and then back off again, in the XyzMoveToAngle(...) function.
  Controlling the power of the motor through a button was not needed so this
  function was not used. However it's implementation is very simple:

  if(Enabled==true) {

  RS232_enableDTR(port_nmrN);

  }

  else if(Enabled==false) {

  RS232_disableDTR(port_nmrN);

  }

*/




  DllPowerOn = Enabled;
  return true;
}


/********************************** End of routines to set up motion parameters *******************************/







/********************************** Routines for motion command *******************************************/


/* XyzMoveToPosition(...) and XyzMoveToAngle(...) are meant to move,
 respectively, the linear and the angular positions to the absolute values
 supplied in the arguments. Arguments are pointers to double[3] containing
 the new values. The routines are expected to return immediately - waiting
 for the stage to reach it's position is handled by OMDAQ alone and not by the
 DLL.
*/
XYZ_DLL bool _CALLSTYLE_ XyzMoveToPosition(double * NewPosition) {



	/*
	This was not a needed funcionality so this function was not used.

	NOTE: be aware that this function is meant to move the stage along
	the LINEAR axes, which are not available in the case of the tomography
	setup. Don't send any move orders to the V8849 control board through this
	function. Use instead the XyzMoveToAngle(...) function, which is the
	appropriate one for the	ROTATIONAL axes. Using the wrong function may
	result in stage damage.
	*/


  tLin = clock();
  return true;
}

XYZ_DLL bool _CALLSTYLE_ XyzMoveToAngle(double * NewAngle) {



	/*This function sends a move order - "Cmove(val, axis)" - to the
	programmable motor board. Cmove(val, axis) moves the motor number
	"axis" (motor no. 0 or no. 1) to the position val, where the position
	is absolute and in motor steps. For more details about this function
	please check the Programmable Stepper Motor Control Board manual
	(V8849 RS Components).

	*/


	/*
	NOTE: The current angle is stored in an auxiliar variable, double
	c_dll_angle, at the beggining of the function because sometimes OMDAQ
	updates the double CurrentDllAngle[3] variable in the mid of the execution
	of this function. It calls the XyzGetAngle(double * CurrentAngle) in a
	different thread. The angles before and after motion, along with the speed
	of the motor, are used to calculate the time that the motor must be ON so
	that it can move, i.e., the time that the program must pause. If the
	starting angle is not stored in an auxiliary variable there is no way to
	calculate this time, stored in the float "time_sleep" variable, in
	milliseconds, down below, since many times it would be equal to 0 due to
	the unwanted update of the variable CurrentDllAngle[0].
	*/

	double c_dll_angle=CurrentDllAngle[0];
	double n_angle;






	for (int i = 0; i < 3; ++i) {
		DemandAngle[i] = NewAngle[i];
	}

	/*
	Setting up move order. Converting the required angle into number of
	motor steps, in a double variable, and then converting the double into a
	string without decimals places since the Cmove(val, axis) function only
	accepts an integer number of steps.
	*/

	n_angle=NewAngle[0]*steps_rev/360;
	n_angle = round(n_angle);

	string na0=to_string(n_angle);
	string::size_type m = na0.find(".");
	na0.erase(m, string::npos);

	string order;
	order="Cmove("+na0+",0)\n";
	const char* order2=order.data();

	//Turning motor on
	if(COMS){
	RS232_enableDTR(port_nmrN);
	}

	/*
	Calculating the time that the program sleeps so that the motor is
	correctly turned off only AFTER the motion is completed.
	*/
	float time_sleep;
	time_sleep=abs(NewAngle[0]-c_dll_angle)/RotSpeed[0]*1000 +2000;

	//Sending the move order
	if(COMS){
	RS232_cputs(port_nmr, order2);
	}



	Sleep(time_sleep);


	//Turning the motor back off again
	if(COMS) {
	RS232_disableDTR(port_nmrN);
	}


	tRot = clock();
	return true;
}

/* XyzHalt() is meant to perform an immediate halt (emergency stop, so no
deceleration) on all axes
*/
XYZ_DLL bool _CALLSTYLE_ XyzHalt() {
	
	/*This was not a needed funcionality
	so this function was not used.*/

  return true;
}


/********************************** End of routines for motion command *******************************************/





/********************************** Routines for stage status reporting **********************************************/





/* XyzGetPosition(...) and XyzGetAngle(...) are meant to provide the current
value of the linear and angular positions, respectively, to OMDAQ-3. These
values are exported in the arguments, which are pointers to double[3]. For each
case the library receives the required position for the stage by the user,
through OMDAQ, and exports the actual position reached, to OMDAQ. Of course
that there is no feedback from the stage to get the actual physical positions.
But these functions are here to provide the expected value of the reached
positions.

In this case since the stage consists only of a rotation motor, the
function XyzGetPosition(...) always exports the position (0,0,0). The function
XyzGetAngle(...) exports the closest approximation to the required position
that the stepper motor is capable of, since that is the position that the
function XyzMoveToAngle(...) orders the motor to move to.
*/
XYZ_DLL bool _CALLSTYLE_ XyzGetPosition(double * CurrentPosition) {


  /*

  Due to a shortcoming in OMDAQ-3 the stage capaility mask
  (function XyzCapabilityMask()) must always return translation along the
  3 Cartesian axes (linear axes). This requires that:

  2) The procedure to obtain the position of the stage along the linear axes
  (XyzGetPosition(...)) must export 0 for the unavailable axes (and always
  return true).

  */

  clock_t tNow = clock();
  for (int i = 0; i < 3; ++i) {
	CurrentPosition[i]=0;
	CurrentDllPosition[i]=0;
}
  tLin = tNow;
  return true;
}


XYZ_DLL bool _CALLSTYLE_ XyzGetAngle(double * CurrentAngle) {
  clock_t tNow = clock();

  /*

  This function exports the closest approximation to the required position
  that the stepper motor is capable of, since that is the position that the
  function XyzMoveToAngle(...) orders the motor to move to.

  */



  /*Step slightly smaller than real step just because of the (finite)
  precision of doubles. This actualy is unnecessary with the current code...
  */
  double anglestep=AngleStep[0]-0.001*AngleStep[0];

  if(DemandAngle[0]>CurrentDllAngle[0]){

		/*Checking if the motor approximates more the required position
		by defect (the value of the position angle is smaller than the required
		position) or by excess (the value of the position angle is greater than
		the required position).*/

		/*This "while" gets the closest approximation of the stage to the
		required angle position, by defect.
		*/
		while ((DemandAngle[0]-CurrentDllAngle[0]) >= anglestep ) {
			CurrentDllAngle[0]+= AngleStep[0];
	  }


	  /*After getting the closest approximation by defect, this "if" checks
	  if the approximation by excess is better or worse than the one by defect.
	 */
	  if(abs(CurrentDllAngle[0]+AngleStep[0] - DemandAngle[0])<abs(CurrentDllAngle[0] - DemandAngle[0])) {

		  CurrentDllAngle[0]+=AngleStep[0];
	  }
  }

  else if(DemandAngle[0]<CurrentDllAngle[0]){


		/*Checking if the motor approximates more the required position
		by defect (the value of the position angle is smaller than the required
		position) or by excess (the value of the position angle is greater than
		the required position).*/

		/*This "while" gets the closest approximation of the stage to the
		required angle position, by excess.
		*/
		while ((CurrentDllAngle[0] - DemandAngle[0]) >= anglestep ) {
			CurrentDllAngle[0]-= AngleStep[0];
	  }

	  /*After getting the closest approximation by excess, this "if" checks
	  if the approximation by defect is better or worse than the one by excess.
	 */
	  if(abs(CurrentDllAngle[0]-AngleStep[0] - DemandAngle[0])<abs(CurrentDllAngle[0] - DemandAngle[0])) {

		  CurrentDllAngle[0]-=AngleStep[0];
	  }

  }

  CurrentAngle[0] =CurrentDllAngle[0];
  CurrentAngle[1]=0;
  CurrentAngle[2]=0;

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
  reached hardware limits. However this motor does not have any limit on it's
  poisition so I removed them.
  This function checks if the stage is in rotation or not and sets the status
  unsigned integer word accordingly.
  */

  /*
  NOTE:
  Due to a shortcoming in OMDAQ-3 the function XyzCapabilityMask(...) must
  always declare that the stage is capable of translation along the 3 Cartesian
  axes. This REQUIRES that:

  3) When the status of one the unavailable axis is queried through the
  function XyzAxisStatus(int iAxis, DWORD *AxisStauts) the status mask must be
  ORed with the appropriate ST_AX_INPOSITION flag and with the
  ST_ALL_XYZ_INPOSITION flag. This informs OMDAQ that the missing motors
  are always in position and ON.

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
  for (int i = iMin; i < iMax; ++i) {
	if (i < 3) {

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
	else {
	  if (fabs(CurrentDllAngle[i - 3] - DemandAngle[i - 3]) > AngleStep[i-3]) {
		switch (i - 3) {
		case 0:
		  status = status | ST_RO1_MOVING;
		  break;
		case 1:
		  break;
		case 2:
		  break;
		}
	  }
	  else {
		switch (i - 3) {
		case 0:
		  status = status | ST_RO1_INPOSITION;
		  break;
		case 1:
		  break;
		case 2:
		  break;
		}
	  }


	  }


	}


  //All motors on
  if (DllPowerOn) {
	status |= (ST_ALL_XYZ_MOTORS_ON | ST_ALL_R1_MOTORS_ON);
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
