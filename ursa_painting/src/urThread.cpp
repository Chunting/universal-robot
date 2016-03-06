//
//  urThread.cpp
//  ursa_udp
//
//  Created by Ben Snell on 2/20/16.
//
//

#include "urThread.h"

void urThread::setScript(string path, int udp_from_port, int udp_to_port) {
    
    string dataPath = ofToDataPath(path);
    cout << "Loading py script at " << dataPath << endl;
    cmd = "python " + dataPath + " " + ofToString(udp_from_port) + " " + ofToString(udp_to_port);
    cout << cmd << endl;
    
}

void urThread::threadedFunction() {
    
    while (isThreadRunning()) {
        
        system(cmd.c_str());
        
    }
}
