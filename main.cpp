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


vector<int> seedX,seedY,m00;
Vec3b pinto;
VideoCapture camera;
Mat currentImage,grayImage,binaryImage,yiqImage,segmented, binaria;
char sel = 'e';
int N = 1;

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

    Vec3b fondo;
    fondo[0]=0;
    fondo[1]=0;
    fondo[2]=0;

    /*segImg.at<Vec3b>(y,x)[0] = 254;
    segImg.at<Vec3b>(y,x)[1] = 0;
    segImg.at<Vec3b>(y,x)[2] = 0;*/

    /*int dR,dG,dB,diff;
    dB = original.at<Vec3b>(y,x)[0]-original.at<Vec3b>(y,x+1)[0];
    dG = original.at<Vec3b>(y,x)[1]-original.at<Vec3b>(y,x+1)[1];
    dR = original.at<Vec3b>(y,x)[2]-original.at<Vec3b>(y,x+1)[2];
    diff=(dR+dG+dB);
    cout<<diff<<endl;*/
    //if((diff<25 && diff>(-25)) && segImg.at<Vec3b>(y,x+1)!=pinto){
    if(segImg.at<Vec3b>(y,x+1)!= fondo && segImg.at<Vec3b>(y,x+1)!=pinto){
        seedX.push_back(x+1);
        seedY.push_back(y);
        m00[N-1]++;
    }
    /*dB = original.at<Vec3b>(y,x)[0]-original.at<Vec3b>(y,x-1)[0];
    dG = original.at<Vec3b>(y,x)[1]-original.at<Vec3b>(y,x-1)[1];
    dR = original.at<Vec3b>(y,x)[2]-original.at<Vec3b>(y,x-1)[2];
    diff=(dR+dG+dB);
    cout<<diff<<endl;*/
    //if((diff<25 && diff>(-25)) && segImg.at<Vec3b>(y,x-1)!=pinto){
    if(segImg.at<Vec3b>(y,x-1)!= fondo && segImg.at<Vec3b>(y,x-1)!=pinto){
        seedX.push_back(x-1);
        seedY.push_back(y);
        m00[N-1]++;
    }
    /*dB = original.at<Vec3b>(y,x)[0]-original.at<Vec3b>(y+1,x)[0];
    dG = original.at<Vec3b>(y,x)[1]-original.at<Vec3b>(y+1,x)[1];
    dR = original.at<Vec3b>(y,x)[2]-original.at<Vec3b>(y+1,x)[2];
    diff=(dR+dG+dB);
    cout<<diff<<endl;*/
    //if((diff<25 && diff>(-25)) && segImg.at<Vec3b>(y+1,x)!=pinto){
    if(segImg.at<Vec3b>(y+1,x)!= fondo && segImg.at<Vec3b>(y+1,x)!=pinto){
        seedX.push_back(x);
        seedY.push_back(y+1);
        m00[N-1]++;
    }
    /*dB = original.at<Vec3b>(y,x)[0]-original.at<Vec3b>(y-1,x)[0];
    dG = original.at<Vec3b>(y,x)[1]-original.at<Vec3b>(y-1,x)[1];
    dR = original.at<Vec3b>(y,x)[2]-original.at<Vec3b>(y-1,x)[2];
    diff=(dR+dG+dB);
    cout<<diff<<endl;*/
    //if((diff<25 && diff>(-25)) && segImg.at<Vec3b>(y-1,x)!=pinto){
    if(segImg.at<Vec3b>(y-1,x)!= fondo && segImg.at<Vec3b>(y-1,x)!=pinto){
        seedX.push_back(x);
        seedY.push_back(y-1);
        m00[N-1]++;
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
       
        m00.push_back(0);
        m00.push_back(0);
        
        seed(segmented, height, width/2, 0);               
        segment(currentImage,segmented);        
        cout << "Area 1 = " << m00[0] <<endl;
        N++; 
   
        seed(segmented, height, width, width/2);
        segment(currentImage,segmented);        
        cout << "Area 2 = " << m00[1] <<endl;   
        N=1;   
        
        m00.clear();
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
  //inRange(original, Scalar(0,0,130), Scalar(10,10,255),maskRGB); 
  // Para Video OJO -> ajustar limites de color segun su tinta 
  inRange(original, Scalar(0,0,130), Scalar(90,90,255),maskRGB);     
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




