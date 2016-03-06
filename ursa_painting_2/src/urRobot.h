//
//  urRobot.h
//  ursa_udp
//
//  Created by Ben Snell on 2/19/16.
//
//

#ifndef __ursa_udp__urRobot__
#define __ursa_udp__urRobot__

#include "ofMain.h"

class urRobot {
    
public:
    
    // update the robot data
    // input: string of comma-delimited values of the same type
    void updateRobotData(string message);
    
    // ------------------------------
    // -------- ROBOT VALUES --------
    // ------------------------------
    
    // 132 total doubles:
    
    double elapsedTime;
    
    double targetJointPositions[6];
    double targetJointVelocities[6];
    double targetJointAccelerations[6];
    
    double targetJointCurrents[6];
    
    double targetJointMoments[6];
    
    double actualJointPositions[6];
    double actualJointVelocities[6];
    
    double actualJointCurrents[6];
    
    double jointControlCurrents[6];
    
    ofVec3f actualTcpPosition;
    ofVec3f actualTcpOrientation; // axis angle
    
    ofVec3f actualTcpSpeed;
    ofVec3f actualTcpSpeedOrientation; // ????
    
    double tcpForces[6];
    
    ofVec3f targetTcpPosition;
    ofVec3f targetTcpOrientation;
    
    ofVec3f targetTcpSpeed;
    ofVec3f targetTcpSpeedOrientation;
    
    double digitalInputBits; // should be vector of bools?
    
    double motorTemperatures[6];
    
    double controllerTimer;
    
    double testValue; // not important
    
    double robotMode; // should be int?
    /* Robot Modes: robot_ ...
     0  disconnected
     1  confirm_safety
     2  booting
     3  power_off
     4  power_on
     5  idle
     6  backdrive
     7  running
     8  updating_firmware
     */
    
    double jointModes[6]; // should be int?
    /* Joint Modes: joint_ ...
     236    shutting_down_mode
     237    part_d_calibration_mode
     238    backdrive_mode
     239    power_off_mode
     245    not_responding_mode
     246    motor_initialisation_mode
     247    booting_mode
     248    part_d_calibration_error_mode
     249    bootloader_mode
     250    calibration_mode
     252    fault_mode
     253    running_mode
     255    idle_mode
     */
    
    double safetyMode; // should be int?
    /* Safety Modes: safety_mode_ ...
     1  normal
     2  reduced
     3  protective_stop
     4  recovery
     5  safeguard_stop
     6  system_emergency_stop
     7  robot_emergency_stop
     8  violation
     9  stop
     */
    
    double unknown1[6];
    
    ofVec3f tcpAcceleration; // target or actual?
    
    double unknown2[6];
    
    double speedScaling;
    
    double linearMomentumNorm;
    
    double unknown3;
    double unknown4;
    
    double mainVoltage;
    double robotVoltage;
    double robotCurrent;
    double actualJointVoltages[6];
    
    double digitalOutputs; // should be vector of ints?
    
    double programState;
    
};


#endif /* defined(__ursa_udp__urRobot__) */
