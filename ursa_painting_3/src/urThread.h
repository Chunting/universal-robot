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
#include "ofxNetwork.h"
#include "urComm.h"
#include "urPath.h"

// main thread which manages sending out messages at 125 Hz using a urComm object
class urThread : public ofThread {
    
public:

    // function that runs at 125Hz
    void threadedFunction();
    int lastCycle = -1;
    
    // -------------------------
    // -------- START UP -------
    // -------------------------
    
    // ur communication object that handles sending formatted data and parsing received data
    urComm ur;
    
    // connect to the python script
    void connect(int _commandPort, int _dataPort);
    // default communication ports
    int commandPort = 5001;
    int dataPort = 5002;
    
    // -------------------------
    // ------ COMMUNICATE ------
    // -------------------------
    
    // flag to start outputting data
    bool flagOutput = false;
    // bool to know whether we are actively outputting data
    bool bOutputActive = false;
    
    // prepare to start outputting data
    void setupOutputFnct(urRobot robot);
    
    // output data
    // function that runs when flag output is turned on. stops running when outputData returns false
    bool outputFnct(urRobot robot);
    
    // -------------------------
    // -------- SHUTDOWN -------
    // -------------------------
    
    // close python connection --> call this before shutting down thread to prevent ports from staying open
    void closeConnections();
    
};

#endif /* defined(__ursa_udp__urThread__) */
