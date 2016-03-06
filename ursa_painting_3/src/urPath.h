//
//  urPath.h
//  ursa_painting_2
//
//  Created by Ben Snell on 2/27/16.
//
//

#ifndef __ursa_painting_2__urPath__
#define __ursa_painting_2__urPath__

#include "ofMain.h"
#include "urRobot.h"

class urPath {
    
public:
    
    // ------------------------
    // --------- SETUP --------
    // ------------------------
    
    // add points to this line (scaled to physical space)
    ofPolyline line;
    ofPolyline screenLine;
    int nPts = 0;
    
    // setup the path
    // set the scaling of the square screen in relationship to the robot's workspace
    void setup(float _screenScale);
    float screenScale = 0.5;
    
    // starting point is the position of the tcp
    void setStartingPoint(ofVec3f point);
//    float zHeight;
    
    void addPoint(ofVec3f point);
    bool mirrorThirdPoint = false;
    ofVec3f origin;
    
    void drawPath(ofColor color);
    
    void clearPath();
    
    ofVec3f actualTcpPosition;
    ofVec3f actualTcpOrientation;
    ofVec3f actualTcpSpeed;
    double actualJointVelocities[6];
    double actualJointPositions[6];
    
    void updateRobotData(urRobot robot);
    
    // ------------------------
    // --------- OUTPUT -------
    // ------------------------
    
    // keep track of current distance along the path
    float lastDistAlongPath = 0;
    float lineLength = 0;
    
    // current speed
    float curSpeed = 0;
    // max speed (m/s)
    float maxSpeed = 0.05;
    // max angular velocity
    float maxAngularVelocity = 1.0; //1 rad/s
    
    // current acceleration (m/s/s)
    float acc = 0.001;
    
    // at the beginning of outputting data, set the starting speed
    void setStartingSpeed();
    
    // return joint speeds
    vector<float> outputData();
    
    float timeStep = 0.008; //125Hz
    
    // return the joint positions for a given TCP
    vector<float> getJointPos(ofVec3f tcpPos);
    // after evaluating a point along our path, save them so we don't have to recaluclate them later
    vector<float> lastJointPos;
    ofVec3f lastPt;
    
    // given three sides, find angle across from A
    float lawOfCosineAngle(float opp, float adj1, float adj2);
    
//    float maxAngularVelocity = 1.0;
    
    float piNum = 3.141592654;
    

    // whether we have begun stopping sequence
//    bool bStopSequence = false;

    bool bFlagStop = false;
    
    void reset();
    
    // ------------------------
    // ----- DECELERATION -----
    // ------------------------
    
    // deceleration in progress
//    bool bDeccel = false;
    
    // distance from the end of the path within which a path is considered finished
//    float finishDist = 0.0; // 1 mm
    
    // distance within which to begin deceleration
//    float decDist = 0.02; // 2 cm
    
    // -----------------------------------
    // ----- LIVE PATH RECALCULATION -----
    // -----------------------------------
    
    // reculate after new point is added
    void recalculate();
    
};

#endif /* defined(__ursa_painting_2__urPath__) */
