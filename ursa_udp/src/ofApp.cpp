#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetFrameRate(125); // 125Hz target for port 30003
    
    // start the python script
    thread.setScript("scripts/ursa_udp_5.py", commandPort, dataPort);
    thread.startThread(true);
    
    // connect the tcp and open the udp ports
    ur.setConnectionParams("127.0.0.1", commandPort, dataPort);
    ur.connect();
    
}

//--------------------------------------------------------------
void ofApp::update(){

    // get data from the robot
    ur.getData();
    ur.updateFPS();
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(255);
    
    ofSetColor(0);
    
    // draw robot data to screen
    ur.drawSystemValues(10, 35);
    
    // fps
    stringstream ss;
    ss << setprecision(3) << fixed << "App FPS: " << ofGetFrameRate() << "\t\t" << "Data FPS: " << ur.fps;
    ofDrawBitmapStringHighlight(ss.str(), 10, 20);
}

//--------------------------------------------------------------
void ofApp::exit() {

    // on exit, send message to py server to close its connections
    ur.send("exit");
    // then close own connections
    ur.closeConnections();
    
    // stop running the py server
    thread.stopThread();
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if (key == 's') ur.send("Hi there. I'm alive!!!");
    if (key == 'c') ur.send("c");
    if (key == 'q') ur.send("q");
    if (key == 'o') ur.send("o");
    if (key == OF_KEY_UP) {
        ur.send("movej(p[0,0.5,0.2,3.14,0,0])");
    }
    if (key == OF_KEY_DOWN) {
        ur.send("movej(p[0,0.5,0.1,2.4,2.4,2.4])");
    }

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
//    float px = x/ofGetWidth() * 0.5;
//    float py = (1-(y - ofGetHeight()/2.)/(ofGetHeight()/2.)) * 0.5;
//    ofVec3f point(px, 0.5, 0.35 + py);
//    stringstream ss;
//    ss << setprecision(3) << "movej(p[" << point.x << "," << point.y << "," << point.z << ",2.4,2.4,2.4])";
//    ur.send(ss.str());

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
