//
//  urPath.cpp
//  ursa_painting_2
//
//  Created by Ben Snell on 2/27/16.
//
//

#include "urPath.h"

void urPath::setup(float _screenScale) {
    screenScale = _screenScale;
}

// ---------------------------------------------------------------

void urPath::setStartingPoint(ofVec3f point) {
    
    // clear the line
    line.clear();
    screenLine.clear();
    nPts = 0;
    
    // save the second point
    origin = point;
    
    // save this point as the lastpt and calculate the lastjointpos for it
    lastPt = origin;
    for (int i = 0; i < 6; i++) {
        lastJointPos.clear();
        lastJointPos.push_back(actualJointPositions[i]);
    }
    lastJointPos = getJointPos(origin);
    
    // flag that the third point be mirrored
    mirrorThirdPoint = true;
    
}

// ---------------------------------------------------------------

void urPath::addPoint(ofVec3f point) {
    
    float px = origin.x - (point.x / ofGetWidth() - 0.5) * screenScale;
    float py = origin.y + (point.y / ofGetHeight() - 0.5) * screenScale;
    
    ofVec3f newPoint(px, py, origin.z);
    
    if (mirrorThirdPoint) {
        
        // add the first, second and third point
        ofVec3f firstPt = newPoint.getRotated(180, origin, ofVec3f(0,0,1));
        line.curveTo(firstPt);
        screenLine.curveTo(point.getRotated(180, ofVec3f(ofGetWidth()/2, ofGetHeight()/2, 0), ofVec3f(0,0,1)));
        
        line.curveTo(origin);
        screenLine.curveTo(ofVec3f(ofGetWidth()/2, ofGetHeight()/2, 0));
        
        line.curveTo(newPoint);
        screenLine.curveTo(point);
        
        mirrorThirdPoint = false;
        
    } else {
        
        line.curveTo(newPoint);
        screenLine.curveTo(point);
    }
    nPts++;
    
    // reculate line length
    lineLength = line.getPerimeter();
    // reset acceleration to signal recaluclation of decceleration curve
    acc = abs(acc);
        
}

// ---------------------------------------------------------------

void urPath::drawPath(ofColor color) {
    
    if (nPts > 3) {
        ofSetColor(color);
//        ofPushMatrix();
//        ofTranslate(-origin.x, -origin.y);
//        ofScale(1/screenScale, 1/screenScale);
//        ofScale(-1, 1);
//        ofTranslate(0.5, 0.5);
//        ofScale(ofGetWidth(), ofGetHeight());
        
        screenLine.draw();
        
//        ofPopMatrix();
    }
}

// ---------------------------------------------------------------

void urPath::clearPath() {
    
    line.clear();
    screenLine.clear();
    nPts = 0;
}

// ---------------------------------------------------------------

void urPath::updateRobotData(urRobot robot) {
    
    actualTcpPosition = robot.actualTcpPosition;
    actualTcpSpeed = robot.actualTcpSpeed;
    actualTcpOrientation = robot.actualTcpOrientation;
    copy(robot.actualJointVelocities, robot.actualJointVelocities + 6, actualJointVelocities);
    copy(robot.actualJointPositions, robot.actualJointPositions + 6, actualJointPositions);
}

// ---------------------------------------------------------------

void urPath::setStartingSpeed() {
    
    // set the current speed
    curSpeed = actualTcpSpeed.length();
    
    // find the current joint positions
    
    
}

// ---------------------------------------------------------------

vector<float> urPath::outputData() {
    
    vector<float> jointSpeeds;
    
    // increment the speed if it's not already moving
    curSpeed += acc;
    // make sure the speed never gets too high
    curSpeed = CLAMP(curSpeed, -maxSpeed, maxSpeed);

    // find the projected distance along the line to move
    float projDist = curSpeed * timeStep;
//    cout << "projected distance is: " << projDist << endl;
    
    // find the point at this distance along the curve
    float thisDistAlongPath = lastDistAlongPath + projDist;
    
    cout << "speed: " << curSpeed << "  acc: " << acc << "   last dist: " << lastDistAlongPath << "   this dist: " << thisDistAlongPath << "   line length: " << lineLength <<  endl;
    
//    cout << "this much of line traversed:" << ofToString(thisDistAlongPath/lineLength) << endl;
    // check if this distance is longer than the line length
//    if (!bStopSequence && lineLength - thisDistAlongPath <= accDist) {
//        acc = -acc;
//        bStopSequence = true;
//    }
    if (thisDistAlongPath >= lineLength || curSpeed < 0) {
        // send a stop message and stop outputting data
        bFlagStop = true;
        vector<float> stopMsg({0,0,0,0,0,0});
        return stopMsg;
    }
    if (acc > 0) {
        if ((lineLength - thisDistAlongPath) < (curSpeed/acc * timeStep * curSpeed/2.)) {
            // we're within distance of decelerating, so flip the acc sign
            acc = - abs(acc);
//            bDeccel = true;
        }
    }
    
    ofVec3f targetPt = line.getPointAtLength(thisDistAlongPath);
//    cout << "target point: " << targetPt.x << " " << targetPt.y << " " << targetPt.z << endl;
    
    // find the joint positions for this point
    vector<float> targetJointPos = getJointPos(targetPt);
    
//    cout << "TARGET JOINT POSITIONS: " << targetJointPos[0] << " " << targetJointPos[1] << " " << targetJointPos[2] << " " << targetJointPos[3] << " " << targetJointPos[4] << " " << targetJointPos[5] << endl;
    
    // the joint positions for the last point are in lastjointpos
    
    // find the joint speeds for each of the joints
    for (int i = 0; i < 6; i++) {
//        if (i==0) cout << " delta angle: " << ofToString(targetJointPos[i] - lastJointPos[i]) << endl;
        float deltaAngle = ofDegToRad(targetJointPos[i] - lastJointPos[i]);
        float unclampedSpeed = deltaAngle / timeStep;
        jointSpeeds.push_back(CLAMP(unclampedSpeed, -maxAngularVelocity, maxAngularVelocity));
    }
    
    // put these speeds into the correct range (necessary?)
//    if (jointSpeeds[0] > 0) jointSpeeds[0] -= piNum;
//    if (jointSpeeds[1] > 0) jointSpeeds[1] -= piNum;
//    if (jointSpeeds[2] < 0) jointSpeeds[2] += piNum;
//    if (jointSpeeds[3] < 0) jointSpeeds[3] += piNum;
    
    // update params
    lastDistAlongPath += projDist;
    lastJointPos = targetJointPos;
    
    return jointSpeeds;
}

// ---------------------------------------------------------------

float urPath::lawOfCosineAngle(float opp, float adj1, float adj2) {
    
    float angle = acos(
                       (pow(opp,2) - pow(adj1,2) - pow(adj2,2))
                       /
                       (-2 * adj1 * adj2)
                       );
    return ofRadToDeg(angle);
    
}

// ---------------------------------------------------------------

vector<float> urPath::getJointPos(ofVec3f tcpPos) {
    
    float jointPos[6];
    
    float ES = pow(sqrt(tcpPos.x * tcpPos.x + tcpPos.y * tcpPos.y) - 0.09475, 2) + pow(tcpPos.z - 0.0067, 2);
    float EJ = 0.392;
    float JS = 0.425;
    
    float alpha = asin((tcpPos.z - 0.0067)/ES);
    
    float beta = lawOfCosineAngle(EJ, ES, JS);
    
    float mu = lawOfCosineAngle(ES, EJ, JS);
    
    jointPos[1] = - beta;
    jointPos[2] = 180 - mu;
    jointPos[3] = beta + mu - 270;
    
    jointPos[0] = ofRadToDeg(atan2(tcpPos.y, tcpPos.x)) - 180;
    
    jointPos[4] = lastJointPos[4];
    jointPos[5] = lastJointPos[5];
    
//    cout << "new joint positions are: " << jointPos[0] << " " << jointPos[1] << " " << jointPos[2] << " " << jointPos[3] << " " << jointPos[4] << " " << jointPos[5] << endl;
    
    
//    // grossly oversimplified
//    jointPos[0] = atan2(tcpPos.y, tcpPos.x);
//    
//    float radius = sqrt(tcpPos.x * tcpPos.x + tcpPos.y * tcpPos.y);
//    
//    jointPos[1] = 360 - lawOfCosineAngle(0.392, radius, 0.425);
//    
//    jointPos[2] = 180 - lawOfCosineAngle(radius, 0.425, 0.392);
//    
//    jointPos[3] = 270 - jointPos[1] - jointPos[2];
//    
//    jointPos[4] = actualJointPositions[4];
//    jointPos[5] = actualJointPositions[5];
    
    vector<float> jp({jointPos[0], jointPos[1], jointPos[2], jointPos[3], jointPos[4], jointPos[5]});
    
    return jp;
}

void urPath::reset() {
    
    line.clear();
    screenLine.clear();
    nPts = 0;
    mirrorThirdPoint = false;
    lastDistAlongPath = 0;
    lineLength = 0;
    curSpeed = 0;
    acc = abs(acc);
    lastJointPos.clear();
    bFlagStop = false;
//    bDeccel = false;
    
}

void urPath::recalculate() {
    
    // line length already recaluclated when a new point is added
    
    
    
    
    
}


