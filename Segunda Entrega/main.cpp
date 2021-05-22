/*
Proyecto: Visión para robots
Profesor José Luis Gordillo
Equipo 5
Luis Sandro González Solalinde A01365445
Nathalie Vichis Lagunes A01364838
Delia Itzel López Dueñas A00821792
*/
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <unistd.h>
#include <bits/stdc++.h>
using namespace cv;
using namespace std;


vector<int> seedX,seedY;
Vec3b pinto;
VideoCapture camera;
Mat currentImage,grayImage,binaryImage,yiqImage,segmented,binaria;
char sel = 'e';
int N = 1;

// Centroide
Mat src_gray;
int thresh = 100;
int thresh_binarized = 240;
RNG rng(12345);
void thresh_callback(int, void* );
void thresh_binarized_callback(int, void* );
Mat src_binarized;
const int max_thresh = 255;


//Obtiene los valores de YIQ de un pixel RGB
void yiq(const Vec3b &pix,unsigned char &Y, unsigned char &I, unsigned char &Q){
    Y=(unsigned char)(int)(0.299*(int)pix[2]+0.587*(int)pix[1]+0.114*(int)pix[0]);
    I=(unsigned char)(int)(0.596*(int)pix[2]-0.275*(int)pix[1]-0.321*(int)pix[0]+255)/2;
    Q=(unsigned char)(int)(0.212*(int)pix[2]-0.523*(int)pix[1]+0.311*(int)pix[0]+255)/2;   
}

void paint(const Mat &original, Mat &segImg,int x, int y){
	cout<<"Empieza paint"<<endl;
	//Vec3b pinto;  // color a pintar
	
	pinto[0] = 254 *(N-2)*(N-3);
	pinto[1] = 200 *(N-1)*(N-3);
	pinto[2] = 200 *(N-2)*(N-1);
	
	segImg.at<Vec3b>(y,x) = pinto;
	
	Vec3b fondo;  
        fondo[0]=0;
        fondo[1]=0;
        fondo[2]=0;
	
	/*segImg.at<Vec3b>(y,x)[0] = 254;
	segImg.at<Vec3b>(y,x)[1] = 0;
	segImg.at<Vec3b>(y,x)[2] = 0;*/
	
	int dR,dG,dB,diff;
	dB = original.at<Vec3b>(y,x)[0]-original.at<Vec3b>(y,x+1)[0];
	dG = original.at<Vec3b>(y,x)[1]-original.at<Vec3b>(y,x+1)[1];
	dR = original.at<Vec3b>(y,x)[2]-original.at<Vec3b>(y,x+1)[2];
	diff=(dR+dG+dB);
		cout<<diff<<endl;
	//if((diff<25 && diff>(-25)) && segImg.at<Vec3b>(y,x+1)!=pinto){
	if(segImg.at<Vec3b>(y,x+1)!= fondo && segImg.at<Vec3b>(y,x+1)!=pinto){
		seedX.push_back(x+1);
		seedY.push_back(y);
	}
	dB = original.at<Vec3b>(y,x)[0]-original.at<Vec3b>(y,x-1)[0];
	dG = original.at<Vec3b>(y,x)[1]-original.at<Vec3b>(y,x-1)[1];
	dR = original.at<Vec3b>(y,x)[2]-original.at<Vec3b>(y,x-1)[2];
	diff=(dR+dG+dB);
		  cout<<diff<<endl;
	//if((diff<25 && diff>(-25)) && segImg.at<Vec3b>(y,x-1)!=pinto){
	if(segImg.at<Vec3b>(y,x-1)!= fondo && segImg.at<Vec3b>(y,x-1)!=pinto){
		seedX.push_back(x-1);
		seedY.push_back(y);
	}
	dB = original.at<Vec3b>(y,x)[0]-original.at<Vec3b>(y+1,x)[0];
	dG = original.at<Vec3b>(y,x)[1]-original.at<Vec3b>(y+1,x)[1];
	dR = original.at<Vec3b>(y,x)[2]-original.at<Vec3b>(y+1,x)[2];
	diff=(dR+dG+dB);
		  cout<<diff<<endl;
	//if((diff<25 && diff>(-25)) && segImg.at<Vec3b>(y+1,x)!=pinto){
	if(segImg.at<Vec3b>(y+1,x)!= fondo && segImg.at<Vec3b>(y+1,x)!=pinto){
		seedX.push_back(x);
		seedY.push_back(y+1);
	}
	dB = original.at<Vec3b>(y,x)[0]-original.at<Vec3b>(y-1,x)[0];
	dG = original.at<Vec3b>(y,x)[1]-original.at<Vec3b>(y-1,x)[1];
	dR = original.at<Vec3b>(y,x)[2]-original.at<Vec3b>(y-1,x)[2];
	diff=(dR+dG+dB);
		  cout<<diff<<endl;
	//if((diff<25 && diff>(-25)) && segImg.at<Vec3b>(y-1,x)!=pinto){
	if(segImg.at<Vec3b>(y-1,x)!= fondo && segImg.at<Vec3b>(y-1,x)!=pinto){
		seedX.push_back(x);
		seedY.push_back(y-1);
	}

}

void segment(const Mat &original, Mat &segImg){
	//original.copyTo(segImg);
	int x , y;
	
	while(!seedX.empty()){
		x = seedX.back();
		y = seedY.back();
		seedX.pop_back();
		seedY.pop_back();
		if(y<original.rows-2 && x<original.cols-2)
			paint(original,segImg,x,y);
		
	}
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
    yiq(pix,y_yiq,i_yiq,q_yiq);
    switch (event)
    {
        case EVENT_LBUTTONDOWN:
            //printf("\033[2J");
            //printf("\033[%d;%dH", 0, 0);
            cout << "X: " << x << " Y: "<< y <<endl;
            cout << "R: " << (int)pix[2] << " G: " << (int)pix[1]<< " B: " << (int)pix[0]<<endl;
            cout << "Y: " << (int)y_yiq << " I: " << (int)i_yiq<< " Q: " << (int)q_yiq<<endl;
            if(sel == 'e'){
            	seedX.push_back(x);
            	seedY.push_back(y);
            }
            break;
        case EVENT_MOUSEMOVE:
            break;
        case EVENT_LBUTTONUP:
            break;
    }
}

void mouseClicked2(int event, int x, int y, int flags, void* param){
   
    Vec3b pix = currentImage.at<Vec3b>(y,x);
    unsigned char y_yiq,i_yiq,q_yiq;
    yiq(pix,y_yiq,i_yiq,q_yiq);
    switch (event)
    {
        case EVENT_LBUTTONDOWN:
            //printf("\033[2J");
            //printf("\033[%d;%dH", 0, 0);
            cout << "X: " << x << " Y: "<< y <<endl;
            cout << "R: " << (int)pix[2] << " G: " << (int)pix[1]<< " B: " << (int)pix[0]<<endl;
            cout << "Y: " << (int)y_yiq << " I: " << (int)i_yiq<< " Q: " << (int)q_yiq<<endl;
            if(sel == 'e'){
            	seedX.push_back(x);
            	seedY.push_back(y);
            	segment(currentImage,segmented);
            	// cambiar color 
            	/*pinto[0] = 254 *(N-2)*(N-3);
	        pinto[1] = 254 *(N-1)*(N-3);
	        pinto[2] = 254 *(N-2)*(N-1);*/
	        N++;
            	
            }
            break;
        case EVENT_MOUSEMOVE:
            break;
        case EVENT_LBUTTONUP:
            break;
    }
}


//Función para crear la imagen en fomrato YIQ
void makeYIQ(const Mat &original, Mat &destination){
    if(destination.empty())
        destination = Mat(original.rows, original.cols, original.type());
    for(int y=0; y <original.rows; y++){
        for(int x=0; x<original.cols;x++){
          yiq(original.at<Vec3b>(y,x),destination.at<Vec3b>(y,x)[0],destination.at<Vec3b>(y,x)[1],destination.at<Vec3b>(y,x)[2]);
        }
    }
}


void onTrackbar(int, void*){
  //Función vacía para los trackbars
}

//Funcion para encontrar centroide y bordes
void thresh_callback(int, void* )
{
    Mat canny_output;
    Canny( src_binarized, canny_output, thresh, thresh*3.5, 3 );
    vector<vector<Point> > contours;
    findContours( canny_output, contours, RETR_TREE, CHAIN_APPROX_SIMPLE );
    vector<Moments> mu(contours.size() );
    for( size_t i = 0; i < contours.size(); i++ )
    {
        mu[i] = moments( contours[i] );
    }
    vector<Point2f> mc( contours.size() );
    for( size_t i = 0; i < contours.size(); i++ )
    {
        //add 1e-5 to avoid division by zero
        mc[i] = Point2f( static_cast<float>(mu[i].m10 / (mu[i].m00 + 1e-5)),
                         static_cast<float>(mu[i].m01 / (mu[i].m00 + 1e-5)) );
        cout << "mc[" << i << "]=" << mc[i] << endl;
    }
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( size_t i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
        drawContours( drawing, contours, (int)i, color, 2 );
        circle( drawing, mc[i], 4, color, -1 );
    }
    imshow( "Contornos", drawing );
    cout << "\t Info: Area and Contour Length \n";
    for( size_t i = 0; i < contours.size(); i++ )
    {
        cout << " * Contorno[" << i << "] - Area (M_00) = " << std::fixed << std::setprecision(2) << mu[i].m00
             << " - Area OpenCV: " << contourArea(contours[i]) << " - Longitud: " << arcLength( contours[i], true ) << endl;
    }
}

//Para el trackbar de la imagen binaria para el centroide
void thresh_binarized_callback(int, void* ){
	threshold( src_gray, src_binarized, thresh_binarized, 255, 0);
	imshow("Binarizada", src_binarized);
}

int main(int argc, char *argv[]){
    camera.open(0);
    int thresh = 0;
    bool clicked = false, run = true;
    //currentImage = imread("eeveelutions.jpg",IMREAD_COLOR);
    //currentImage = imread("frutas2.jpg",IMREAD_COLOR);
    currentImage = imread("rojo.jpg",IMREAD_COLOR);
    //currentImage.copyTo(segmented);
    //inRange(currentImage, Scalar(minB,minG,minR), Scalar(maxB,maxG,maxR),segmented); 
    inRange(currentImage, Scalar(0,0,190), Scalar(10,10,255),binaria);
    //segmented = binaria; //CAMBIAAAR
    currentImage.copyTo(segmented,binaria);
    
    //Centroide
    CommandLineParser parser( argc, argv, "{@input | stuff.jpg | input image}" );
    Mat src = imread("rojo.jpg");
    cvtColor(src, src_gray, COLOR_BGR2GRAY );
    thresh_binarized_callback(0,0); 
    const char* source_window = "Source";
    namedWindow( source_window );
    imshow( source_window, src );
    createTrackbar( "Canny thresh:", source_window, &thresh, max_thresh, thresh_callback );
    createTrackbar( "Binary threshold:", source_window, &thresh_binarized, max_thresh, thresh_binarized_callback );
    thresh_callback( 0, 0 );
  
       
    
    while (run)
    {   
         //currentImage.copyTo(segmented);
         
        /*if(!clicked){
            camera >> currentImage;
        }*/
        if (currentImage.data) 
        {
            switch(sel){
                case 'a':
                    namedWindow("Camera");
                    setMouseCallback("Camera", mouseClicked);
                    imshow("Camera", currentImage); 
                    //imshow("Camera", segmented); 
                    break;                                
                case 'b':
                    namedWindow("Grayscale");
                    setMouseCallback("Grayscale", mouseClicked);
                    rgbToBW(currentImage,grayImage);
                    imshow("Grayscale",grayImage);
                    break;              
                case 'c':
                    namedWindow("Binarized");
                    setMouseCallback("Binarized", mouseClicked);
                    rgbToBW(currentImage,grayImage);
                    binarize(grayImage,binaryImage,thresh);
                    createTrackbar("Threshold","Binarized",&thresh,255,onTrackbar);
                    imshow("Binarized",binaryImage);
                break;  
                case 'd':
                    namedWindow("YIQ");
                    setMouseCallback("YIQ", mouseClicked);
                    makeYIQ(currentImage,yiqImage);
                    imshow("YIQ",yiqImage);
                break;
                case 'e':
                    namedWindow("Original");
                    setMouseCallback("Original", mouseClicked2);
                    imshow("Original",currentImage);
                    if(segmented.data){
 	                   imshow("Segmented",segmented);
 	                   setMouseCallback("Segmented", mouseClicked2);
 	                   }
                break;
                default:
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
                    seedY.clear();
                    seedX.clear();
                    printf("\033[2J");
                    printf("\033[%d;%dH", 0, 0);
                    cout<<"Select an image:\na)Camera\nb)Grayscale\nc)Binarized\nd)YIQ\ne)Segmentation\n";
                    cin>>sel;                  
                    break;
                case 'r':
                	segment(currentImage,segmented);
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





