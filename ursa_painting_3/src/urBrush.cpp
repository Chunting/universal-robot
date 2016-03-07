//
//  urBrush.cpp
//  ursa_painting_3
//
//  Created by Ben Snell on 3/6/16.
//
//

#include "urBrush.h"

void urBrush::setup(float _pRefillLength, float _pRefillClearance, float _distThresh) {
    
    pRefillLength = _pRefillLength;
    pRefillClearance = _pRefillClearance;
    distThresh = _distThresh;
}

// -------------------------------------------------------------------

void urBrush::addDigitalLine(ofPolyline line, bool bSimplify, int smoothingSize) {
    
    // initial simplification of line
    if (bSimplify) line.simplify();
    if (smoothingSize != 0) line = line.getSmoothed(smoothingSize);
    
    // add line to queue
    queue.push_back(line);
}

// -------------------------------------------------------------------

void urBrush::setSubmersionPoint(urRobot robot) {
    
    inInk = { robot.actualTcpPosition.x, robot.actualTcpPosition.y, robot.actualTcpPosition.z, robot.actualTcpOrientation.x, robot.actualTcpOrientation.y, robot.actualTcpOrientation.z };
    outInk = inInk;
    outInk[2] += pRefillClearance;
    
}

// -------------------------------------------------------------------

string urBrush::getPose(vector<float> pose) {
    
    int count = 0;
    stringstream ss;
    ss << "p[";
    for (int i = 0, end = MIN(pose.size(), 6); i < end; i++) {
        ss << ofToString(pose[i]);
        count++;
        if (count < 5) ss << ",";
    }
    while (count < 6) {
        ss << "0";
        count++;
        if (count < 5) ss << ",";
    }
    ss << "]";
    
    return ss.str();
}

// -------------------------------------------------------------------

void urBrush::setupDrawing(float _p2dScale, float _XYRotation, bool _bRightHanded, float _maxArcDeviation) {
    
    p2dScale = _p2dScale;
    XYRotation = _XYRotation;
    bRightHanded = _bRightHanded;
    maxArcDeviation = _maxArcDeviation;
}

// -------------------------------------------------------------------

void urBrush::setPStartPoint(urRobot robot) {

    // set physical start point
    // current tcp postion
    pStartPoint = robot.actualTcpPosition;
}

// -------------------------------------------------------------------

void urBrush::setDStartPoint() {
    
    // set digital start point
    dStartPoint = queue[0].getPointAtLength(distAlongLine);
}

// -------------------------------------------------------------------

bool urBrush::updateDrawing(urRobot robot, string &message) {
    
    if (queue.size() == 0) return false;
    
    ofVec3f nextStartPoint = queue[0].getPointAtLength(distAlongLine);
    float diff = robot.actualTcpPosition.distance(nextStartPoint);
    
    // if the robot is stopped and the current line length is zero, then issue a move command to this next startPoint
    if (robot.linearMomentumNorm == 0 && currentLineLength == 0) {
        message = "movel(" + getPose(vector<float> {nextStartPoint.x, nextStartPoint.y, nextStartPoint.z} ) + ")";
        return true;
    }
    if (diff > distThresh) return false;
    
    // update current line length
    currentLineLength = queue[0].getPerimeter();
    
    // distance is within limits to signal a new message
    // find a circular arc that fits as far as possible along curve
    float sampleDist = 0.02; // attempt to make 1 cm arc
    
    float startIndex = queue[0].getIndexAtLength(distAlongLine);
    ofVec3f startTangent = queue[0].getTangentAtIndexInterpolated(startIndex);

    // get the arc to draw
    Arc arc = findArc(nextStartPoint, startIndex, startTangent, sampleDist, queue, currentLineLength, distAlongLine, false, false, maxArcDeviation / p2dScale);
    
    // output the arc to draw in physical world. Note: this arc is translated by dStartPoint, scaled by d2p scale, and translated by pStartPoint
    float x_via = (arc.midPoint.x - dStartPoint.x) * p2dScale + pStartPoint.x;
    float y_via = (arc.midPoint.y - dStartPoint.y) * p2dScale + pStartPoint.y;
    float z_via = (arc.midPoint.z - dStartPoint.z) * p2dScale + pStartPoint.z;
    
    float x_to = (arc.stopPoint.x - dStartPoint.x) * p2dScale + pStartPoint.x;
    float y_to = (arc.stopPoint.y - dStartPoint.y) * p2dScale + pStartPoint.y;
    float z_to = (arc.stopPoint.z - dStartPoint.z) * p2dScale + pStartPoint.z;

    stringstream ss;
    ss << "movec("<< getPose(vector< float > {x_via, y_via, z_via}) << "," << getPose(vector< float > {x_to, y_to, z_to}) << ",a=" << ofToString(acc) << ",v=" << ofToString(vel) << ",r=" << ofToString(rad) << ")";
    message = ss.str();
    
    // update the currentLineLength and pop the line if need be
    currentLineLength = arc.stopDistance;
    if (arc.popLine) {
        queue.erase(queue.begin(), queue.begin() + 1);
        currentLineLength = 0;
    }
    
    return true;
}

// ------------------------------------------------------------------

Arc urBrush::findArc(ofVec3f _startPoint, float _startIndex, ofVec3f _startTangent, float _sampleDist, vector<ofPolyline> _queue, float _lineLength, float _distAlongLine, bool bNoBigger, bool bJustMadeSmaller, float _maxArcDev) {
    
    // find the nextPoint up the line at dist along line + sampleDist
    float newDistAlongLine = _distAlongLine + _sampleDist;
    
    // if the new sample distance is too big, take note of it
    if (newDistAlongLine >= _lineLength) {
        bNoBigger = true;
        newDistAlongLine = _lineLength;
        _sampleDist = _lineLength - _distAlongLine;
    }
    
    // find the next point
    ofVec3f stopPoint = _queue[0].getPointAtLength(newDistAlongLine);
    // find middle point
    ofVec3f midPoint = _queue[0].getPointAtLength(_distAlongLine + _sampleDist/2.);
    // find the arc through these three points
    ofVec3f center = getCircleCenterFromThreePoints(_startPoint, midPoint, stopPoint);
    float radius = center.distance(midPoint);
    
    // test points 1/4 and 3/4 for being within maxArcDeviation of radius
    ofVec3f oneQtrPoint = _queue[0].getPointAtLength(_distAlongLine + _sampleDist * 0.25);
    float oneQtrDeviation = abs(radius - center.distance(oneQtrPoint));
    ofVec3f threeQtrPoint = _queue[0].getPointAtLength(_distAlongLine + _sampleDist*0.75);
    float threeQtrDeviation = abs(radius - center.distance(threeQtrPoint));
    
    if (oneQtrDeviation <= _maxArcDev && threeQtrDeviation <= _maxArcDev) {
        // this arc fits
        if (bNoBigger) {
            Arc myArc;
            myArc.startPoint = _startPoint;
            myArc.midPoint = midPoint;
            myArc.stopPoint = stopPoint;
            myArc.stopDistance = newDistAlongLine;
            myArc.popLine = true;
            myArc.arcLength = _sampleDist;
            return myArc;
        } else if (bJustMadeSmaller) {
            // can't get any bigger
            Arc myArc;
            myArc.startPoint = _startPoint;
            myArc.midPoint = midPoint;
            myArc.stopPoint = stopPoint;
            myArc.stopDistance = newDistAlongLine;
            myArc.popLine = false;
            myArc.arcLength = _sampleDist;
            return myArc;
        } else {
            // we may be able to get safely bigger
            return findArc(_startPoint, _startIndex, _startTangent, _sampleDist * 2., _queue, _lineLength, _distAlongLine, bNoBigger, bJustMadeSmaller, _maxArcDev);
        }
    } else {
        // arc doesn't fit, so make it smaller
        return findArc(_startPoint, _startIndex, _startTangent, _sampleDist * 0.5, _queue, _lineLength, _distAlongLine, bNoBigger, true, _maxArcDev);
    }
    
}

// ------------------------------------------------------------------

ofVec3f urBrush::getCircleCenterFromThreePoints(ofVec3f p1, ofVec3f p2, ofVec3f p3) {
    
    float s12 = (p2.y - p1.y)/(p2.x - p1.x); // slope
    float s23 = (p3.y - p2.y)/(p3.x - p2.x);
    
    float px = (s12 * s23 * (p1.y - p3.y) + s23 * (p1.x + p2.x) - s12 * (p2.x + p3.x))/(2. * (s23 - s12));
    float py = -1/s12 * (px - (p1.x + p2.x)/2.) + (p1.y + p2.y)/2.;
    
    return ofVec3f(px, py, p2.z);
}









