//
//  urCommThread.h
//  ursa_painting_2
//
//  Created by Ben Snell on 2/27/16.
//
//

#ifndef __ursa_painting_2__urCommThread__
#define __ursa_painting_2__urCommThread__

#include "ofMain.h"
#include "ofxNetwork.h"
#include "urComm.h"
#include "urPath.h"

class urCommThread : public ofThread {
    
public:
    
    void threadedFunction();
    
    urComm ur;

    int commandPort = 5001;
    int dataPort = 5002;
    
    int lastCycle = -1;
    
    void connect(int _commandPort, int _dataPort);
    
    void closeConnections();
    
    urPath path;
    
    // flag to start outputting data
    bool flagOutput = false;
    // bool to know whether we are actively outputting data
    bool outputActive = false;
    
};


#endif /* defined(__ursa_painting_2__urCommThread__) */
