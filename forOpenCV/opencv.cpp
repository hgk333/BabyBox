#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>

char ack[] = "alramOFF";
char quit[] = "quitAll";
int alramFlag = 0;
int ackFlag = 1;

void error(char *msg)
{
	fprintf(stderr,"%s: %s\n",msg, strerror(errno));
	exit(1);
}

int open_listener_socket(){

	int s = socket(PF_INET,SOCK_STREAM,0);	//server socket

	if(s == -1)
		error("can not open server socket");

	return s;
}

void bind_to_port(int socket, int port){
	struct sockaddr_in name; //server port config

	name.sin_family = AF_INET;
	name.sin_port = (in_port_t)htons(30000);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	int reuse = 1;

	if(setsockopt(socket, SOL_SOCKET,SO_REUSEADDR,(char *) &reuse, sizeof(int)) == -1)
		error("can not use socket\n");


	int c = bind(socket, (struct sockaddr *) &name, sizeof(name)); // binding = select server port

	if(c == -1)
		error("can not binding"); 

}
int say(int socket, char *msg){ // send msg

	write(socket,msg,strlen(msg));
}


int read_in(int socket, char *buf, int len){

	int str_len =0;
	while((str_len = read(socket,buf,len)) !=0){ // read server msg
		if(strncmp(buf, ack, 8) == 0) { // quit == buf
			ackFlag = 1;
		}else if(strncmp(buf, quit, 7) == 0) { // quit == buf
			return 1;
		}
		buf[str_len]='\0';
	}	
	return 0;
}

void* read_thread(void* d_sock){

	int socket = (int)d_sock;
	char buf[20];
	while(socket != -1){
		if(read_in(socket,buf,20)) // read msg  return value = 1 -> buf = quit
			break;
	}

}
void* say_thread(void* d_sock){

	int socket = (int)d_sock;
	char buf[10]="alramON";
	while(socket != -1){
        if(alramFlag){ //send msg
			say(socket,buf);
            alramFlag = 0;
        }
	}
	
}

void handle_shutdown(int socket){

	if(socket)
		close(socket);

	fprintf(stderr,"byebye!!\n");
	exit(0);

}


int main(void){
	struct sockaddr_storage client_addr;
	unsigned int address_size = sizeof(client_addr);
	int listener_d = open_listener_socket();	//server socket
	char buf[256];
	int connect_d;
	void* result;
	pthread_t r_thread;
	pthread_t w_thread;
    int faceData[30];
    int faceDataCnt = 0;

    for(int i = 0 ; i < 30 ; i ++)
        faceData[i] = 1;

	bind_to_port(listener_d,30000);

	if(listen(listener_d, 10) == -1)  // queue size 10
		error("can not open listener");

	connect_d = accept(listener_d,(struct sockaddr *)&client_addr,&address_size); // server wait client

	if(connect_d == -1)
		error("can not open client socket");
	if(pthread_create(&r_thread,NULL,read_thread,(void*) connect_d) ==-1)
		error("can not create read thread");
	if(pthread_create(&w_thread,NULL,say_thread,(void*) connect_d) ==-1)
		error("can not create say thread");

	pthread_join(r_thread,&result);
	pthread_join(w_thread,&result);

    cv::VideoCapture capture(0);
    
    if( !capture.isOpened() ) {
        std::cerr << "Could not open camera" << std::endl;
        return 0;
    }
    
    // -------------------------------------------------------------------------
    // face detection configuration
    cv::CascadeClassifier face_classifier;
    face_classifier.load("haarcascade_frontalface_default.xml");
    
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
                if(ackFlag){
                    faceData[faceDataCnt++] = faces.size();
                    if(faceDataCnt==30)
                        faceDataCnt = 0;
                    
                    int alramCheck =0 ;
                    for(int i = 0 ; i<30; i++)
                        if(faceData[i]==0)
                            alramCheck++;
                    if(alramCheck > 20){
                        ackFlag = 0;
                        alramFlag = 1;
                    }
                }
                
            } catch(cv::Exception& e) {
                std::cerr << "Exception occurred. Ignoring frame... " << e.err
                << std::endl;
            }
        }
        if (cv::waitKey(30) >= 0) break;
    }

	close(connect_d);

	handle_shutdown(listener_d);

	return 0;
}