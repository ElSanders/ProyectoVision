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


vector<Point> points;
vector<int> muestraR,muestraG,muestraB ;
vector<int> muestraH,muestraS,muestraV ;
vector<int> muestraY,muestraI,muestraQ ;
VideoCapture camera;
Mat currentImage,grayImage,binaryImage,hsvImage,yiqImage;
Mat histR,histG,histB, histY,histI,histQ, histH,histS,histV;
Mat NewImageYIQ,NewImageRGB,NewImageHSV;
int minR,maxR,minG,maxG,minB,maxB,val1=-1,val2=-1,val3=-1;
int minY,maxI,minQ,maxY,minI,maxQ;
int minH,maxS,minV,maxH,minS,maxV;
char sel = 'a';
bool ready = false;

//Obtiene los valores de YIQ de un pixel RGB
vector<int> yiq(int R, int G, int B){
    vector<int> yiq;
    yiq.push_back((0.299*R+0.587*G+0.114*B));
    yiq.push_back((0.596*R-0.275*G-0.321*B+255)/2);
    yiq.push_back((0.212*R-0.523*G+0.311*B+255)/2);
    return yiq;   
}

void hsv(const Mat &original, Mat &destination){
  cvtColor(original,destination,COLOR_RGB2HSV);
}

//Separar objeto    
void separar(const Mat &original, Mat &editRGB, Mat &editYIQ,Mat &editHSV){
  Mat maskRGB, maskYIQ, maskHSV;
  //Reset image
  editRGB = Mat(0, 0, 0, Scalar( 0,0,0));
  editYIQ = Mat(0, 0, 0, Scalar( 0,0,0));
  editHSV = Mat(0, 0, 0, Scalar( 0,0,0));
  
  //Filtro y máscara
  inRange(original, Scalar(minB,minG,minR), Scalar(maxB,maxG,maxR),maskRGB);     
  original.copyTo(editRGB,maskRGB);
  
  inRange(original, Scalar(minY,minI,minQ), Scalar(maxY,maxI,maxQ),maskYIQ);     
  original.copyTo(editYIQ,maskYIQ);
  
  inRange(original, Scalar(minV,minS,minH), Scalar(maxV,maxS,maxH),maskHSV);     
  original.copyTo(editHSV,maskHSV);
}


//Muestreo de imagen 
void muestreo(int event, int x, int y, int flags, void* param){
    hsv(currentImage,hsvImage);
    Vec3b pix = currentImage.at<Vec3b>(y,x);  
    Vec3b hsvPix = hsvImage.at<Vec3b>(y,x);
    vector<int> yiq_vec = yiq((int)pix[2],(int)pix[1],(int)pix[0]); 
     switch (event)
     {
        
        case EVENT_LBUTTONDOWN:
          if (muestraR.size() < 10 ) {   
            printf("\033[2J");
            printf("\033[%d;%dH", 0, 0);
            cout << "X: " << x << " Y: "<< y <<endl;
            cout << "R: " << (int)pix[2] << " G: " 
            << (int)pix[1]<< " B: " << (int)pix[0]<<endl;
            cout << "Y: " << yiq_vec[0] << " I: " << yiq_vec[1]<< " Q: " << yiq_vec[2]<<endl;
            cout << "H: " << (int)hsvPix[2] << " S: " << (int)hsvPix[1]<< " V: " << (int)hsvPix[0]<<endl;
            
            //crear vectores 
            muestraR.push_back((int)pix[2]);   
            muestraG.push_back((int)pix[1]); 
            muestraB.push_back((int)pix[0]); 
            
            muestraY.push_back(yiq_vec[0]);   
            muestraI.push_back(yiq_vec[1]); 
            muestraQ.push_back(yiq_vec[2]); 
            
            muestraH.push_back((int)hsvPix[2]);   
            muestraS.push_back((int)hsvPix[1]); 
            muestraV.push_back((int)hsvPix[0]); 
            
            //mostrar valores recolectados
            cout << muestraR[muestraR.size()-1] << endl;        
            cout << muestraG[muestraG.size()-1] << endl;           
            cout << muestraB[muestraB.size()-1] << endl; 
            
            cout << muestraY[muestraY.size()-1] << endl;        
            cout << muestraI[muestraI.size()-1] << endl;           
            cout << muestraQ[muestraQ.size()-1] << endl;
            
            cout << muestraH[muestraH.size()-1] << endl;        
            cout << muestraS[muestraS.size()-1] << endl;           
            cout << muestraV[muestraV.size()-1] << endl;
            
            //Mostrar número de muestras
            cout << muestraR.size() <<endl;                        
           }
        
        else{
           
           //valores Máximos y Mínimos
           minR= *min_element(muestraR.begin(), muestraR.end()); 
           maxR= *max_element(muestraR.begin(), muestraR.end());
           minG= *min_element(muestraG.begin(), muestraG.end()); 
           maxG= *max_element(muestraG.begin(), muestraG.end());
           minB= *min_element(muestraB.begin(), muestraB.end()); 
           maxB= *max_element(muestraB.begin(), muestraB.end());
           
           minY= *min_element(muestraY.begin(), muestraY.end()); 
           maxY= *max_element(muestraY.begin(), muestraY.end());
           minI= *min_element(muestraI.begin(), muestraI.end()); 
           maxI= *max_element(muestraI.begin(), muestraI.end());
           minQ= *min_element(muestraQ.begin(), muestraQ.end()); 
           maxQ= *max_element(muestraQ.begin(), muestraQ.end());
           
           minH= *min_element(muestraH.begin(), muestraH.end()); 
           maxH= *max_element(muestraH.begin(), muestraH.end());
           minS= *min_element(muestraS.begin(), muestraS.end()); 
           maxS= *max_element(muestraS.begin(), muestraS.end());
           minV= *min_element(muestraV.begin(), muestraV.end()); 
           maxV= *max_element(muestraV.begin(), muestraV.end());
           
           
           // solo probando que fuincione el min y max 
           //cout<< "\nMin Element = "<< minR <<endl;
           //cout << "\nMax Element = "<< maxR <<endl;
           
           //Borrar vectores
           muestraR.clear();
           muestraG.clear();
           muestraB.clear();
           
           muestraY.clear();
           muestraI.clear();
           muestraQ.clear();
           
           muestraH.clear();
           muestraS.clear();
           muestraV.clear();
           
           ready= true;        
            
           }
           
           break;
        case EVENT_MOUSEMOVE:
            break;
        case EVENT_LBUTTONUP:
            break;
       
      }
    
    
}

//Histogramas en tiempo real 
void histogram(const Mat &original, Mat &histImageR, Mat &histImageG, Mat &histImageB, int v1, int v2, int v3){
    histImageR = Mat(0, 0, 0, Scalar( 0,0,0));
    histImageG = Mat(0, 0, 0, Scalar( 0,0,0));
    histImageB = Mat(0, 0, 0, Scalar( 0,0,0));
   
             /// Separar imágen en RGB
  vector<Mat> bgr_planes;
  split( original, bgr_planes );

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
  if(histImageR.empty())
        histImageR = Mat(hist_h, hist_w, CV_8UC3, Scalar( 0,0,0));
  if(histImageG.empty())
        histImageG = Mat(hist_h, hist_w, CV_8UC3, Scalar( 0,0,0));
  if(histImageB.empty())
        histImageB = Mat(hist_h, hist_w, CV_8UC3, Scalar( 0,0,0));
        
  
             /// Normalizar a  [ 0, histImage.rows ]
  normalize(b_hist, b_hist, 0, histImageB.rows, NORM_MINMAX, -1, Mat() );
  normalize(g_hist, g_hist, 0, histImageG.rows, NORM_MINMAX, -1, Mat() );
  normalize(r_hist, r_hist, 0, histImageR.rows, NORM_MINMAX, -1, Mat() );

             /// Graficaar cada channel
  for( int i = 1; i < histSize; i++ )
  {
      line( histImageB, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
                       Scalar( 255, 0, 0), 2, 8, 0  );
      line( histImageG, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
                       Scalar( 0, 255, 0), 2, 8, 0  );
      line( histImageR, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
                       Scalar( 0, 0, 255), 2, 8, 0  );
  }
  
  if(val1!=-1){
      line(histImageR,Point(v1*512/255,0),Point(v1*512/255,400),Scalar(255,255,255),1,LINE_4,0);
      line(histImageG,Point(v2*512/255,0),Point(v2*512/255,400),Scalar(255,255,255),1,LINE_4,0);
      line(histImageB,Point(v3*512/255,0),Point(v3*512/255,400),Scalar(255,255,255),1,LINE_4,0);
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
    hsv(currentImage,hsvImage);
    Vec3b pix = currentImage.at<Vec3b>(y,x);
    Vec3b hsvPix = hsvImage.at<Vec3b>(y,x);
    vector<int> yiq_vec = yiq((int)pix[2],(int)pix[1],(int)pix[0]);
    switch (event)
    {
        case EVENT_LBUTTONDOWN:
            printf("\033[2J");
            printf("\033[%d;%dH", 0, 0);
            cout << "X: " << x << " Y: "<< y <<endl;
            cout << "R: " << (int)pix[2] << " G: " << (int)pix[1]<< " B: " << (int)pix[0]<<endl;
            cout << "Y: " << yiq_vec[0] << " I: " << yiq_vec[1]<< " Q: " << yiq_vec[2]<<endl;
            cout << "H: " << (int)hsvPix[2] << " S: " << (int)hsvPix[1]<< " V: " << (int)hsvPix[0]<<endl;
            switch(sel){
              case 'd':
                val1 = (int)pix[2];
                val2 = (int)pix[1];
                val3 = (int)pix[0];
              break;
              case 'e':
                val1 = yiq_vec[2];
                val2 = yiq_vec[1];
                val3 = yiq_vec[0];
              break;
              case 'f':
                val1 = (int)hsvPix[2];
                val2 = (int)hsvPix[1];
                val3 = (int)hsvPix[0];
              break;
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
    vector<int> yiq_vec;
    for(int y=0; y <original.rows; y++){
        for(int x=0; x<original.cols;x++){
          yiq_vec= yiq((int)original.at<Vec3b>(y,x)[2],(int)original.at<Vec3b>(y,x)[1],(int)original.at<Vec3b>(y,x)[0]);
          for(int i = 0; i<original.channels();i++){
            destination.at<Vec3b>(y,x)[i] = yiq_vec[i];
          }
        }
    }
}


void onTrackbar(int, void*){
  //Función vacía para los trackbars
}

int main(int argc, char *argv[]){
    camera.open(0);
    int thresh = 0;
    bool clicked = false, run = true;
    while (run)
    {   
        
        if(!clicked)
            camera >> currentImage;
        if (currentImage.data) 
        {
            switch(sel){
                case 'a':
                    namedWindow("Camera");
                    setMouseCallback("Camera", mouseClicked);
                    imshow("Camera", currentImage); 
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
                    namedWindow("Camera");
                    setMouseCallback("Camera", mouseClicked);
                    histogram(currentImage,histR,histG,histB,val1,val2,val3);
                    imshow("Camera", currentImage);                    
                    imshow("Histogram R",histR);
                    imshow("Histogram G",histG);
                    imshow("Histogram B",histB);  
                    break;  
                case 'e':
                    namedWindow("YIQ");
                    setMouseCallback("YIQ", mouseClicked);
                    makeYIQ(currentImage,yiqImage);
                    histogram(yiqImage,histQ,histI,histY,val1,val2,val3);
                    imshow("YIQ",yiqImage);
                    imshow("Histogram Q",histQ);
                    imshow("Histogram I",histI);
                    imshow("Histogram Y",histY);  
                break;
                case 'f':
                    namedWindow("HSV");
                    setMouseCallback("HSV", mouseClicked);
                    hsv(currentImage,hsvImage);
                    histogram(hsvImage,histH,histS,histV,val1,val2,val3);
                    imshow("HSV",hsvImage);
                    imshow("Histogram H",histH);
                    imshow("Histogram S",histS);
                    imshow("Histogram V",histV);
                break;
                case 'g':
                    if (!ready){
                      namedWindow("Camera");
                      setMouseCallback("Camera", muestreo);
                      imshow("Camera", currentImage);                   
                    }else{
                      namedWindow("Camera");
                      setMouseCallback("Camera", muestreo);
                      //namedWindow("Final");                    
                      separar(currentImage, NewImageRGB,NewImageYIQ,NewImageHSV);
                      /createTrackbar("Rmin","Final",&minR,255,onTrackbar);
                      createTrackbar("Rmax","Final",&maxR,255,onTrackbar);
                      createTrackbar("Gmin","Final",&minG,255,onTrackbar);
                      createTrackbar("Gmax","Final",&maxG,255,onTrackbar);
                      createTrackbar("Bmin","Final",&minB,255,onTrackbar);
                      createTrackbar("Bmax","Final",&maxB,255,onTrackbar);
                      imshow("Final RGB", NewImageRGB);
                      imshow("Final YIQ", NewImageYIQ);
                      imshow("Final HSV", NewImageHSV);
                      imshow("Camera", currentImage);                     
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
                    val1=-1;
                    val2=-1;
                    val3=-1;
                    printf("\033[2J");
                    printf("\033[%d;%dH", 0, 0);
                    cout<<"Select an image:\na)Camera\nb)Grayscale\nc)Binarized\nd)RGB\ne)YIQ\nf)HSV\ng)Contrast\n";
                    cin>>sel;
                    ready = false;                    
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




