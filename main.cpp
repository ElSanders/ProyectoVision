#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
using namespace cv;
using namespace std;


vector<Point> points;
VideoCapture camera;
Mat currentImage;
Mat grayImage;
Mat binaryImage;
vector<float> yiq(int R, int G, int B){
    vector<float> yiq;
    yiq.push_back((0.299*R+0.587*G+0.114*B)/255);
    yiq.push_back((0.596*R-0.275*G-0.321*B)/255);
    yiq.push_back((0.212*R-0.523*G+0.311*B)/255);
    return yiq;   
}

void rgbToBW(const Mat &original, Mat &gray){
    if(gray.empty())
        gray = Mat(original.rows, original.cols, original.type());
    int sum=0;
    for(int y=0; y <original.rows; y++){
        for(int x=0; x<original.cols;x++){
            sum+=((int)original.at<Vec3b>(y,x)[0]+(int)original.at<Vec3b>(y,x)[2]+(int)original.at<Vec3b>(y,x)[2]);
            sum/=3; 
            for(int i = 0; i<original.channels();i++){
                gray.at<Vec3b>(y,x)[i] = (unsigned char)sum;
            }
        }
    }
}

void binarize(const Mat &original, Mat &bin,int thresh){
    threshold(original,bin,thresh,255,THRESH_BINARY);
}

/*void makeYIQ(Mat &original){
    Mat YIQIBOI = original.clone();
    Vec3b pix;
    vector<float> yiq_vec;
    for(int j = 0; j<currentImage.cols;j++){
        for(int i = 0; i<currentImage.rows;i++){
            pix = currentImage.at<Vec3b>(j,i);
            yiq_vec= yiq((int)pix[2],(int)pix[1],(int)pix[0]);
            YIQIBOI.at<Vec3f>(i,j) = {yiq_vec[0],yiq_vec[1],yiq_vec[2]};
        }
    }
    imshow("YIQ",YIQIBOI);
}*/

void mouseClicked(int event, int x, int y, int flags, void* param){
    Vec3b pix = currentImage.at<Vec3b>(y,x);
    vector<float> yiq_vec = yiq((int)pix[2],(int)pix[1],(int)pix[0]);
    switch (event)
    {
        case EVENT_LBUTTONDOWN:
            printf("\033[2J");
            printf("\033[%d;%dH", 0, 0);
            cout << "\r\rR: " << (int)pix[2] << " G: " << (int)pix[1]<< " B: " << (int)pix[0]<<endl;
            cout << "Y: " << yiq_vec[0] << " I: " << yiq_vec[1]<< " Q: " << yiq_vec[2]<<endl;
            break;
        case EVENT_MOUSEMOVE:
            break;
        case EVENT_LBUTTONUP:
            break;
    }
}


int main(int argc, char *argv[]){
    camera.open(0);
    namedWindow("Image");
    setMouseCallback("Image", mouseClicked);
    int thresh = 0;
    bool clicked = false, run = true, sel = true;
    while (run)
	{	
		if(!clicked)
			camera >> currentImage;
		if (currentImage.data) 
		{
            rgbToBW(currentImage,grayImage);
            binarize(grayImage,binaryImage,thresh);
            if(sel)
                imshow("Image", currentImage);
            else
                imshow("Gray",grayImage);
            //imshow("Binary",binaryImage);
            switch(waitKey(3)){
                case 't':
                cout<<"Threshold: ";
                cin>>thresh;
                break;
                case 'p':
                clicked = !clicked;
                break;
                case 's':
                destroyAllWindows();
                sel = !sel;
                break;
                case 'x':
                run = false;
                break;
            }
              
		}
		else
		{
			cout << "No image data.. " << endl;
		}
	}
}




