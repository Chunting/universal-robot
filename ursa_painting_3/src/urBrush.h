//
//  urBrush.h
//  ursa_painting_3
//
//  Created by Ben Snell on 3/6/16.
//
//

#ifndef __ursa_painting_3__urBrush__
#define __ursa_painting_3__urBrush__

#include "ofMain.h"
#include "urRobot.h"

// Abbreviations:
// p --> physical world
// d --> digital world

struct Arc {
    
public:
    ofVec3f startPoint;
    ofVec3f midPoint;
    ofVec3f stopPoint;
    float stopDistance;
    bool popLine;
    float arcLength;
};

class urBrush {
    
    // ---------------------
    // ------- SETUP -------
    // ---------------------

    // set brush params
    void setup(float _pRefillLength, float _pRefillClearance, float _distThresh);
    float pRefillLength; // distance brush travels to know when it has to refill on ink
    float pRefillClearance = 0.1; // height above refill point to raise the brush (clearance)
    
    // set the point at which the ink is fully submerged
    void setSubmersionPoint(urRobot robot);
    vector< float > inInk; // submerged
    vector< float > outInk; // out of ink (submerged + clearance)
    
    // ---------------------
    // ------ DRAWING ------
    // ---------------------
    
    // queue holds all polylines that are being or are to be drawn
    vector< ofPolyline > queue;
    
    // add a line or curve to the vector
    void addDigitalLine(ofPolyline line, bool bSimplify = false, int smoothingSize = 0);
    
    // setup the drawing
    void setupDrawing(float _p2dScale, float _XYRotation, bool _bRightHanded, float _maxArcDeviation);
    float p2dScale = 0.0005; // p units : d units scale (default = 0.05 m : 100 px)
    float XYRotation = 0;
    bool bRightHanded = true;
    
    // set the physical start point (tcp position)
    void setPStartPoint(urRobot robot);
    ofVec3f pStartPoint = ofVec3f(0,0,0);
    
    // set digital start point
    // distance along line of the first line in the queue
    void setDStartPoint();
    ofVec3f dStartPoint = ofVec3f(0,0,0);
    
    // draw using the brush (update the path, place along the path, and orientation of the brush at start and end points
    
    // update the drawing (if within a certain distance of next point,
    // returns true if there is a message to send
    bool updateDrawing(urRobot robot, string &message);
    float distThresh = 0.002; // distance within which the next message is "released"
    float distAlongLine = 0; // distance along the first path in the queue
    float maxArcDeviation = 0.002; // maximum deviation of the center of a perfect arc from the line its sampling
    float currentLineLength;

    Arc findArc(ofVec3f _startPoint, float _startIndex, ofVec3f _startTangent, float _sampleDist, vector<ofPolyline> _queue, float _lineLength, float _distAlongLine, bool bNoBigger, bool bJustMadeSmaller, float _maxArcDev);
    
    float acc = 1.2;
    float vel = 0.25;
    float rad = 0;
    
    
    // ---------------------
    // ------- UTILS -------
    // ---------------------
    
    string getPose(vector<float> pose);

    ofVec3f getCircleCenterFromThreePoints(ofVec3f p1, ofVec3f p2, ofVec3f p3);
    
};

#endif /* defined(__ursa_painting_3__urBrush__) */
