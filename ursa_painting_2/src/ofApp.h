#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "urComm.h"
#include "urThread.h"
#include "ofxGui.h"
#include "urPath.h"
#include "urCommThread.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void exit();
    
    urCommThread cThread;
    urThread thread;
    
    int commandPort = 5001;
    int dataPort = 5002;
    
    ofParameterGroup robot;
    ofParameter<bool> bFreedrive;
    bool prevFreedrive = false;
    
    ofxPanel panel;
    
    // previous coordinates
    int pmx = 0;
    int pmy = 0;
		
};
