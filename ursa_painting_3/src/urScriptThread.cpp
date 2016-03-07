//
//  urScriptThread.cpp
//  ursa_painting_3
//
//  Created by Ben Snell on 3/6/16.
//
//

#include "urScriptThread.h"

// set the python script from which we'll communicate to the robot
void urScriptThread::setup(string path, int udp_from_port, int udp_to_port, bool logging) {
    
    string dataPath = ofToDataPath(path);
    cout << "Loading py script at " << dataPath << endl;
    cmd = "python " + dataPath + " " + ofToString(udp_from_port) + " " + ofToString(udp_to_port) + " " + ofToString(int(logging));
    
}

void urScriptThread::threadedFunction() {
    
    while (isThreadRunning()) {
        
        system(cmd.c_str());
        
    }
}
