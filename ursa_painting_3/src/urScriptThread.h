//
//  urScriptThread.h
//  ursa_painting_3
//
//  Created by Ben Snell on 3/6/16.
//
//

#ifndef __ursa_painting_3__urScriptThread__
#define __ursa_painting_3__urScriptThread__

#include "ofMain.h"

// run a python script that communicated with the controller
class urScriptThread : public ofThread {
    
public:
    
    void setup(string path, int udp_from_port, int udp_to_port, bool logging);
    
    string cmd;
    
    void threadedFunction();
    
    
};

#endif /* defined(__ursa_painting_3__urScriptThread__) */
