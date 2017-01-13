
#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>
#include "ServerSocket.hpp"
#include "SocketException.h"
#include <thread>

std::string message = "123";

void thread1(){
    try {
        ServerSocket server(7603);
        while (true) {
            ServerSocket new_sock;
            server.accept(new_sock);
            try {
                while(true){
                    
                    new_sock >> message;
                    new_sock << message;
                    
                }
            } catch (SocketException &) {
                std::cout<<"fail"<<std::endl;
            }
        }
        
    } catch ( SocketException e) {
        std::cout<<"fail"<<std::endl;
    }
}

int main(int argc, char *argv[])
{
    bool faceState = false;
    int faceFalseCheck = 0;
    int faceTrueCheck = 0;
    
    std::thread t1(&thread1);
    // -------------------------------------------------------------------------
    // webcam routine
    cv::VideoCapture capture(0);
    
    // rasperry pi
    //cv::VideoCapture capture("http://10.30.115.21:8080/?action=stream?dummy=param.mjpg");
    
    
    if( !capture.isOpened() ) {
        std::cerr << "Could not open camera" << std::endl;
        return 0;
    }
    
    // create a window
    cv::namedWindow("webcam",1);
    
    // -------------------------------------------------------------------------
    // face detection configuration
    cv::CascadeClassifier face_classifier;
    face_classifier.load("/Users/Tony/Documents/자료/EmbeddedSystem/haarcascade_frontalface_default.xml");
    
    while (true) {
        bool frame_valid = true;
        
        cv::Mat frame_original;
        cv::Mat frame;
        
        try {
            capture >> frame_original; // get a new frame from webcam
            cv::resize(frame_original,frame,cv::Size(frame_original.cols/2,
                                                     frame_original.rows/2),0,0,CV_INTER_NN); // downsample 1/2x
        } catch(cv::Exception& e) {
            std::cerr << "Exception occurred. Ignoring frame... " << e.err
            << std::endl;
            frame_valid = false;
        }
        
        if (frame_valid) {
            if(faceState== false && faceFalseCheck == 5){
                faceState = true;
                faceFalseCheck = 0;
                faceTrueCheck = 0;
            }
            else if(faceState == false){
                faceFalseCheck=0;
                faceTrueCheck++;
            }
            message = "123";
            
            try {
                cv::Mat grayframe;
                cv::cvtColor(frame, grayframe, CV_BGR2GRAY);
                cv::equalizeHist(grayframe,grayframe);
                
                // -------------------------------------------------------------
                // face detection routine
                
                // a vector array to store the face found
                std::vector<cv::Rect> faces;
                
                face_classifier.detectMultiScale(grayframe, faces,
                                                 1.1, // increase search scale by 10% each pass
                                                 3,   // merge groups of three detections
                                                 CV_HAAR_FIND_BIGGEST_OBJECT|CV_HAAR_SCALE_IMAGE,
                                                 cv::Size(30,30));
                
                // -------------------------------------------------------------
                // draw the results
                for(int i=0; i<faces.size(); i++) {
                    cv::Point lb(faces[i].x + faces[i].width,
                                 faces[i].y + faces[i].height);
                    cv::Point tr(faces[i].x, faces[i].y);
                    
                    cv::rectangle(frame, lb, tr, cv::Scalar(0,255,0), 3, 4, 0);
                }
                
                // print the output
                cv::imshow("webcam", frame);
                
            } catch(cv::Exception& e) {
                std::cerr << "Exception occurred. Ignoring frame... " << e.err
                << std::endl;
            }
        } else {
            message = "321";
            if (faceState == true && faceTrueCheck == 5) {
                faceFalseCheck = 0;
                faceState = false;
                faceTrueCheck = 0;
            }else if ( faceState == true){
                faceFalseCheck++;
                faceTrueCheck=0;
            }
        }
        if (cv::waitKey(30) >= 0) break;
    }
    
    // VideoCapture automatically deallocate camera object
    return 0;
}