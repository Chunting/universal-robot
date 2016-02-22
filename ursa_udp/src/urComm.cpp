//
//  urComm.cpp
//  ursa_udp
//
//  Created by Ben Snell on 2/19/16.
//
//

#include "urComm.h"

urComm::urComm(string _hostName, int _portReceive, int _portSend) {
    
    hostName = _hostName;
    portReceive = _portReceive;
    portSend = _portSend;
    
}

// -------------------------------------------------------------------

void urComm::connect() {
    
    // setup receiver to stream robot data back to OF
    udpReceive.Create();
    udpReceive.Bind(portReceive);
    udpReceive.SetNonBlocking(bNonBlocking);
    
    // setup sender to stream to python program in order to send to robot
    udpSend.Create();
    udpSend.Connect(hostName.c_str(), portSend);
    udpSend.SetNonBlocking(bNonBlocking);
    
//    udpReceive.SetTimeoutReceive(1);
    
//    cout <<  udpReceive.GetTimeoutReceive() << endl;
    
}

// -------------------------------------------------------------------

bool urComm::getData() {
    
    int bytesReceived = 1;
    while (bytesReceived > 0) {
        // get the data
        unsigned int bufSize = 100000;
        char buf[bufSize];
        bytesReceived = udpReceive.Receive(buf, bufSize);
        string msg;
        if (bytesReceived > 0) {
            cout << "received data" << endl;
            msg = buf;
            
            // add this update time to the fps counter
            updateTimes.push_back(ofGetElapsedTimeMillis());
            
            robot.updateRobotData(msg);
        }
    }
    
    return (bytesReceived > 0);
}

// -------------------------------------------------------------------

void urComm::updateFPS() {
    
    // update vector
    float timeNow = ofGetElapsedTimeMillis();
    int nTimes = updateTimes.size();
    for (int i = 0; i < nTimes; i++) {
        if (updateTimes[0] + 1000 < timeNow) {
            updateTimes.erase(updateTimes.begin());
        } else {
            break;
        }
    }
    
    // refresh rate
    nTimes = updateTimes.size();
    fps = fps * 0.5 + float(nTimes) * 0.5;
}

// -------------------------------------------------------------------

void urComm::closeConnections() {
    
    // send close message to python script to close its sockets
    send("close");
    
    // close connections to python script
    udpReceive.Close();
    udpSend.Close();

}

// -------------------------------------------------------------------

// no need to send "\n"
void urComm::send(string message) {
    
    // send message as a null terminated string
    int bytesSent = udpSend.Send(message.c_str(), message.length());
    
    cout << "Send message with " << bytesSent << " bytes: \"" << message << "\"" << endl;
    
}

// -------------------------------------------------------------------

void urComm::drawSystemValues(int px, int py) {
    
    stringstream ss;
    ss << setprecision(3) << fixed;
    
    // SYSTEM INFO
    ss << "\nSYSTEM INFO\n";
    ss << "Time:\t\t\t" << robot.elapsedTime << "\n";
    ss << "Controller Timer:\t" << robot.controllerTimer << "\n";
    ss << setprecision(0);
    ss << "Robot Mode:\t\t" << robot.robotMode << "\n";
    ss << "Safety Mode:\t\t" << robot.safetyMode << "\n";
    ss << "Program State:\t\t" << robot.programState << "\n";
    ss << "Joint Modes:\t\t" << robot.jointModes[0] << "\t" << robot.jointModes[1] << "\t" << robot.jointModes[2] << "\t" << robot.jointModes[3] << "\t" << robot.jointModes[4] << "\t" << robot.jointModes[5] << "\n";
    
    ss << setprecision(3) << fixed;
    
    // TOOL CENTER POINT
    ss << "\nTCP MOTION\n";
    ss << "Position: \t (T) \t" << robot.targetTcpPosition.x << "\t" << robot.targetTcpPosition.y << "\t" << robot.targetTcpPosition.z << "\t" << robot.targetTcpOrientation.x << "\t" << robot.targetTcpOrientation.y << "\t" << robot.targetTcpOrientation.z << "\n";
    ss << " \t \t (A) \t" << robot.actualTcpPosition.x << "\t" << robot.actualTcpPosition.y << "\t" << robot.actualTcpPosition.z << "\t" << robot.actualTcpOrientation.x << "\t" << robot.actualTcpOrientation.y << "\t" << robot.actualTcpOrientation.z << "\n";
    ss << "Speed: \t \t (T) \t" << robot.targetTcpSpeed.x << "\t" << robot.targetTcpSpeed.y << "\t" << robot.targetTcpSpeed.z << "\t" << robot.targetTcpSpeedOrientation.x << "\t" << robot.targetTcpSpeedOrientation.y << "\t" << robot.targetTcpSpeedOrientation.z << "\n";
    ss << "\t \t (A) \t" << robot.actualTcpSpeed.x << "\t" << robot.actualTcpSpeed.y << "\t" << robot.actualTcpSpeed.z << "\t" << robot.actualTcpSpeedOrientation.x << "\t" << robot.actualTcpSpeedOrientation.y << "\t" << robot.actualTcpSpeedOrientation.z << "\n";
    ss << "Acceleration: \t \t" << robot.tcpAcceleration.x << "\t" << robot.tcpAcceleration.y << "\t" << robot.tcpAcceleration.z << "\n";
    
    ss << "Forces: \t \t" << robot.tcpForces[0] << "\t" << robot.tcpForces[1] << "\t" << robot.tcpForces[2] << "\t" << robot.tcpForces[3] << "\t" << robot.tcpForces[4] << "\t" << robot.tcpForces[5] << "\n";
    
    ss << "Speed Scaling: \t \t" << robot.speedScaling << "\n";
    ss << "LinearMomentumNorm: \t" << robot.linearMomentumNorm << "\n";
    
    // JOINT MOTION INFO
    ss << "\nJOINT MOTION\n";
    ss << "Position: \t (T) \t" << robot.targetJointPositions[0] << "\t" << robot.targetJointPositions[1] << "\t" << robot.targetJointPositions[2] << "\t" << robot.targetJointPositions[3] << "\t" << robot.targetJointPositions[4] << "\t" << robot.targetJointPositions[5] << "\n";
    ss << "\t \t (A) \t" << robot.actualJointPositions[0] << "\t" << robot.actualJointPositions[1] << "\t" << robot.actualJointPositions[2] << "\t" << robot.actualJointPositions[3] << "\t" << robot.actualJointPositions[4] << "\t" << robot.actualJointPositions[5] << "\n";
    ss << "Velocity: \t (T) \t" << robot.targetJointVelocities[0] << "\t" << robot.targetJointVelocities[1] << "\t" << robot.targetJointVelocities[2] << "\t" << robot.targetJointVelocities[3] << "\t" << robot.targetJointVelocities[4] << "\t" << robot.targetJointVelocities[5] << "\n";
    ss << "\t \t (A) \t" << robot.actualJointVelocities[0] << "\t" << robot.actualJointVelocities[1] << "\t" << robot.actualJointVelocities[2] << "\t" << robot.actualJointVelocities[3] << "\t" << robot.actualJointVelocities[4] << "\t" << robot.actualJointVelocities[5] << "\n";
    ss << "Acceleration: \t (T) \t" << robot.targetJointAccelerations[0] << "\t" << robot.targetJointAccelerations[1] << "\t" << robot.targetJointAccelerations[2] << "\t" << robot.targetJointAccelerations[3] << "\t" << robot.targetJointAccelerations[4] << "\t" << robot.targetJointAccelerations[5] << "\n";
    
    ss << "Target Moments: \t" << robot.targetJointMoments[0] << "\t" << robot.targetJointMoments[1] << "\t" << robot.targetJointMoments[2] << "\t" << robot.targetJointMoments[3] << "\t" << robot.targetJointMoments[4] << "\t" << robot.targetJointMoments[5] << "\n";

    // SYSTEM ENERGY INFO
    ss << "\nSYSTEM ENERGY\n";
    
    ss << "Main Voltage: \t \t" << robot.mainVoltage << "\n";
    ss << "Robot Voltage: \t \t" << robot.robotVoltage << "\n";
    ss << "Robot Current: \t \t" << robot.robotCurrent << "\n";
    
    ss << "Joint Current: \t (T) \t" << robot.targetJointCurrents[0] << "\t" << robot.targetJointCurrents[1] << "\t" << robot.targetJointCurrents[2] << "\t" << robot.targetJointCurrents[3] << "\t" << robot.targetJointCurrents[4] << "\t" << robot.targetJointCurrents[5] << "\n";
    ss << "\t \t (A) \t" << robot.actualJointCurrents[0] << "\t" << robot.actualJointCurrents[1] << "\t" << robot.actualJointCurrents[2] << "\t" << robot.actualJointCurrents[3] << "\t" << robot.actualJointCurrents[4] << "\t" << robot.actualJointCurrents[5] << "\n";
    
    ss << "Joint Ctrl Currents: \t" << robot.jointControlCurrents[0] << "\t" << robot.jointControlCurrents[1] << "\t" << robot.jointControlCurrents[2] << "\t" << robot.jointControlCurrents[3] << "\t" << robot.jointControlCurrents[4] << "\t" << robot.jointControlCurrents[5] << "\n";
    
    ss << "Motor Temperatures: \t" << robot.jointControlCurrents[0] << "\t" << robot.jointControlCurrents[1] << "\t" << robot.jointControlCurrents[2] << "\t" << robot.jointControlCurrents[3] << "\t" << robot.jointControlCurrents[4] << "\t" << robot.jointControlCurrents[5] << "\n";

    // DIGITAL I/O
    ss << "\nDIGITAL I/O\n";
    ss << "Digital Input Bits: \t" << robot.digitalInputBits << "\n";
    ss << "Digital Output Bits: \t" << robot.digitalInputBits << "\n";
    
    ofDrawBitmapString(ss.str(), px, py);
    
}











