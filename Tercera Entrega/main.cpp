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
Mat currentImage, filteredImage,color, animationImage,binaryImage,grayImage,kernel,linea;
bool debug = false; 
bool pxy = false;
char sel = 'c';
int N = 1;
int cx1,cy1,cx2,cy2,small,big,px,py;

bool startSelected = false;

vector<int>pathY,pathX;
vector<int> lista1x, lista1y, lista2x,lista2y;

int startingX = 0, startingY = 0, finishingX, finishingY; 
enum lastStep{
    LEFT,
    RIGHT,
    UP,
    DOWN
}last;

/*void routeFinder(Mat &image, int startX, int startY, int finishX, int finishY, int stepSize){
    cout<<"Empieza route"<<endl;
    int currentX = finishingX, currentY = finishingY;
    while(startX != currentX || startY != currentY){
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
}*/

void paint(Mat &img, int x, int y){
    if(y>0)img.at<Vec3b>(x-1,x) = {0,0,255};
    if(y<img.rows-1)img.at<Vec3b>(y+1,x) = {0,0,255};
    if(x>0)img.at<Vec3b>(y,x-1) = {0,0,255};
    if(x<img.cols-1)img.at<Vec3b>(y,x+1) = {0,0,255};
    pathY.push_back(y);
    pathX.push_back(x);
     
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
        	
        	px=x;   py=y;
        	pxy = true;
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
  inRange(original, Scalar(100,100,100), Scalar(195,195,195),maskRGB);     
  original.copyTo(editRGB,maskRGB);
  
}


//Función para Elevación
void pozo(){
  
    int x= finishingX; //Punto más bajo
    int y= finishingY;
    float gota = 1 ;
    
    
    color=binaryImage;
    Vec3b  pinto;    
    
    
    pinto[0]=10;
    pinto[1]=0; // canal para gota
    pinto[2]=10;
    
    
    Vec3b fondo;    
    fondo[0]=0;
    fondo[1]=0;
    fondo[2]=0;
    
    int region=0;
    
    int d4[4][2] = {{1,0},{0,1},{-1,0},{0,-1}}; 
    
    // seleccionar semilla
    vector<int> s= {x,y}; 
    // pintar semilla
    color.at<Vec3b>(Point(s[0],s[1]))= pinto;
    //guargar coordenada de semilla en lista 1
    lista1x.push_back (s[0]);
    lista1y.push_back (s[1]);           
            
    while (region==0) {   // si región no está terminada 
       pinto[0]=10;
       pinto[1]=gota; // canal para gota AQUI fLOAT
       pinto[2]=10;
              
          //evaluar elementos de lista 1
          for(int y=0; y < (lista1x.size()); y++){ 
            // distancia 4
            for(int r=0; r<4; r++){ 
                // Si no está pintado
                if (color.at<Vec3b>(Point(lista1x[y]+d4[r][0],lista1y[y]+d4[r][1]))[0] != pinto[0]){ 
                  // si es objeto
                  if (color.at<Vec3b>(Point(lista1x[y]+d4[r][0],lista1y[y]+d4[r][1])) != fondo){ 
                   // pintar
                   color.at<Vec3b>(Point(lista1x[y]+d4[r][0],lista1y[y]+d4[r][1])) = pinto; 
                   //guardar coordenadas 
                   lista2x.push_back(lista1x[y]+d4[r][0]);
                   lista2y.push_back(lista1y[y]+d4[r][1]);
                                       
                  }
                }                
            }        
                         
           }
           // Si la región no está terminada
           if (lista2x.size() !=0){
              lista1x = lista2x;
              lista1y = lista2y;
              lista2x.clear();
              lista2y.clear();
              static bool doble = false;
              if ( !doble ){              
              gota=gota+1;
              doble=true;}
              else doble=false;
              //cout << pinto[1] <<endl;
             
           }
           // Si la región ya está terminada
           else{
              lista1x.clear();
              lista1y.clear();
              region = 1;                         
              
           }
       }
       
     
}
   
//pintar linea en imagen original
void camino(){ 
  
    int x= startingX; //Punto más bajo
    int y= startingY;
        
    color=binaryImage;
    linea=animationImage;
    Vec3b  pinto;    
        
    pinto[0]=255;
    pinto[1]=0; 
    pinto[2]=0;
    
    Vec3b fondo;    
    fondo[0]=0;
    fondo[1]=0;
    fondo[2]=0;
      
    int d4[4][2] = {{1,0},{0,1},{-1,0},{0,-1}}; 
    int d42[4][2] = {{2,0},{0,2},{-2,0},{0,-2}}; 
    bool siguiente = false;
     
    float inicio= color.at<Vec3b>(Point(x,y))[1];
    
    float sig= inicio-1;
    
    cout << "x inicial " << x <<endl;
    cout << "y inicial " << y <<endl;
    cout << "inicio " << inicio <<endl;
    
    cout << "sig " << sig <<endl;
    
   while((x!=finishingX) || (y!=finishingY)){
    
    for(int r=0; r<4; r++){ 
      static int m=1; 
      if (!siguiente){
	    if ((color.at<Vec3b>(Point(x+d4[r][0],y+d4[r][1]))[1] < inicio)&& (color.at<Vec3b>(Point(x+d4[r][0],y+d4[r][1]))!= fondo)){
	    	siguiente = true;
	        x=x+d4[r][0];
	        y=y+d4[r][1];
	        paint(linea,x,y);
	         //if(sig!=0)
	        sig = sig-1;
	        cout << "avanzo" <<endl;   
	        inicio = color.at<Vec3b>(Point(x,y))[1];
	        cout << "x nueva " << x <<endl;
	        cout << "y nueva " << y <<endl ;
	        cout << "inicio Nuevo " << inicio <<endl;     
	     }
       }  
                 
      }
      if (!siguiente){
          for(int r=0; r<4; r++){ 
      static int m=1; 
      if (!siguiente){
       if ((color.at<Vec3b>(Point(x+d42[r][0],y+d42[r][1]))[1] < inicio)&& (color.at<Vec3b>(Point(x+d42[r][0],y+d42[r][1]))!= fondo)){
         siguiente = true;
         x=x+d42[r][0];
         y=y+d42[r][1];         
         paint(linea,x,y);
         if(sig!=0) sig = sig-1;
         cout << "avanzo2" <<endl;   
         inicio = color.at<Vec3b>(Point(x,y))[1];
         cout << "x nueva " << x <<endl;
         cout << "y nueva " << y <<endl ;
         cout << "inicio Nuevo " << inicio <<endl;     
       }}}
      }
      siguiente = false;
      //imshow("Linea",linea);
     // cout << inicio <<endl; 
      }
      
      cout << "termina camino" <<endl;
      
     
}




int main(int argc, char *argv[]){
    camera.open(0);
    int thresh = 0;
    bool clicked = true, run = true;
    currentImage = imread("parking2.jpg",IMREAD_COLOR);
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
                    
                    kernel = getStructuringElement(MORPH_RECT, Size(5,5));
                    erode(binaryImage, binaryImage, kernel);
                    kernel = getStructuringElement(MORPH_RECT, Size(5,5));
		     		dilate(binaryImage, binaryImage, kernel);
                    threshold(binaryImage,binaryImage,120,150,THRESH_BINARY);
                    imshow("Filtered",binaryImage);
                    if(animationImage.data)
                        imshow("Animated",animationImage);
                    break;
                case 'd':
                    if(debug)cout << "d";
                    
                    namedWindow("Filtered");
                    setMouseCallback("Filtered", mouseClicked);
                    separar(currentImage,grayImage);                     
                    rgbToBW(grayImage,grayImage);                    
                    threshold(grayImage,binaryImage,100,150,THRESH_BINARY);
					bilateralFilter(grayImage, binaryImage, 7, 100, 100);
                    
                    kernel = getStructuringElement(MORPH_RECT, Size(5,5));
                    erode(binaryImage, binaryImage, kernel);
                    kernel = getStructuringElement(MORPH_RECT, Size(5,5));
		     		dilate(binaryImage, binaryImage, kernel);
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
                    
                    if (pxy){
                    pozo();
                   	camino();
                   	reverse(pathX.begin(),pathX.end());
					reverse(pathY.begin(),pathY.end());   
					/*while(!pathX.empty()){
						cout<<pathX.back()<<endl;
						pathX.pop_back();
					} */               	
                    imshow("Elevation",color);
                    imshow("Linea",linea);
                    }
                   	if(sel == 'd'){
                   		//Aquí lo de la mira
                   	}
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
