//
//  urThread.h
//  ursa_udp
//
//  Created by Ben Snell on 2/20/16.
//
//

#ifndef __ursa_udp__urThread__
#define __ursa_udp__urThread__

#include "ofMain.h"

class urThread : public ofThread {
    
public:
    
    string cmd;
    
    void threadedFunction();
    
    void setScript(string path, int udp_from_port, int udp_to_port);
    
};

#endif /* defined(__ursa_udp__urThread__) */
