/*
Proyecto: Visión para robots
Profesor José Luis Gordillo
Equipo 5
Luis Sandro González Solalinde A01365445
Nathalie Vichis Lagunes A01364838

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


vector<int> seedX,seedY,m00,m01,m20,m02,m11,m10,mu20,mu02,mu11;
vector <float> n20,n02,n11,fi1,fi2,fis1,fis2;
Vec3b pinto;
int p =0;
VideoCapture camera;
Mat currentImage, grayImage, binaryImage, yiqImage, segmented, binaria, mira, phiGraph;
bool debug = false; 
char sel = 'e';
int N = 1;
int cx1,cy1,cx2,cy2,small,big;



const float maxApplefi1   = .13,     
            maxPearfi1    = .17,    
            maxBanannafi1 = 0.24,    
            maxCarrotfi1  = .4, 
            minApplefi1   = .08,   
            minPearfi1    = .12,
            minBanannafi1 = 0.18, 
            minCarrotfi1  = .25;

bool leftRight = false; // left is false, right is true


// ************************** /Graph\ **************************
/*
 * Funcion para graficar las phi
 * Parametros:
 *      phi1: phi1 de la figura
 *      phi2: phi2 de la figura
 *
 */
void graph(float phi1, float phi2){
    // Size 841 x 480; 425 -> eje x
    // Distancia entre grid horizontal => 0.05 -> 90 px
    // Distancia entre grid vertical => 0.02 -> -35 px
    // 0.1 = 242, 425
    // 0.05 = 152, 425
    try {
        float dx = phi1 / 0.05;
        int iDxPx = dx * 90 + 90;

        float dy = phi2 / 0.02;
        int iDyPx = 425 - dy * 35;

        cout << "iDXPX = " << iDxPx << "   Phi1 = " << phi1 << endl;
        cout << "iDYPX = " << iDyPx << "   Phi2 = " << phi2 << endl;

        circle(phiGraph, Point(iDxPx, iDyPx), 4, (255, 0, 0), -1);
        imshow("Phi Graph", phiGraph);
        throw(0);
    }
    catch(int zero) {
        cout << endl;
    }

}

// ************************** /Seed\ **************************
/*
 * Funcion para encontrar la semilla a un segmento
 * Parametros:
 *      original: Matriz imagen original
 *      R, G, B: colors of the segment to look at
 *
 */
void seed(const Mat &original, int height, int width, int offset) {

    // Boolean to stay in loop until a seed is found
    bool bSeed = false;
    int rand_X;
    int rand_Y;

    Vec3b fondo;
    fondo[0]=0;
    fondo[1]=0;
    fondo[2]=0;

   while (bSeed == false ) {

        // Getting rand number in image size range
            rand_X = rand() % width + offset; // Offset it is the initial value for random
            rand_Y = rand() % height;

        if(rand_Y<original.rows-2 && rand_X<original.cols-2 && rand_X>-1 && rand_Y>-1){
	        if (original.at<Vec3b>(rand_Y, rand_X) != fondo){
	            bSeed = true;
	            seedX.push_back(rand_X);
	            seedY.push_back(rand_Y);
	        }
    	}
    }
}

bool inBounds(float val, float min, float max){
    if(val<=max && val>=min)
        return true;
    return false;
}

//Función para identificar objetos
bool identify(float fi1, float fi2){

    // Bool to identify large figure
    bool bLargeFigure;

    if(inBounds(fi1,minApplefi1,maxApplefi1) ){
        cout<<"Manzana reconocida"<<endl;
        small=1;
        bLargeFigure = false;
    }
   if(inBounds(fi1,minPearfi1,maxPearfi1) ){
        cout<<"Cereza reconocida"<<endl;
        small=2;
       bLargeFigure = false;
    }
    if(inBounds(fi1,minBanannafi1,maxBanannafi1) ){
        cout<<"Plátano reconocido"<<endl;
        big=1;
        bLargeFigure = true;
    }
    if(inBounds(fi1,minCarrotfi1,maxCarrotfi1) ){
        cout<<"Zanahoria reconocida"<<endl;
        big=2;
        bLargeFigure = true;
    }
    
    //------MIRA---pintar cuadrante
    int x, y ;
    if(debug)cout << "Valores de Small y Big SB -> " << small << big << "--------------------------"<< endl;
    if ((small == 1) && (big ==1)){ // manzana y platano
        x = 365; y = 135;
    }
    if ((small == 2) && (big ==1)){ // cherry y platano
        x = 365; y = 205;
    }
    if ((small == 1) && (big ==2)){ // manzana y zanahoria
        x = 285; y = 135;
    }
    if ((small == 2) && (big ==2)){ // cherry y zanahoria
        x = 285; y = 205;
    }

    circle (mira,Point(x,y),25,(0,0,255),-1);
    imshow("Mira", mira); // falta borrar el circulo para pintarlo en otro lado

    return bLargeFigure;
}

//Obtiene los valores de YIQ de un pixel RGB
void yiq(const Vec3b &pix,unsigned char &Y, unsigned char &I, unsigned char &Q){
    Y=(unsigned char)(int)(0.299*(int)pix[2]+0.587*(int)pix[1]+0.114*(int)pix[0]);
    I=(unsigned char)(int)(0.596*(int)pix[2]-0.275*(int)pix[1]-0.321*(int)pix[0]+255)/2;
    Q=(unsigned char)(int)(0.212*(int)pix[2]-0.523*(int)pix[1]+0.311*(int)pix[0]+255)/2;
}

void paint(const Mat &original, Mat &segImg,int x, int y){
    if(debug)cout<<"Empieza paint"<<endl;
    
    pinto[0] = 254 *(N-2)*(N-3);
    pinto[1] = 200 *(N-1)*(N-3);
    pinto[2] = 200 *(N-2)*(N-1);

    segImg.at<Vec3b>(y,x) = pinto;
    m00[N-1]= m00[N-1]+1;
    m01[N-1]= m01[N-1]+ y;
    m10[N-1]= m10[N-1]+ x;
    
    m02[N-1]= m02[N-1] + pow(y,2);
    m20[N-1]= m20[N-1] + pow(x,2);
    m11[N-1]= m11[N-1] + x*y;
       

    Vec3b fondo;
    fondo[0]=0;
    fondo[1]=0;
    fondo[2]=0;

    if(segImg.at<Vec3b>(y,x+1)!= fondo && segImg.at<Vec3b>(y,x+1)!=pinto){
        seedX.push_back(x+1);
        seedY.push_back(y);
    }

    if(segImg.at<Vec3b>(y,x-1)!= fondo && segImg.at<Vec3b>(y,x-1)!=pinto){
        seedX.push_back(x-1);
        seedY.push_back(y);
    }

    if(segImg.at<Vec3b>(y+1,x)!= fondo && segImg.at<Vec3b>(y+1,x)!=pinto){
        seedX.push_back(x);
        seedY.push_back(y+1);
    }

    if(segImg.at<Vec3b>(y-1,x)!= fondo && segImg.at<Vec3b>(y-1,x)!=pinto){
        seedX.push_back(x);
        seedY.push_back(y-1);
    }
    if(debug)cout<<"Termina paint"<<endl;
}

void segment(const Mat &original, Mat &segImg){
    if(debug)cout<<"Empieza segment"<<endl;
    int x , y;

    while(!seedX.empty()){
        x = seedX.back();
        y = seedY.back();
        seedX.pop_back();
        seedY.pop_back();
        paint(original,segImg,x,y);
    }
    if(debug)cout<<"Termina segment"<<endl;
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

// -------------------------------------------------------------------------------------------------------
void busca(){
    if(debug)cout<<"Empieza busca"<<endl;
    // Getting size of image
    
    Size s = segmented.size();
    if(debug)cout<<"Se obtiene size"<<endl;
    int height = s.height;
    int width = s.width;

    // Variables para graficar angulo
    int centerWMira = 640/2 + 5;
    int centerHMira = 360/2 - 12;

        //inicializar momentos

        m00.push_back(0);
        m01.push_back(0);
        m10.push_back(0);
        m20.push_back(0);
        m02.push_back(0);
        m11.push_back(0);
         // doble o falla el segundo centroide

        m00.push_back(0);
        m01.push_back(0);
        m10.push_back(0);
        m20.push_back(0);
        m02.push_back(0);
        m11.push_back(0);


        if(debug)cout<<"Empieza seed"<<endl;
        // Figura izquierda
        seed(segmented, height, width/2, 0);
        if(debug)cout<<"Termina seed"<<endl;
        segment(currentImage,segmented);
        if(debug)cout <<"Termina segment"<<endl;
        if(debug)cout << "Area 1 = " << m00[0] <<endl;

        //centroide figura 1
        cx1 = (m10[0]/(m00[0]+ 1e-5)); //add 1e-5 to avoid division by zero
        cy1 = (m01[0]/(m00[0]+ 1e-5)); // float ?
        if(debug)cout << "suma X " << m10[0] <<endl;
        if(debug)cout << "suma Y " << m01[0] <<endl;
        if(debug)cout << "CX " << cx1 <<endl;
        if(debug)cout << "CY " << cy1 <<endl;
        circle (segmented,Point(cx1,cy1),4,(255,0,0),-1);


            // momentos segundo orden
        mu20.push_back(m20[0] - (cx1*m10[0]));
        mu02.push_back(m02[0] - (cy1*m01[0])) ;
        mu11.push_back(m11[0] - (cy1*m10[0])) ;

        //momentos normalizados
        n20.push_back((float) (mu20[0]/pow(m00[0],2)));
        n02.push_back((float) (mu02[0]/pow(m00[0],2)));
        n11.push_back((float) (mu11[0]/pow(m00[0],2)));

        // fi 1 y fi 2
        fi1.push_back(n20[0]+n02[0]+ 1e-5);
        fi2.push_back(pow((n20[0]-n02[0]),2)+4*pow(n11[0],2)+ 1e-5);
        if(debug)cout << "Fi1  " << fi1[0] <<endl;
        if(debug)cout << "Fi2  " << fi2[0] <<endl;
        bool isLarge = identify(fi1[0],fi2[0]);
        graph(fi1[0], fi2[0]);

        // recolentacndo valores de fi para ENTRENAMIENTO, solo una figura
        fis1.push_back(fi1[0]);
        fis2.push_back(fi2[0]);
        if(debug)cout<<" , "<< fi1 [0]<< " , " << fi2 [0]  << endl;
        
        N++;



        // ************************************** /Angulo de la figura 1\ **************************************
        if (isLarge) {
            double theta = 0.5 * atan2((2*mu11.back()), mu20.back() - mu02.back());
            if(debug)cout << "Angle is " << theta << endl;
            double arrowHeadX = 100.0; // width of the figure. SET LATER WITH REAL VALUES ----------------------------------
            double arrowHeadY = tan(theta) * arrowHeadX;

            double arrowTailX = 100.0; // width of the figure. SET LATER WITH REAL VALUES ----------------------------------
            double arrowTailY = tan(theta) * arrowTailX;

            // Drawing line
            line(segmented, Point(cx1-arrowTailX, cy1 - arrowTailY), Point(cx1+arrowHeadX,
                                                                           cy1+arrowHeadY), (255, 100, 100), 3);
            // Drawing line
            line(segmented, Point(cx1+arrowTailX, cy1 - arrowTailY), Point(cx1-arrowHeadX,
                                                                           cy1+arrowHeadY), (255, 100, 100), 3);


            arrowedLine(mira, Point(centerWMira, centerHMira), Point(centerWMira+arrowTailX,
                                                                     centerHMira+arrowTailY), (255, 100, 100), 3);
        }

    //Figura derecha
        seed(segmented, height, width, width/2);
        segment(currentImage,segmented);
        if(debug)cout << "Area 2 = " << m00[1] <<endl;

        //centroide figura 2
        cx2 = (m10[1]/(m00[1]+ 1e-5)); //add 1e-5 to avoid division by zero
        cy2 = (m01[1]/(m00[1]+ 1e-5)); // float ?
        if(debug)cout << "suma X " << m10[1] <<endl;
        if(debug)cout << "suma Y " << m01[1] <<endl;
        if(debug)cout << "CX " << cx2 <<endl;
        if(debug)cout << "CY " << cy2 <<endl;
        circle (segmented,Point(cx2,cy2),4,(255,0,0),-1);


        // momentos segundo orden
        mu20.push_back(m20[1] - (cx2*m10[1]));
        mu02.push_back(m02[1] - (cy2*m01[1])) ;
        mu11.push_back(m11[1] - (cy2*m10[1])) ;


        //momentos normalizados
        n20.push_back((float) (mu20[1]/pow(m00[1],2)));
        n02.push_back((float) (mu02[1]/pow(m00[1],2)));
        n11.push_back((float) (mu11[1]/pow(m00[1],2)));
        
        // fi 1 y fi 2 
        fi1.push_back(n20[1]+n02[1]);
        fi2.push_back(pow((n20[1]-n02[1]),2)+4*pow(n11[1],2));

        if(debug)cout << "Fi1  " << fi1[1] <<endl;
        if(debug)cout << "Fi2  " << fi2[1] <<endl;
        isLarge = identify(fi1[1],fi2[1]);
        graph(fi1[1], fi2[1]);
        N=1;

        if (isLarge){
            // ************************************** /Angulo de la figura 2\ **************************************
            double theta2 = 0.5 * atan2((2*mu11.back()), mu20.back() - mu02.back());
            if(debug)cout << "Angle is " << theta2 << endl;

            double arrowHeadX2 = 100.0; // width of the figure. SET LATER WITH REAL VALUES ----------------------------------
            double arrowHeadY2 = tan(theta2) * arrowHeadX2;

            double arrowTailX2 = 100.0; // width of the figure. SET LATER WITH REAL VALUES ----------------------------------
            double arrowTailY2 = tan(theta2) * arrowTailX2;

            // Drawing line
            line(segmented, Point(cx2-arrowTailX2, cy2 - arrowTailY2), Point(cx2+arrowHeadX2,
                                                                             cy2+arrowHeadY2), (255, 100, 100), 3);
            // Drawing line
            line(segmented, Point(cx2+arrowTailX2, cy2 - arrowTailY2), Point(cx2-arrowHeadX2,
                                                                             cy2+arrowHeadY2), (255, 100, 100), 3);

            arrowedLine(mira, Point(centerWMira, centerHMira), Point(centerWMira+arrowTailX2,
                                                                     centerHMira+arrowTailY2), (255, 100, 100), 3);
        }


    m00.clear();
        m10.clear();
        m01.clear();
        m20.clear();
        m02.clear();
        m11.clear();
        mu02.clear();
        mu20.clear();
        mu11.clear();
        n20.clear();
        n02.clear();
        fi1.clear();
        fi2.clear();
        if(debug)cout<<"Termina busca"<<endl;
}


void busca2(){
    if(debug)cout<<"Empieza busca"<<endl;
    // Getting size of image
    
    Size s = segmented.size();
    if(debug)cout<<"Se obtiene size"<<endl;
    int height = s.height;
    int width = s.width;

    // Variables para graficar angulo
    double arrowXFigure, arrowYFigure;

        //inicializar momentos

        m00.push_back(0);
        m01.push_back(0);
        m10.push_back(0);
        m20.push_back(0);
        m02.push_back(0);
        m11.push_back(0);
         // doble o falla el segundo centroide

        m00.push_back(0);
        m01.push_back(0);
        m10.push_back(0);
        m20.push_back(0);
        m02.push_back(0);
        m11.push_back(0);


        if(debug)cout<<"Empieza seed"<<endl;
        // Figura izquierda
        seed(segmented, height, width/2, 0);
        if(debug)cout<<"Termina seed"<<endl;
        segment(currentImage,segmented);
        if(debug)cout <<"Termina segment"<<endl;
        if(debug)cout << "Area 1 = " << m00[0] <<endl;

        //centroide figura 1
        cx1 = (m10[0]/(m00[0]+ 1e-5)); //add 1e-5 to avoid division by zero
        cy1 = (m01[0]/(m00[0]+ 1e-5)); // float ?
        if(debug)cout << "suma X " << m10[0] <<endl;
        if(debug)cout << "suma Y " << m01[0] <<endl;
        if(debug)cout << "CX " << cx1 <<endl;
        if(debug)cout << "CY " << cy1 <<endl;
        circle (segmented,Point(cx1,cy1),4,(255,0,0),-1);


            // momentos segundo orden
        mu20.push_back(m20[0] - (cx1*m10[0]));
        mu02.push_back(m02[0] - (cy1*m01[0])) ;
        mu11.push_back(m11[0] - (cy1*m10[0])) ;


        //momentos normalizados
        n20.push_back((float) (mu20[0]/pow(m00[0],2)));
        n02.push_back((float) (mu02[0]/pow(m00[0],2)));
        n11.push_back((float) (mu11[0]/pow(m00[0],2)));

        // fi 1 y fi 2
        fi1.push_back(n20[0]+n02[0]+ 1e-5);
        fi2.push_back(pow((n20[0]-n02[0]),2)+4*pow(n11[0],2)+ 1e-5);
        if(debug)cout << "Fi1  " << fi1[0] <<endl;
        if(debug)cout << "Fi2  " << fi2[0] <<endl;
        bool isLarge = identify(fi1[0],fi2[0]);
        // recolentacndo valores de fi para ENTRENAMIENTO, solo una figura 
        
        fis1.push_back(fi1[0]);
        fis2.push_back(fi2[0]);
        if(debug)cout<<" , "<< fi1 [0]<< " , " << fi2 [0]  << endl;

    // ************************************** /Angulo de la figura 1\ **************************************
    //if (isLarge) {
        double theta = 0.5 * atan2((2*mu11.back()), mu20.back() - mu02.back());
        if(debug)cout << "Angle is " << theta << endl;
        double arrowHeadX = 100.0; // width of the figure. SET LATER WITH REAL VALUES ----------------------------------
        double arrowHeadY = tan(theta) * arrowHeadX;

        double arrowTailX = 100.0; // width of the figure. SET LATER WITH REAL VALUES ----------------------------------
        double arrowTailY = tan(theta) * arrowTailX;

        // Drawing line
        line(segmented, Point(cx1-arrowTailX, cy1 - arrowTailY), Point(cx1+arrowHeadX,
                                                                       cy1+arrowHeadY), (255, 100, 100), 3);
        // Drawing line
        line(segmented, Point(cx1+arrowTailX, cy1 - arrowTailY), Point(cx1-arrowHeadX,
                                                                       cy1+arrowHeadY), (255, 100, 100), 3);
    //}
         
        
        m00.clear();
        m10.clear();
        m01.clear();
        m20.clear();
        m02.clear();
        m11.clear();
        mu02.clear();
        mu20.clear();
        mu11.clear();
        n20.clear();
        n02.clear();
        fi1.clear();
        fi2.clear();
        if(debug)cout<<"Termina busca"<<endl;
}

int buscaWrapped()
{
    std::mutex m;
    std::condition_variable cond;
    int retValue;

    std::thread t([&cond, &retValue]() 
    {
        busca();
        cond.notify_one();
    });

    t.detach();

    {
        std::unique_lock<std::mutex> l(m);
        if(cond.wait_for(l, 1s) == std::cv_status::timeout) 
            throw std::runtime_error("Timeout");
    }

    return retValue;    
}


int buscaWrapped2()
{
    std::mutex m;
    std::condition_variable cond;
    int retValue;

    std::thread t([&cond, &retValue]() 
    {
        busca2();
        cond.notify_one();
    });

    t.detach();

    {
        std::unique_lock<std::mutex> l(m);
        if(cond.wait_for(l, 1s) == std::cv_status::timeout) 
            throw std::runtime_error("Timeout");
    }

    return retValue;    
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


void separar(const Mat &original, Mat &editRGB){
  Mat maskRGB;
  //Reset image
  editRGB = Mat(0, 0, 0, Scalar( 0,0,0));
  inRange(original, Scalar(0,0,125), Scalar(90,90,255),maskRGB);     
  original.copyTo(editRGB,maskRGB);
  
}


int main(int argc, char *argv[]){
    camera.open(0);
    int thresh = 0;
    bool clicked = false, run = true;

    while (run)
    {
        if(!clicked){
        // "platCer.jpeg"
        // "rojo2.jpg"
        // "zanCer.jpeg"
        // "platCer.jpeg"
        string imageFruit = "platCer.jpeg";
        //currentImage = imread(imageFruit,IMREAD_COLOR);
        camera >> currentImage;
        mira = imread("mira2.jpg",IMREAD_COLOR);
        phiGraph = imread("phiGraph.jpeg", IMREAD_COLOR);

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
                    namedWindow("Binarized");
                    setMouseCallback("Binarized", mouseClicked);
                    rgbToBW(currentImage,grayImage);
                    binarize(grayImage,binaryImage,thresh);
                    createTrackbar("Threshold","Binarized",&thresh,255,onTrackbar);
                    imshow("Binarized",binaryImage);
                    break;
                case 'd':
                    if(debug)cout << "d";
                    namedWindow("YIQ");
                    setMouseCallback("YIQ", mouseClicked);
                    makeYIQ(currentImage,yiqImage);
                    imshow("YIQ",yiqImage);
                    break;
                case 'f':
                    if(debug)cout << "e";
                    separar(currentImage, segmented);
                    namedWindow("Original");
                    try{
                        buscaWrapped2();
                    }
                    catch(runtime_error& e){
                        cout<<"Timed out, trying again."<<endl;
                    }
                    imshow("Original",currentImage);
                    imshow("Segmented",segmented);
                    break;
                case 'e':
                    if(debug)cout << "e";
                    separar(currentImage, segmented);
                    namedWindow("Original");
                    try{
                        buscaWrapped();
                    }
                    catch(runtime_error& e){
                        cout<<"Timed out, trying again."<<endl;
                    }
                    imshow("Original",currentImage);
                    imshow("Segmented",segmented);
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
                    seedY.clear();
                    seedX.clear();
                    printf("\033[2J");
                    printf("\033[%d;%dH", 0, 0);
                    cout<<"Select an image:\na)Camera\nb)Grayscale\nc)Binarized\nd)YIQ\ne)Segmentation\nf)Barra\n";
                    cin>>sel;
                    break;
                case 'x':
                    fis1.clear();
                    fis2.clear();
                    run = false;
                    break;
            }
        }else{
            cout << "No image data.. " << endl;
        }
    }
}
