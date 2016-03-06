//
//  urComm.h
//  ursa_udp
//
//  Created by Ben Snell on 2/19/16.
//
//

#ifndef __ursa_udp__urComm__
#define __ursa_udp__urComm__

#include "ofMain.h"
#include "ofxNetwork.h"
#include "urRobot.h"

// communicate with the UR5 robotic arm over the real-time client, port 30003
class urComm {
    
public:
    
    // initialize a ur communication object with connection info
    void setConnectionParams(string _hostName, int _portReceive, int _portSend);
    string hostName;
    int portReceive;
    int portSend;
    
    // connect the udp
    void connect();
    
    // udp connections
    ofxUDPManager udpReceive;
    ofxUDPManager udpSend;
    
    // non-blocking or not?
    bool bNonBlocking = true;
    
    // get data (true if some kind of data is received)
    bool getData();
    
    // holds robot info
    urRobot robot;
    
    // update fps of data received
    float fps = 0.;
    vector<unsigned int> updateTimes; // update times in the last second
    void updateFPS();
    
    // call this at exit to close the connections
    void closeConnections();
    
    // send a string message to the robot (don't need "\n')
    void send(string message);
    
    void drawSystemValues(int px, int py);
    
    
    
};

#endif /* defined(__ursa_udp__urComm__) */
