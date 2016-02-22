#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetFrameRate(125); // 125Hz target for port 30003
    
    // start the python script
    thread.startThread(true);
    
    // connect the tcp and open the udp ports
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
    if (key == 'c') ur.send("close");
    if (key == 'e') ur.send("exit");
    if (key == 'o') ur.send("open");
    if (key == OF_KEY_UP) {
        ur.send("movej(p[0,0.5,0.5,2.4,2.4,2.4])");
    }
    if (key == OF_KEY_DOWN) {
        ur.send("movej(p[0,0.5,0,2.4,2.4,2.4])");
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
