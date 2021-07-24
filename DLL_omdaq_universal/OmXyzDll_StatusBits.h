#ifndef OMXYZDLL_STATUSBITS_H
#define OMXYZDLL_STATUSBITS_H

/*
These definitions are used to report the status of the XYZ stage.

They cover most status conditions and have been found to be sufficient for
general work with a wide range of sample stages

The overall status of the stage is reported by setting the bits of a UINT64
variable with the ST_AX... or ST_ROT... masks.

In the flags, the pesence of "AX" OR "XYZ" means tht the flag applies to a linear axis
(assumed to be axis 1,2 or 3) and anyth with "R" (R1, RO, et.c)
refers to rotary axes (assumed to be 4 to 6)

The typedef DRVSTAT has been defined to refer to UINT64.

These definitions are largely based on the information returned by the Aerotech
A3200 motion control software and may not be relevant to other systems.

*/

//========  18/06/2019 =========================================================
//  THESE FLAGS ARE NO LONGER USED IN THE OMDAQ-3 IMPLEMENTATION.
//  PLEASE RETURN THE STATUS USING THE INDIVIDUAL ST_AX... FLAGS
//
//// Axis status flag masks
//#define AX_MISSING               0x01   // 1 Axis missing
//#define AX_MOTOR_ON              0x02   // 2 Motor on
//#define AX_ACTIVE                0x04   // 3 Axis acive
//#define AX_MOVING                0x08   // 4 Axis moving
//#define AX_INFINITE_MOVE         0x10   // 5 Permanent motion
//#define AX_FOLLOWING             0x20   // 6 Following error
//                               0x40   // 7
//                               0x80   // 8
//#define AX_REF_HOME             0x100   // 9 Refrenced to home
//#define AX_NEG_LIMIT            0x200   // 10 Neg limit active
//#define AX_POS_LIMIT            0x400   // 11 Pos limit active
//#define AX_CONST_SPEED          0x800   // 12 Constant speed phase
//#define AX_SYNC                0x1000   // 13  Axis synchronised
//                             0x2000   // 14
//                             0x4000   // 15
//                             0x8000   // 16
// Stage Status bits
//#define ST_INPOSITION                  0x01UI64      // All linear axes in position
//#define ST_MOTORS_ON                   0x02UI64      // All motors on
//#define ST_AXISFAULT                   0x04UI64      // Fault on some axis
//#define ST_AXISLIMIT                   0x08UI64      // Limit hit on some axis
//#define ST_MOVING                      0x10UI64      // Some linear axis moving
//
//==============================================================================

/*----------------------------------------------------------------------------
   Note that the flags for each axis can be constructed as an n*8-bit left shift
   of the AX1 flags, so for example:
	   ST_AX3_POSLIM = (ST_AX1_POSLIM << 16)
------------------------------------------------------------------------------*/

#define ST_AX1_MOVING                 0x100UI64      // Axis 1 moving
#define ST_AX1_POSLIM                 0x200UI64      // Axis 1 positive limit
#define ST_AX1_NEGLIM                 0x400UI64      // Axis 1 negative limit
#define ST_AX1_INPOSITION             0x800UI64      // Axis 1 in position
#define ST_AX1_MOTOR_ON              0x1000UI64      // Axis 1 motor on
#define ST_AX1_HWFAULT               0x2000UI64      // Axis 1 hardware fault
#define ST_AX1_OVERTEMP              0x4000UI64      // Axis 1 over temperature

#define ST_AX2_MOVING               0x10000UI64      // Axis 2 moving
#define ST_AX2_POSLIM               0x20000UI64      // Axis 2 positive limit
#define ST_AX2_NEGLIM               0x40000UI64      // Axis 2 negative limit
#define ST_AX2_INPOSITION           0x80000UI64      // Axis 2 in position
#define ST_AX2_MOTOR_ON            0x100000UI64      // Axis 2 motor on
#define ST_AX2_HWFAULT             0x200000UI64      // Axis 2 hardware fault
#define ST_AX2_OVERTEMP            0x400000UI64      // Axis 2 over temperature

#define ST_AX3_MOVING             0x1000000UI64      // Axis 3 moving
#define ST_AX3_POSLIM             0x2000000UI64      // Axis 3 positive limit
#define ST_AX3_NEGLIM             0x4000000UI64      // Axis 3 negative limit
#define ST_AX3_INPOSITION         0x8000000UI64      // Axis 3 in position
#define ST_AX3_MOTOR_ON          0x10000000UI64      // Axis 3 motor on
#define ST_AX3_HWFAULT           0x20000000UI64      // Axis 3 hardware fault
#define ST_AX3_OVERTEMP          0x40000000UI64      // Axis 3 over temperature

#define ST_RO1_MOVING           0x100000000UI64      // RO1 moving
#define ST_RO1_POSLIM           0x200000000UI64      // RO1 positive limit
#define ST_RO1_NEGLIM           0x400000000UI64      // RO1 negative limit
#define ST_RO1_INPOSITION       0x800000000UI64      // RO1 in position
#define ST_RO1_MOTOR_ON        0x1000000000UI64      // RO1 motor on
#define ST_RO1_HWFAULT         0x2000000000UI64      // RO1 hardware fault
#define ST_RO1_OVERTEMP        0x4000000000UI64      // RO1 over temperature

#define ST_RO2_MOVING         0x10000000000UI64      // RO2 moving
#define ST_RO2_POSLIM         0x20000000000UI64      // RO2 positive limit
#define ST_RO2_NEGLIM         0x40000000000UI64      // RO2 negative limit
#define ST_RO2_INPOSITION     0x80000000000UI64      // RO2 in position
#define ST_RO2_MOTOR_ON      0x100000000000UI64      // RO2 motor on
#define ST_RO2_HWFAULT       0x200000000000UI64      // RO2 hardware fault
#define ST_RO2_OVERTEMP      0x400000000000UI64      // RO2 over temperature

#define ST_RO3_MOVING       0x1000000000000UI64      // RO3 moving
#define ST_RO3_POSLIM       0x2000000000000UI64      // RO3 positive limit
#define ST_RO3_NEGLIM       0x4000000000000UI64      // RO3 negative limit
#define ST_RO3_INPOSITION   0x8000000000000UI64      // RO3 in position
#define ST_RO3_MOTOR_ON    0x10000000000000UI64      // RO3 motor on
#define ST_RO3_HWFAULT     0x20000000000000UI64      // RO3 hardware fault
#define ST_RO3_OVERTEMP    0x40000000000000UI64      // RO3 over temperature

//#define ST_ROTINPOSITION  0x100000000000000UI64      // All rotary axes in position
//#define ST_ROTMOVING      0x200000000000000UI64      // Some rotary axis moving

// Summary masks.  These are used by OMDAQ to decode the bitmask,
// but can be used to set flags for all axes.
#define ST_ANY_XYZ_MOVING (ST_AX1_MOVING | ST_AX2_MOVING | ST_AX3_MOVING )
#define ST_ANY_XYZ_INPOSITION (ST_AX1_INPOSITION | ST_AX2_INPOSITION | ST_AX3_INPOSITION )
#define ST_ANY_XYZ_NEGLIMIT (ST_AX1_NEGLIM | ST_AX2_NEGLIM | ST_AX3_NEGLIM )
#define ST_ANY_XYZ_POSLIMIT (ST_AX1_POSLIM | ST_AX2_POSLIM | ST_AX3_POSLIM )
#define ST_ANY_XYZ_LIMIT (ST_ANY_XYZ_POSLIMIT | ST_ANY_XYZ_NEGLIMIT)
#define ST_ANY_XYZ_HWFAULT (ST_AX1_HWFAULT | ST_AX2_HWFAULT | ST_AX3_HWFAULT )
#define ST_ANY_XYZ_OVERTEMP (ST_AX1_OVERTEMP | ST_AX2_OVERTEMP | ST_AX3_OVERTEMP )
#define ST_ANY_XYZ_MOTORS_ON (ST_AX1_MOTOR_ON | ST_AX2_MOTOR_ON | ST_AX3_MOTOR_ON )

#define ST_ALL_XYZ_MOVING (ST_AX1_MOVING | ST_AX2_MOVING | ST_AX3_MOVING )
#define ST_ALL_XYZ_INPOSITION (ST_AX1_INPOSITION | ST_AX2_INPOSITION | ST_AX3_INPOSITION )
#define ST_ALL_XYZ_NEGLIMIT (ST_AX1_NEGLIM | ST_AX2_NEGLIM | ST_AX3_NEGLIM )
#define ST_ALL_XYZ_POSLIMIT (ST_AX1_POSLIM | ST_AX2_POSLIM | ST_AX3_POSLIM )
#define ST_ALL_XYZ_LIMIT (ST_ALL_XYZ_POSLIMIT | ST_ALL_XYZ_NEGLIMIT)
#define ST_ALL_XYZ_HWFAULT (ST_AX1_HWFAULT | ST_AX2_HWFAULT | ST_AX3_HWFAULT )
#define ST_ALL_XYZ_OVERTEMP (ST_AX1_OVERTEMP | ST_AX2_OVERTEMP | ST_AX3_OVERTEMP )
#define ST_ALL_XYZ_MOTORS_ON (ST_AX1_MOTOR_ON | ST_AX2_MOTOR_ON | ST_AX3_MOTOR_ON )

#define ST_ANY_R1_MOVING ( ST_RO1_MOVING )
#define ST_ANY_R1_INPOSITION ( ST_RO1_INPOSITION )
#define ST_ANY_R1_NEGLIMIT ( ST_RO1_NEGLIM )
#define ST_ANY_R1_POSLIMIT ( ST_RO1_POSLIM )
#define ST_ANY_R1_HWFAULT ( ST_RO1_HWFAULT )
#define ST_ANY_R1_OVERTEMP ( ST_RO1_OVERTEMP )
#define ST_ANY_R1_LIMIT (ST_ANY_R1_POSLIMIT | ST_ANY_R1_NEGLIMIT)
#define ST_ANY_R1_MOTORS_ON ( ST_RO1_MOTOR_ON )
#define ST_ANY_R1_MOVING ( ST_RO1_MOVING )

#define ST_ALL_R1_INPOSITION ( ST_RO1_INPOSITION )
#define ST_ALL_R1_NEGLIMIT ( ST_RO1_NEGLIM )
#define ST_ALL_R1_POSLIMIT ( ST_RO1_POSLIM )
#define ST_ALL_R1_HWFAULT ( ST_RO1_HWFAULT )
#define ST_ALL_R1_OVERTEMP ( ST_RO1_OVERTEMP )
#define ST_ALL_R1_LIMIT (ST_ALL_R1_POSLIMIT | ST_ALL_R1_NEGLIMIT)
#define ST_ALL_R1_MOTORS_ON ( ST_RO1_MOTOR_ON )

#define ST_ANY_R2_MOVING ( ST_RO1_MOVING | ST_RO2_MOVING )
#define ST_ANY_R2_INPOSITION ( ST_RO1_INPOSITION | ST_RO2_INPOSITION)
#define ST_ANY_R2_NEGLIMIT ( ST_RO1_NEGLIM | ST_RO2_NEGLIM )
#define ST_ANY_R2_POSLIMIT ( ST_RO1_POSLIM | ST_RO2_POSLIM )
#define ST_ANY_R2_HWFAULT ( ST_RO1_HWFAULT | ST_RO2_HWFAULT )
#define ST_ANY_R2_OVERTEMP ( ST_RO1_OVERTEMP | ST_RO2_OVERTEMP )
#define ST_ANY_R2_LIMIT (ST_ANY_R2_POSLIMIT | ST_ANY_R2_NEGLIMIT)
#define ST_ANY_R2_MOTORS_ON ( ST_RO1_MOTOR_ON | ST_RO2_MOTOR_ON )

#define ST_ALL_R2_MOVING ( ST_RO1_MOVING | ST_RO2_MOVING )
#define ST_ALL_R2_INPOSITION ( ST_RO1_INPOSITION | ST_RO2_INPOSITION)
#define ST_ALL_R2_NEGLIMIT ( ST_RO1_NEGLIM | ST_RO2_NEGLIM )
#define ST_ALL_R2_POSLIMIT ( ST_RO1_POSLIM | ST_RO2_POSLIM )
#define ST_ALL_R2_HWFAULT ( ST_RO1_HWFAULT | ST_RO2_HWFAULT )
#define ST_ALL_R2_OVERTEMP ( ST_RO1_OVERTEMP | ST_RO2_OVERTEMP )
#define ST_ALL_R2_LIMIT (ST_ALL_R2_POSLIMIT | ST_ALL_R2_NEGLIMIT)
#define ST_ALL_R2_MOTORS_ON ( ST_RO1_MOTOR_ON | ST_RO2_MOTOR_ON )

#define ST_ANY_R3_MOVING ( ST_RO1_MOVING | ST_RO2_MOVING | ST_RO3_MOVING )
#define ST_ANY_R3_INPOSITION ( ST_RO1_INPOSITION | ST_RO2_INPOSITION | ST_RO3_INPOSITION )
#define ST_ANY_R3_NEGLIMIT ( ST_RO1_NEGLIM | ST_RO2_NEGLIM | ST_RO3_NEGLIM )
#define ST_ANY_R3_POSLIMIT ( ST_RO1_POSLIM | ST_RO2_POSLIM | ST_RO3_POSLIM )
#define ST_ANY_R3_HWFAULT ( ST_RO1_HWFAULT | ST_RO2_HWFAULT | ST_RO3_HWFAULT )
#define ST_ANY_R3_OVERTEMP ( ST_RO1_OVERTEMP| ST_RO2_OVERTEMP | ST_RO3_OVERTEMP )
#define ST_ANY_R3_LIMIT (ST_ANY_R3_POSLIMIT | ST_ANY_R3_NEGLIMIT)
#define ST_ANY_R3_MOTORS_ON ( ST_RO1_MOTOR_ON | ST_RO2_MOTOR_ON | ST_RO3_MOTOR_ON )

#define ST_ALL_R3_MOVING ( ST_RO1_MOVING | ST_RO2_MOVING | ST_RO3_MOVING )
#define ST_ALL_R3_INPOSITION ( ST_RO1_INPOSITION | ST_RO2_INPOSITION | ST_RO3_INPOSITION )
#define ST_ALL_R3_NEGLIMIT ( ST_RO1_NEGLIM | ST_RO2_NEGLIM | ST_RO3_NEGLIM )
#define ST_ALL_R3_POSLIMIT ( ST_RO1_POSLIM | ST_RO2_POSLIM | ST_RO3_POSLIM )
#define ST_ALL_R3_HWFAULT ( ST_RO1_HWFAULT | ST_RO2_HWFAULT | ST_RO3_HWFAULT )
#define ST_ALL_R3_OVERTEMP ( ST_RO1_OVERTEMP| ST_RO2_OVERTEMP | ST_RO3_OVERTEMP )
#define ST_ALL_R3_LIMIT (ST_ALL_R3_POSLIMIT | ST_ALL_R3_NEGLIMIT)
#define ST_ALL_R3_MOTORS_ON ( ST_RO1_MOTOR_ON | ST_RO2_MOTOR_ON | ST_RO3_MOTOR_ON )


//=====  Capability mask bits. These are ORed to tell OMDAQ what the stage can do
#define    XYZCAP_XYZ3              0x01   // three axis orthogonal XYZ stage
#define    XYZCAP_ROT1              0x02   // single axis rotation stage (There is also ROT2)
#define    XYZCAP_ROT3              0x04   // three axis rotation stage
#define    XYZCAP_HOMESWITCH_XYZ    0x08   // All XYZ axes are homed using hardware home switch
#define    XYZCAP_HOMESWITCH_ROT   0x010   // All rotation axes are homed using hardware home switch
#define    XYZCAP_POWER_ONOFF      0x020   // Stage power can be switched off by a software command
#define    XYZCAP_ROT2             0x040   // Two axis rotation stage
#define    XYZCAP_TEMPSENSOR       0x080   // Some motors have readable temperature sensors

#define    XYZCAP_ROT              (XYZCAP_ROT1 | XYZCAP_ROT2 | XYZCAP_ROT3)
#define    XYZCAP_ROT123           (XYZCAP_ROT1 | XYZCAP_ROT2 | XYZCAP_ROT3)
#define    XYZCAP_ROT23            (XYZCAP_ROT2 | XYZCAP_ROT3)

   // These next three flags are set internally by OMDAQ in response to the users setup and DLL
#define    XYZCAP_FIELDS          0x0100   // The stage has separet fields or faces each using the same
										   // logical coordinate frame but with different physicaal origins.
				 // If you use this option, the external DLL MUST be supplied and
				 // the number of fields, names etc. must be defined in it.
				 // (see AngleTransformMain.H for details)
#define    XYZCAP_HANDCONTROL     0x0200   // The stage has a hand control fitted
#define    XYZCAP_EXTDLL          0x0400   // The stage uses an external DLL (AngleTransform.DLL) to transform
										   // between logical and stage coordinates
//
//--------------------------------------------------------------------------------

typedef unsigned __int64 DRVSTAT;

// Fault recovery return values.
#define XyzFltAckOK    0    // Fault has been cleared OK (as far as I can tell)
#define XyzFltAckFatal 1    // Fault cannot be cleared and the stage is dead (in which case OMDAQ will try to do a tidy shutdown)
#define XyzFltAckRetry 2    // I may be able to clear the fault if you try again,

// Stage behaviour during shutdown
#define XyzShutdown_StageMayHaveMoved     0x01
#define XyzShutdown_ReportedPositionLost  0x02

// Home switch behaviour options
#define XyzOpt_HomeLinear 0x01
#define XyzOpt_HomeRotary 0x02

// Wait option
#define XyzOpt_WaitNative 0x04 // specifies that wait moves use native move-wait commands
															 // otherwise uses an immediate command and tests for
															 // position separately

// External fault options
//  This was added really for using the A3200 digital input as fault gnerators to
//  allow external TTL inpits to stop the stage.
//  This makes bits 8 to 23 available for defining this behaviour.
#define XyzOpt_ExtFaultShift 8
#define XyzOpt_ExtFaultMask  0xffff

#define  fltOptionEnable    0x80    // Bits in the flt Option word
#define  fltOptionPolarity 0x100    // for A3200


#endif

