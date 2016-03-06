//
//  urCommThread.cpp
//  ursa_painting_2
//
//  Created by Ben Snell on 2/27/16.
//
//

#include "urCommThread.h"

void urCommThread::threadedFunction() {
    
    while (isThreadRunning()) {
        
        // send out at 125Hz
        int thisCycle = int(ofGetElapsedTimeMillis() / 8);
        if (thisCycle != lastCycle) {
            lastCycle = thisCycle;
        } else {
            continue;
        }
        
        // get data from the robot
        ur.getData();
        ur.updateFPS();
        
        // update path's info
        path.updateRobotData(ur.robot);
        
        if (flagOutput) {
            flagOutput = false;
            path.setStartingSpeed();
            outputActive = true;
        }
        
        if (outputActive) {
            vector<float> jointVels = path.outputData();
            if (path.bFlagStop) {
                cout << "FLAG A STOP!" << endl;
                outputActive = false;
                path.bFlagStop = false;
                cout << "STOP" << endl;
                path.reset();
                ur.send("stopj(a=1)");
                continue;
            }
//            cout << jointVels[0] << " " << jointVels[1] << " " << jointVels[2] << " "<< jointVels[3] << " " << jointVels[4] << " " << jointVels[5] << endl;
            string command = "speedj([" + ofToString(jointVels[0]) + "," + ofToString(jointVels[1]) + "," + ofToString(jointVels[2]) + "," + ofToString(jointVels[3]) + "," + ofToString(jointVels[4]) + "," + ofToString(jointVels[5]) + "],a=1.2)";
            ur.send(command, false);
        }
        
        
    }
}

void urCommThread::connect(int _commandPort, int _dataPort) {
    
    commandPort = _commandPort;
    dataPort = _dataPort;
    
    // connect the tcp and open the udp ports
    ur.setConnectionParams("127.0.0.1", commandPort, dataPort);
    ur.connect();
    
}

void urCommThread::closeConnections() {
    
    ur.send("q");
    ur.closeConnections();
}

