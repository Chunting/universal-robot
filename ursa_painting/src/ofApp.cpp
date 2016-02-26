#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    drawing.setName("Drawing");
    drawing.add(bNewDrawing.set("New Drawing", false));
    drawing.add(bDebug.set("Debug", true));
    
    panel.setup();
    panel.add(drawing);
    panel.loadFromFile("settings.xml");

}

//--------------------------------------------------------------
void ofApp::update(){
    
    // reset drawing
    if (bNewDrawing) {
        points.clear();
        bUpdateLine = false;
        line.clear();
        nPts = 0;
        bNewDrawing = false;
    }
    
    if (bUpdateLine) {
        
        
    }

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetBackgroundColor(255);
    
    // draw circles at all vertices
    ofSetColor(0);
    for (int i = 0; i < line.getVertices().size(); i++) {
        
        ofDrawCircle(line.getVertices()[i], 5);
    }

    // draw line
    line.draw();
    
    // debug
    if (bDebug) {
        ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 10, 20);
        panel.draw();
    }
    
}

//--------------------------------------------------------------
void ofApp::exit() {
    
    panel.saveToFile("settings.xml");
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if (key == 'f') ofToggleFullscreen();
    if (key == 'b') bDebug = !bDebug;

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
    
    points.push_back(ofVec3f(x, y, 0));
    if (points.size() > 2) bUpdateLine = true;
    line.curveTo(ofVec3f(x, y, 0));
    nPts++;

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
