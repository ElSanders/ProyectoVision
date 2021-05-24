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
using namespace cv;
using namespace std;


vector<int> seedX,seedY,m00,m01,m20,m02,m11,m10,mu20,mu02,mu11;
vector <float> n20,n02,n11,fi1,fi2;
Vec3b pinto;
VideoCapture camera;
Mat currentImage,grayImage,binaryImage,yiqImage,segmented, binaria;
char sel = 'e';
int N = 1;
int cx1,cy1,cx2,cy2;
const float maxApplefi1   = 0.64667,     maxApplefi2   = 3.71094,
            maxPearfi1    = -1.97895e-05,maxPearfi2    = 1.82586,
            maxBanannafi1 = 5.36383e-05, maxBanannafi2 = 7.4782e-05,
            maxCarrotfi1  = 1.01629e-05, maxCarrotfi2  = 1.45773,
            minApplefi1   = 0.0887028,   minApplefi2   = 6.23816e-05, 
            minPearfi1    = -2.10041e-05,minPearfi2    = 1.82586,
            minBanannafi1 = 4.48383e-05, minBanannafi2 = 0.869049,
            minCarrotfi1  = 7.15575e-06, minCarrotfi2  = 1.44322;

bool leftRight = false; // left is false, right is true


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
        /*
         * Crear un vec3b con solo las seeds, modificar el de segment
         * y dejar esos vectores solos.
         */
        // Getting rand number in image size range
            rand_X = rand() % width + offset; // Offset it is the initial value for random
            rand_Y = rand() % height;

        if(rand_Y<original.rows-2 && rand_X<original.cols-2){
	        if (original.at<Vec3b>(rand_Y, rand_X) != fondo){
	            //cout << "Esto es una semilla" << endl;
	            bSeed = true;
	            seedX.push_back(rand_X);
	            seedY.push_back(rand_Y);
	        }
    	}
    }
    cout<<"Termina seed"<<endl;

}

bool inBounds(float val, float min, float max){
    if(val<=max && val>=min)
        return true;
    return false;
}

//Función para identificar objetos
void identify(float fi1, float fi2){

    if(inBounds(fi1,minApplefi1,maxApplefi1) && inBounds(fi2,minApplefi2,maxApplefi2)){
        cout<<"Manzana reconocida"<<endl;
    }
    else if(inBounds(fi1,minPearfi1,maxPearfi1) && inBounds(fi2,minPearfi2,maxPearfi2)){
        cout<<"Pera reconocida"<<endl;
    }
    else if(inBounds(fi1,minBanannafi1,maxBanannafi1) && inBounds(fi2,minBanannafi2,maxBanannafi2)){
        cout<<"Plátano reconocido"<<endl;
    }
    else if(inBounds(fi1,minCarrotfi1,maxCarrotfi1) && inBounds(fi2,minCarrotfi2,maxCarrotfi2)){
        cout<<"Pera reconocida"<<endl;
    }

}

//Obtiene los valores de YIQ de un pixel RGB
void yiq(const Vec3b &pix,unsigned char &Y, unsigned char &I, unsigned char &Q){
    Y=(unsigned char)(int)(0.299*(int)pix[2]+0.587*(int)pix[1]+0.114*(int)pix[0]);
    I=(unsigned char)(int)(0.596*(int)pix[2]-0.275*(int)pix[1]-0.321*(int)pix[0]+255)/2;
    Q=(unsigned char)(int)(0.212*(int)pix[2]-0.523*(int)pix[1]+0.311*(int)pix[0]+255)/2;
}

void paint(const Mat &original, Mat &segImg,int x, int y){
    //cout<<"Empieza paint"<<endl;
    
    //Vec3b pinto;  // pintar de colores
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

}

void segment(const Mat &original, Mat &segImg){
    //original.copyTo(segImg);
    int x , y;

    while(!seedX.empty()){
        x = seedX.back();
        y = seedY.back();
        seedX.pop_back();
        seedY.pop_back();
        //if(y<original.rows-2 && x<original.cols-2)
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
// -------------------------------------------------------------------------------------------------------
void busca(){
    // Getting size of image
    Size s = segmented.size();
    int height = s.height;
    int width = s.width;
       
        //inicializar momentos
        m00.push_back(0);
        m01.push_back(0);
        m10.push_back(0);
        m20.push_back(0);
        m02.push_back(0);
        m11.push_back(0);
        
        
        // Figura izquierda
        seed(segmented, height, width/2, 0);               
        segment(currentImage,segmented);        
        cout << "Area 1 = " << m00[0] <<endl;
            //centroide figura 1
        cx1 = (m10[0]/(m00[0]+ 1e-5)); //add 1e-5 to avoid division by zero
        cy1 = (m01[0]/(m00[0]+ 1e-5)); // float ?
        cout << "suma X " << m10[0] <<endl;
        cout << "suma Y " << m01[0] <<endl;
        cout << "CX " << cx1 <<endl;
        cout << "CY " << cy1 <<endl; 
            // momentos segundo orden
        mu20.push_back(m20[0] - (cx1*m10[0]));
        mu02.push_back(m02[0] - (cy1*m01[0])) ;
        mu11.push_back(m11[0] - (cy1*m10[0])) ;
        
        //momentos normalizados 
        n20.push_back((float) (mu20[0]/pow(m00[0],2)));
        n02.push_back((float) (mu02[0]/pow(m00[0],2)));
        n11.push_back((float) (mu11[0]/pow(m00[0],2)));
        
        // fi 1 y fi 2 
        fi1.push_back(n20[0]+n02[0]);
        fi2.push_back(pow((n20[0]-n02[0]),2)+4*pow(n11[0],2));
        cout << "Fi1  " << fi1[0] <<endl;
        cout << "Fi2  " << fi2[0] <<endl;
        identify(fi1[0],fi2[0]);     
        circle (segmented,Point(cx1,cy1),4,(255,0,0),-1);
        
        N++; 
        
   
        //Figura derecha
        seed(segmented, height, width, width/2);
        segment(currentImage,segmented);          
        cout << "Area 2 = " << m00[1] <<endl;
              //centroide figura 2
        cx2 = (m10[1]/(m00[1]+ 1e-5)); //add 1e-5 to avoid division by zero
        cy2 = (m01[1]/(m00[1]+ 1e-5)); // float ?
        cout << "suma X " << m10[1] <<endl;
        cout << "suma Y " << m01[1] <<endl;
        cout << "CX " << cx2 <<endl;
        cout << "CY " << cy2 <<endl; 
        
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

        cout << "Fi1  " << fi1[1] <<endl;
        cout << "Fi2  " << fi2[1] <<endl;
        identify(fi1[1],fi2[1]);
        circle (segmented,Point(cx2,cy2),4,(255,0,0),-1);  
        N=1;   
        
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

/*void autoBin (Mat original, Mat roja){
    Mat mask;
    inRange(original, Scalar(0,0,190), Scalar(10,10,255),mask);    
    currentImage.copyTo(roja,mask);
}*/

void separar(const Mat &original, Mat &editRGB){
  Mat maskRGB;
  //Reset image
  editRGB = Mat(0, 0, 0, Scalar( 0,0,0));
    
  //Filtro y máscara
  // Para foto
  inRange(original, Scalar(0,0,130), Scalar(124,131,255),maskRGB); 
  // Para Video OJO -> ajustar limites de color segun su tinta 
  //inRange(original, Scalar(0,0,130), Scalar(90,90,255),maskRGB);     
  original.copyTo(editRGB,maskRGB);
  
}


int main(int argc, char *argv[]){
    camera.open(0);
    int thresh = 0;
    bool clicked = false, run = true;
       
    //currentImage.copyTo(segmented);
    //inRange(currentImage, Scalar(minB,minG,minR), Scalar(maxB,maxG,maxR),segmented);
   
    

    while (run)
    {
        
        //currentImage.copyTo(segmented);

        if(!clicked){
     //currentImage = imread("rojo2.jpg",IMREAD_COLOR);       
     camera >> currentImage;
   
        
        }
        
        if (currentImage.data)
        {
            switch(sel){
                case 'a':
                    cout << "a";
                    namedWindow("Camera");
                    setMouseCallback("Camera", mouseClicked);
                    imshow("Camera", currentImage);
                    //imshow("Camera", segmented);
                    break;
                case 'b':
                    cout << "b";
                    namedWindow("Grayscale");
                    setMouseCallback("Grayscale", mouseClicked);
                    rgbToBW(currentImage,grayImage);
                    imshow("Grayscale",grayImage);
                    break;
                case 'c':
                    cout << "c";
                    namedWindow("Binarized");
                    setMouseCallback("Binarized", mouseClicked);
                    rgbToBW(currentImage,grayImage);
                    binarize(grayImage,binaryImage,thresh);
                    createTrackbar("Threshold","Binarized",&thresh,255,onTrackbar);
                    imshow("Binarized",binaryImage);
                    break;
                case 'd':
                    cout << "d";
                    namedWindow("YIQ");
                    setMouseCallback("YIQ", mouseClicked);
                    makeYIQ(currentImage,yiqImage);
                    imshow("YIQ",yiqImage);
                    break;
                case 'e':
                    cout << "e";
                    separar(currentImage, segmented);
                    namedWindow("Original");                    
                    busca();
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
                    cout<<"Select an image:\na)Camera\nb)Grayscale\nc)Binarized\nd)YIQ\ne)Segmentation\n";
                    cin>>sel;
                    break;
                case 'r':
                    //seed(segmented);
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
