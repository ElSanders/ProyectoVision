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
using namespace cv;
using namespace std;


vector<Point> points;
VideoCapture camera;
Mat currentImage;
Mat grayImage;
Mat binaryImage;
Mat hist;

//Histograma RGB en tiempo real 
void histogram(const Mat &original, Mat &histImage){
    Mat src, dst;
    src = original;
    histImage = Mat(0, 0, 0, Scalar( 0,0,0));
   
             /// Separar imágen en RGB
  vector<Mat> bgr_planes;
  split( src, bgr_planes );

              /// Número de bins (256) y rango de 0 a 256
  int histSize = 256;
  float range[] = { 0, 256 } ;
  const float* histRange = { range };

  bool uniform = true; bool accumulate = false;

  Mat b_hist, g_hist, r_hist;

              /// Crear histogramas
  calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );

              // graficar histograma R G y B
  int hist_w = 512; int hist_h = 400;
  int bin_w = cvRound( (double) hist_w/histSize );

  //Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
  if(histImage.empty())
        histImage = Mat(hist_h, hist_w, CV_8UC3, Scalar( 0,0,0));
  
             /// Normalizar a  [ 0, histImage.rows ]
  normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
  normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
  normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

             /// Graficaar cada channel
  for( int i = 1; i < histSize; i++ )
  {
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
                       Scalar( 255, 0, 0), 2, 8, 0  );
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
                       Scalar( 0, 255, 0), 2, 8, 0  );
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
                       Scalar( 0, 0, 255), 2, 8, 0  );
  }

  usleep(1);

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

//Obtiene los valores de YIQ de un pixel RGB
vector<float> yiq(int R, int G, int B){
    vector<float> yiq;
    yiq.push_back((0.299*R+0.587*G+0.114*B)/255);
    yiq.push_back((0.596*R-0.275*G-0.321*B)/255);
    yiq.push_back((0.212*R-0.523*G+0.311*B)/255);
    return yiq;   
}

//Función de reacción al click en la imagen
void mouseClicked(int event, int x, int y, int flags, void* param){
    Vec3b pix = currentImage.at<Vec3b>(y,x);
    vector<float> yiq_vec = yiq((int)pix[2],(int)pix[1],(int)pix[0]);
    switch (event)
    {
        case EVENT_LBUTTONDOWN:
            printf("\033[2J");
            printf("\033[%d;%dH", 0, 0);
            cout << "X: " << x << " Y: "<< y <<endl;
            cout << "R: " << (int)pix[2] << " G: " << (int)pix[1]<< " B: " << (int)pix[0]<<endl;
            cout << "Y: " << yiq_vec[0] << " I: " << yiq_vec[1]<< " Q: " << yiq_vec[2]<<endl;
            break;
        case EVENT_MOUSEMOVE:
            break;
        case EVENT_LBUTTONUP:
            break;
    }
}

//Función para crear la imagen en fomrato YIQ (Work in progress)
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

int main(int argc, char *argv[]){
    camera.open(0);
    namedWindow("Camera");
    setMouseCallback("Camera", mouseClicked);
    int thresh = 0;
    char sel = 'a';
    bool clicked = false, run = true;
    while (run)
    {   
        if(!clicked)
            camera >> currentImage;
        if (currentImage.data) 
        {
            switch(sel){
                case 'a':
                    imshow("Camera", currentImage);
                    break;
                case 'b':
                    rgbToBW(currentImage,grayImage);
                    imshow("Grayscale",grayImage);
                    break;
                case 'd':
                    histogram(currentImage,hist);
                    imshow("Camera", currentImage);                    
                    imshow("Histogram",hist);
                    break;                
                case 'c':
                    rgbToBW(currentImage,grayImage);
                    binarize(grayImage,binaryImage,thresh);
                    imshow("Binarized",binaryImage);
                break;
                default:
                    printf("\033[2J");
                    printf("\033[%d;%dH", 0, 0);
                    cout<<"Incorrect image, please try again.";
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
                    cout<<"Select an image:\na)Camera\nb)Grayscale\nc)Binarized\nd)Histogram\n";
                    cin>>sel;
                    if(sel == 'c'){
                        printf("\033[2J");
                        printf("\033[%d;%dH", 0, 0);
                        cout<<"Threshold: ";
                        cin>>thresh;
                    }
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




