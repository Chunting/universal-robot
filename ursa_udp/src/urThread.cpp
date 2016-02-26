//
//  urThread.cpp
//  ursa_udp
//
//  Created by Ben Snell on 2/20/16.
//
//

#include "urThread.h"

urThread::urThread() {
    
    string path = ofToDataPath("scripts/ursa_udp_3.py");
    cout << "Loading py script at " << path << endl;
    cmd = "python " + path;
    
}

void urThread::threadedFunction() {
    
    while (isThreadRunning()) {
        
        system(cmd.c_str());
        
    }
}
