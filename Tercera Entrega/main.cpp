/*
Proyecto: Visión para robots
Profesor José Luis Gordillo
Equipo 5
Luis Sandro González Solalinde A01365445
Nathalie Vichis Lagunes A01364838
Cristian
Carlos
*/
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <unistd.h>
#include <bits/stdc++.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <math.h>

using namespace cv;
using namespace std;
using namespace std::chrono_literals;

VideoCapture camera;
Mat currentImage, filteredImage, animationImage,binaryImage,grayImage,kernel;
bool debug = false; 
char sel = 'c';
int N = 1;
int cx1,cy1,cx2,cy2,small,big;

bool startSelected = false;

vector<int>seedY,seedX;
int startingX = 0, startingY = 0, finishingX, finishingY; 
enum lastStep{
    LEFT,
    RIGHT,
    UP,
    DOWN
}last;

void routeFinder(Mat &image, int startX, int startY, int finishX, int finishY, int stepSize){
    cout<<"Empieza route"<<endl;
    int currentX = finishingX, currentY = finishingY;
    while(startX != currentX && startY != currentY){
    	if(abs(startX-currentX)>=100*abs(startY-currentY)){
    		if(startX-currentX > 0){
    			if(image.at<Vec3b>(currentY,currentX+stepSize)[0] > 10){
    				seedX.push_back(currentX+stepSize);
                    seedY.push_back(currentY);
                    last = RIGHT;
    			}else{
                    switch(last){
                        case UP:
                            seedX.push_back(currentX);
                            seedY.push_back(currentY+stepSize);
                        break;
                        case DOWN:
                            seedX.push_back(currentX);
                            seedY.push_back(currentY-stepSize);
                            break;
                        case RIGHT:
                            seedX.push_back(currentX+stepSize);
                            seedY.push_back(currentY);
                        break;
                        case LEFT:
                            seedX.push_back(currentX-stepSize);
                            seedY.push_back(currentY);
                        break;
                    }
                }
    		}else if(startX-currentX < 0){
    			if(image.at<Vec3b>(currentY,currentX-stepSize)[0] > 10){
    				seedX.push_back(currentX-stepSize);
                    seedY.push_back(currentY);
                    last = LEFT;
    			}else{
                    switch(last){
                        case UP:
                            seedX.push_back(currentX);
                            seedY.push_back(currentY+stepSize);
                        break;
                        case DOWN:
                            seedX.push_back(currentX);
                            seedY.push_back(currentY-stepSize);
                            break;
                        case RIGHT:
                            seedX.push_back(currentX+stepSize);
                            seedY.push_back(currentY);
                        break;
                        case LEFT:
                            seedX.push_back(currentX-stepSize);
                            seedY.push_back(currentY);
                        break;
                    }
                }

    		}
    		
    	}else{
    		if(startY-currentY >0){
    			if(image.at<Vec3b>(currentY+stepSize,currentX)[0] > 10){
    				seedX.push_back(currentX);
                    seedY.push_back(currentY+stepSize);
                    last = UP;
    			}else{
                    switch(last){
                        case UP:
                            seedX.push_back(currentX);
                            seedY.push_back(currentY+stepSize);
                        break;
                        case DOWN:
                            seedX.push_back(currentX);
                            seedY.push_back(currentY-stepSize);
                            break;
                        case RIGHT:
                            seedX.push_back(currentX+stepSize);
                            seedY.push_back(currentY);
                        break;
                        case LEFT:
                            seedX.push_back(currentX-stepSize);
                            seedY.push_back(currentY);
                        break;
                    }
                }
    		}else if(startY-currentY < 0){
    			if(image.at<Vec3b>(currentY-stepSize,currentX)[0] > 10){
    				seedX.push_back(currentX);
                    seedY.push_back(currentY-stepSize);
                    last = DOWN;
    			}else{
                    switch(last){
                        case UP:
                            seedX.push_back(currentX);
                            seedY.push_back(currentY+stepSize);
                        break;
                        case DOWN:
                            seedX.push_back(currentX);
                            seedY.push_back(currentY-stepSize);
                            break;
                        case RIGHT:
                            seedX.push_back(currentX+stepSize);
                            seedY.push_back(currentY);
                        break;
                        case LEFT:
                            seedX.push_back(currentX-stepSize);
                            seedY.push_back(currentY);
                        break;
                    }
                }

    		}
    	}
        currentX = seedX.back();
        currentY = seedY.back();
        cout<<seedX.back()<<" "<<seedY.back()<<endl;;
    }
    cout<<"Termina route"<<endl;
}

void paint(Mat &img){
    cout<<"size: "<<seedX.size()<<" "<<seedY.size()<<endl;
    while(!seedX.empty() && !seedY.empty()){
        if(seedY.back()>img.rows || seedY.back()<0 ||seedX.back()>img.cols || seedX.back()<0 ){
            seedY.pop_back();
            seedX.pop_back();
        }else{
            img.at<Vec3b>(seedY.back(),seedX.back()) = {0,0,255};
            cout<<seedY.back()<<" "<<seedX.back()<<endl;
            
            if(seedY.back()>0)img.at<Vec3b>(seedY.back()-1,seedX.back()) = {0,0,255};
            if(seedY.back()<img.rows-1)img.at<Vec3b>(seedY.back()+1,seedX.back()) = {0,0,255};
            if(seedX.back()>0)img.at<Vec3b>(seedY.back(),seedX.back()-1) = {0,0,255};
            if(seedX.back()<img.cols-1)img.at<Vec3b>(seedY.back(),seedX.back()+1) = {0,0,255};
            seedY.pop_back();
            seedX.pop_back();
        }
    }
    cout<<"Termina paint"<<endl;
    seedY.clear();
    seedX.clear();
}

//Cambia una imágen a escala de grises
void rgbToBW(const Mat &original, Mat &gray){
    if(gray.empty())
        gray = Mat(original.rows, original.cols, original.type());
    int sum=0;
    for(int y=0; y <original.rows; y++){
        for(int x=0; x<original.cols;x++){
            sum+=((int)original.at<Vec3b>(y,x)[0]+(int)original.at<Vec3b>(y,x)[2]+(int)original.at<Vec3b>(y,x)[2]);
            sum/=3;
            if(sum>255)
                sum=255;
            if(sum<0)
                sum = 0;
            for(int i = 0; i<original.channels();i++){
                gray.at<Vec3b>(y,x)[i] = (unsigned char)sum;
            }
        }
    }
}

//Binariza una imágen
void binarize(const Mat &original, Mat &bin,int thresh){
    threshold(original,bin,thresh,255,THRESH_BINARY);
}


//Función de reacción al click en la imagen
void mouseClicked(int event, int x, int y, int flags, void* param){

    Vec3b pix = currentImage.at<Vec3b>(y,x);
    unsigned char y_yiq,i_yiq,q_yiq;
    switch (event)
    {
        case EVENT_LBUTTONDOWN:
            cout << "X: " << x << " Y: "<< y <<endl;
            cout << "R: " << (int)pix[2] << " G: " << (int)pix[1]<< " B: " << (int)pix[0]<<endl;
            if(startSelected){
        		finishingY = y;
        		finishingX = x;
                startSelected = false;
        	}else{
        		startingX = x;
        		startingY = y;
                startSelected = true;
        	}
	cout<<"Starting Point "<<startingX<<" "<<startingY<<endl;
	cout<<"Final Point "<<finishingX<<" "<<finishingY<<endl;
        case EVENT_MOUSEMOVE:
            break;
        case EVENT_LBUTTONUP:
            break;  
    }
}


void onTrackbar(int, void*){
    //Función vacía para los trackbars
}


void separar(const Mat &original, Mat &editRGB){
  Mat maskRGB;
  editRGB = Mat(0, 0, 0, Scalar( 0,0,0));
  inRange(original, Scalar(100,100,100), Scalar(150,150,150),maskRGB);     
  original.copyTo(editRGB,maskRGB);
  
}


int main(int argc, char *argv[]){
    camera.open(0);
    int thresh = 0;
    bool clicked = true, run = true;
    currentImage = imread("parking.jpg",IMREAD_COLOR);
    animationImage = imread("parking.jpg",IMREAD_COLOR);
    while (run)
    {
        if(!clicked){
        //camera >> currentImage;
        }
        
        if (currentImage.data)
        {
            switch(sel){
                case 'a':
                    if(debug)cout << "a";
                    namedWindow("Camera");
                    setMouseCallback("Camera", mouseClicked);
                    imshow("Camera", currentImage);
                    break;
                case 'b':
                    if(debug)cout << "b";
                    namedWindow("Grayscale");
                    setMouseCallback("Grayscale", mouseClicked);
                    rgbToBW(currentImage,grayImage);
                    imshow("Grayscale",grayImage);
                    break;
                case 'c':
                    if(debug)cout << "c";
                    namedWindow("Filtered");
                    setMouseCallback("Filtered", mouseClicked);
                    separar(currentImage,grayImage);
                    rgbToBW(grayImage,grayImage);
                    threshold(grayImage,binaryImage,100,150,THRESH_BINARY);
					bilateralFilter(grayImage, binaryImage, 7, 100, 100);
					kernel = getStructuringElement(MORPH_RECT, Size(9,9));
					dilate(binaryImage, binaryImage, kernel);
                    kernel = getStructuringElement(MORPH_RECT, Size(9,9));
                    erode(binaryImage, binaryImage, kernel);
                    threshold(binaryImage,binaryImage,120,150,THRESH_BINARY);
                    imshow("Filtered",binaryImage);
                    if(animationImage.data)
                        imshow("Animated",animationImage);
                    break;
                default:
                    cout << "default";
                    printf("\033[2J");
                    printf("\033[%d;%dH", 0, 0);
                    cout<<"Incorrect image, please try again.\n";
                    run = false;
                    break;
            }

            switch(waitKey(3)){
                case 'p':
                    clicked = !clicked;
                    break;
                case 's':
                    destroyAllWindows();
                    printf("\033[2J");
                    printf("\033[%d;%dH", 0, 0);
                    cout<<"Select an image:\na)Camera\nb)Grayscale\nc)Binarized\nd)YIQ\ne)Segmentation\nf)Barra\n";
                    cin>>sel;
                    break;
                case 'r':
                    routeFinder(binaryImage, startingX, startingY, finishingX, finishingY, 1);
                    paint(animationImage);
                break;
                case 'x':
                    run = false;
                    break;
            }
        }else{
            cout << "No image data.. " << endl;
        }
    }
}
