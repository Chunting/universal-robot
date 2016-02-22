//
//  urRobot.cpp
//  ursa_udp
//
//  Created by Ben Snell on 2/19/16.
//
//

#include "urRobot.h"

void urRobot::updateRobotData(string message) {
    
    // split the string and update the correct entities
    vector< string > tempVals = ofSplitString(message, ",");
    int nValues = tempVals.size(); // should be 132
    double vals[nValues];
    for (int i = 0; i < nValues; i++) {
        vals[i] = ofToDouble(tempVals[i]);
    }
    
    elapsedTime = vals[0];
    
    copy(vals + 1, vals + 1 + 6, targetJointPositions);
    copy(vals + 7, vals + 7 + 6, targetJointVelocities);
    copy(vals + 13, vals + 13 + 6, targetJointAccelerations);
    
    copy(vals + 19, vals + 19 + 6, targetJointCurrents);
    
    copy(vals + 25, vals + 25 + 6, targetJointMoments);
    
    copy(vals + 31, vals + 31 + 6, actualJointPositions);
    copy(vals + 37, vals + 37 + 6, actualJointVelocities);
    
    copy(vals + 43, vals + 43 + 6, actualJointCurrents);
    
    copy(vals + 49, vals + 49 + 6, jointControlCurrents);
    
    actualTcpPosition = ofVec3f(vals[55], vals[56], vals[57]);
    actualTcpOrientation = ofVec3f(vals[58], vals[59], vals[60]);
    
    actualTcpSpeed = ofVec3f(vals[61], vals[62], vals[63]);
    actualTcpSpeedOrientation = ofVec3f(vals[64], vals[65], vals[66]);
    
    copy(vals + 67, vals + 67 + 6, tcpForces);
    
    targetTcpPosition = ofVec3f(vals[73], vals[74], vals[75]);
    targetTcpOrientation = ofVec3f(vals[76], vals[77], vals[78]);
    
    targetTcpSpeed = ofVec3f(vals[79], vals[80], vals[81]);
    targetTcpSpeedOrientation = ofVec3f(vals[82], vals[83], vals[84]);
    
    digitalInputBits = vals[85];
    
    copy(vals + 86, vals + 86 + 6, motorTemperatures);
    
    controllerTimer = vals[92];
    
    testValue = vals[93];
    
    robotMode = vals[94];
    
    copy(vals + 95, vals + 95 + 6, jointModes);
    
    safetyMode = vals[101];
    
    copy(vals + 102, vals + 102 + 6, unknown1);
    
    tcpAcceleration = ofVec3f(vals[108], vals[109], vals[110]);
    
    copy(vals + 111, vals + 111 + 6, unknown2);
    
    speedScaling = vals[117];
    
    linearMomentumNorm = vals[118];
    
    unknown3 = vals[119];
    unknown4 = vals[120];
    
    mainVoltage = vals[121];
    robotVoltage = vals[122];
    robotCurrent = vals[123];
    copy(vals + 124, vals + 124 + 6, actualJointVoltages);
    
    digitalOutputs = vals[130];
    
    programState = vals[131];
    
//    cout << "Updated robot with " << nValues << " of 132 values at time " << ofGetTimestampString("%H-%M-%S-%i") << endl;
    
}













