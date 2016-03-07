//
//  urThread.cpp
//  ursa_udp
//
//  Created by Ben Snell on 2/20/16.
//
//

#include "urThread.h"

void urThread::threadedFunction() {
    
    while (isThreadRunning()) {
        
        // send out messages at 125Hz
        int thisCycle = int(ofGetElapsedTimeMillis() / 8);
        if (thisCycle != lastCycle) {
            lastCycle = thisCycle;
        } else {
            continue;
        }
        
        // get data from the robot
        ur.getData();
        
        // update FPS
        ur.updateFPS();
        
        // if flag output is turned on, start calling outputData
        if (flagOutput) {
            flagOutput = false;
            bOutputActive = true;
            
            setupOutputFnct(ur.robot);
        }
        
        // outputData() will continue running so long as it returns true
        if (bOutputActive) {
            
            bOutputActive = outputFnct(ur.robot);
        }
    }
}

// ----------------------------------------------------------------------

void urThread::connect(int _commandPort, int _dataPort) {
    
    commandPort = _commandPort;
    dataPort = _dataPort;
    
    // connect the tcp and open the udp ports
    ur.setConnectionParams("127.0.0.1", commandPort, dataPort);
    ur.connect();
}

// ----------------------------------------------------------------------

void urThread::setupOutputFnct(urRobot robot) {
    
    
    
}

// ----------------------------------------------------------------------

bool urThread::outputFnct(urRobot robot) {
    
    

    
    return true;
}

// ----------------------------------------------------------------------

void urThread::closeConnections() {
    
    ur.send("q");
    ur.closeConnections();
}

