/*
Proyecto: Visión para robots
Profesor José Luis Gordillo
Equipo 5
Luis Sandro González Solalinde A01365445
Nathalie Vichis Lagunes A01364838
Cristian Aurelio Ramírez Anzaldo A01066337
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


/*
    	  __^__                                      __^__
         ( ___ )------------------------------------( ___ )
          | / |                                      | \ |
          | / |        Variables para camino         | \ |
          |___|                                      |___|
         (_____)------------------------------------(_____)
*/

VideoCapture camera;
Mat currentImage, filteredImage,color, animationImage,binaryImage,grayImage,kernel,linea;
bool debug = false; 
bool pxy = false;
bool angled = false;
bool playAnimation = false;
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

/*
    	  __^__                                      __^__
         ( ___ )------------------------------------( ___ )
          | / |                                      | \ |
          | / |        Variables para la mira        | \ |
          |___|                                      |___|
         (_____)------------------------------------(_____)
*/

vector<int> seedX,seedY,m00,m01,m20,m02,m11,m10,mu20,mu02,mu11;
vector <float> n20,n02,n11,fi1,fi2,fis1,fis2;
Vec3b pinto;
Mat currentImageMira, segmentedMira;
double theta = 45;
int entranceX = 46, entranceY = 31;

const float maxApplefi1   = .13,
        maxPearfi1    = .17,
        maxBanannafi1 = 0.24,
        maxCarrotfi1  = .4,
        minApplefi1   = .08,
        minPearfi1    = .12,
        minBanannafi1 = 0.18,
        minCarrotfi1  = .25;


/*
                               ......
                            .:||||||||:.
                           /            \
                          (   o      o   )
                --@@@@----------:  :----------@@@@--
                                MIRA
                               FRUTAS
 */



/*
    	  __^__                                      __^__
         ( ___ )------------------------------------( ___ )
          | / |                                      | \ |
          | / |                inBounds              | \ |
          |___|                                      |___|
         (_____)------------------------------------(_____)
 * Funcion para detectar que las phi estén dentro de los límites
 * reconocidos
 * Parametros:
 *      val: valor actual
 *      min, max: valores máximos y mínimos de la phi
 *
 */
bool inBounds(float val, float min, float max){
    if(val<=max && val>=min)
        return true;
    return false;
}

/*
    	  __^__                                      __^__
         ( ___ )------------------------------------( ___ )
          | / |                                      | \ |
          | / |                IDENTIFY              | \ |
          |___|                                      |___|
         (_____)------------------------------------(_____)

 * Funcion para hacer el reconocimiento de la figura
 * También imprime y ajusta el ángulo
 *
 * Parametros:
 *      fi1, f2: valores fi de la figura mostrada en camara
 *
*/
void identify(float fi1, float fi2){

    // adjusting angle
    int newTheta;
    if(inBounds(fi1,minBanannafi1,maxBanannafi1) ){
        //cout<<"Zanahoria reconocida"<<endl;
        if (-theta < 0){
            newTheta = 360 - (-theta*180)/(M_PI);
        } else {
            newTheta = (-theta*180)/(M_PI);
        }

        cout << "El angulo es: " << theta*180/M_PI << endl;
    }

//    // revisa si la figura es una zanahoria
//    if(inBounds(fi1,minCarrotfi1,maxCarrotfi1) ){
//        //cout<<"Zanahoria reconocida"<<endl;
//        if (-theta < 0){
//            newTheta = 360 - (-theta*180)/(M_PI);
//        } else {
//            newTheta = (-theta*180)/(M_PI);
//        }
//
//        cout << "El angulo es: " << newTheta << endl;
//    }



}


/*
    	  __^__                                      __^__
         ( ___ )------------------------------------( ___ )
          | / |                                      | \ |
          | / |                 PAINT                | \ |
          |___|                                      |___|
         (_____)------------------------------------(_____)
 * Funcion para pintar los segmentos
 *
 * Parametros:
 *      original: matriz imagen original
 *      segImg: Imagen segmentada
 *      x, y: coordenadas del punto a cambiar de  color
 *
 */
void paintMira(const Mat &original, Mat &segImg,int x, int y){
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



/*
    	  __^__                                      __^__
         ( ___ )------------------------------------( ___ )
          | / |                                      | \ |
          | / |                SEGMENT               | \ |
          |___|                                      |___|
         (_____)------------------------------------(_____)
 * Funcion con datos de semilla y segmento
 *
 * Parametros:
 *      original: matriz imagen original
 *      segImg: Imagen segmentada
 *
 */

void segmentMira(const Mat &original, Mat &segImg){
    if(debug)cout<<"Empieza segment"<<endl;
    int x , y;

    while(!seedX.empty()){
        x = seedX.back();
        y = seedY.back();
        seedX.pop_back();
        seedY.pop_back();
        paintMira(original,segImg,x,y);
    }
    if(debug)cout<<"Termina segment"<<endl;
}


/*
    	  __^__                                      __^__
         ( ___ )------------------------------------( ___ )
          | / |                                      | \ |
          | / |                  SEED                | \ |
          |___|                                      |___|
         (_____)------------------------------------(_____)
 * Funcion para encontrar la semilla a un segmento
 * Parametros:
 *      original: Matriz imagen original
 *      height, width: alto y ancho de la figura
 *
 */
void seedMira(const Mat &original, int height, int width, int offSet) {

    // Boolean to stay in loop until a seed is found
    bool bSeed = false;
    int rand_X;
    int rand_Y;

    Vec3b fondo;
    fondo[0]=0;
    fondo[1]=0;
    fondo[2]=0;

    while (!bSeed) {

        // Getting rand number in image size range
        rand_X = rand() % width + offSet; // Offset it is the initial value for random
        rand_Y = rand() % height;

        if(rand_Y<original.rows-2 && rand_X<original.cols-2 && rand_X>-1 && rand_Y>-1){
            if (original.at<Vec3b>(rand_Y, rand_X) != fondo){
                bSeed = true;
                seedX.push_back(rand_X);
                seedY.push_back(rand_Y);
            }
        }

        if (!bSeed) {
            rand_X = rand() % int((0.75*width)) + int((0.25*width)); // Offset it is the initial value for random
            rand_Y = rand() % int((0.25*height)) + int((0.75*height));
            if(rand_Y<original.rows-2 && rand_X<original.cols-2 && rand_X>-1 && rand_Y>-1){
                if (original.at<Vec3b>(rand_Y, rand_X) != fondo){
                    bSeed = true;
                    seedX.push_back(rand_X);
                    seedY.push_back(rand_Y);
                }
            }
        }
    }
}



/*
    	  __^__                                      __^__
         ( ___ )------------------------------------( ___ )
          | / |                                      | \ |
          | / |               buscaMira              | \ |
          |___|                                      |___|
         (_____)------------------------------------(_____)
 * Funcion para buscar los segmentos
 * Parametros:
 *      none
 *
 */
void buscaMira(){
    if(debug)cout<<"Empieza busca"<<endl;
    // Getting size of image

    Size s = segmentedMira.size();
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
    seedMira(segmentedMira, height, width/2, 0);
    if(debug)cout<<"Termina seed"<<endl;

    segmentMira(currentImageMira,segmentedMira);
    if(debug)cout <<"Termina segment"<<endl;

    if(debug)cout << "Area 1 = " << m00[0] <<endl;

    //centroide figura 1
    cx1 = (m10[0]/(m00[0]+ 1e-5)); //add 1e-5 to avoid division by zero
    cy1 = (m01[0]/(m00[0]+ 1e-5)); // float ?
    if(debug)cout << "suma X " << m10[0] <<endl;
    if(debug)cout << "suma Y " << m01[0] <<endl;
    if(debug)cout << "CX " << cx1 <<endl;
    if(debug)cout << "CY " << cy1 <<endl;
    circle (segmentedMira,Point(cx1,cy1),4,(255,0,0),-1);


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
    identify(fi1[0],fi2[0]);
    // recolentacndo valores de fi para ENTRENAMIENTO, solo una figura

    fis1.push_back(fi1[0]);
    fis2.push_back(fi2[0]);
    if(debug)cout<<" , "<< fi1 [0]<< " , " << fi2 [0]  << endl;


    // ************************************** /Angulo de la figura 1\ **************************************
    //if (isLarge) {
    theta = -0.5 * atan2((2*mu11.back()), mu20.back() - mu02.back());
    if(debug)cout << "Angle is " << theta << endl;
    double arrowHeadX = 100.0; // width of the figure. SET LATER WITH REAL VALUES ----------------------------------
    double arrowHeadY = tan(theta) * arrowHeadX;

    double arrowTailX = 100.0; // width of the figure. SET LATER WITH REAL VALUES ----------------------------------
    double arrowTailY = tan(theta) * arrowTailX;

    // Drawing line 0 = arrowTailX y Y
    line(segmentedMira, Point(cx1-0, cy1 - 0), Point(cx1+arrowHeadX,
                                                                   cy1-arrowHeadY), (255, 100, 100), 3);
    // Drawing line
//    line(segmentedMira, Point(cx1+arrowTailX, cy1 - arrowTailY), Point(cx1-arrowHeadX,
//                                                                   cy1+arrowHeadY), (255, 100, 100), 3);
    //}


    m00.clear(); m10.clear(); m01.clear(); m20.clear(); m02.clear();
    m11.clear(); mu02.clear(); mu20.clear(); mu11.clear();
    n20.clear(); n02.clear();
    fi1.clear(); fi2.clear();

    if(debug)cout<<"Termina busca"<<endl;
}


/*
    	  __^__                                      __^__
         ( ___ )------------------------------------( ___ )
          | / |                                      | \ |
          | / |           buscaWrappedMira           | \ |
          |___|                                      |___|
         (_____)------------------------------------(_____)
 * Funcion para buscar objeto en threads
 * Parametros:
 *      none
 *
 */
int buscaWrappedMira()
{
    std::mutex m;
    std::condition_variable cond;
    int retValue;

    std::thread t([&cond, &retValue]()
                  {
                      buscaMira();
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



/*
    	  __^__                                      __^__
         ( ___ )------------------------------------( ___ )
          | / |                                      | \ |
          | / |                SEPARAR               | \ |
          |___|                                      |___|
         (_____)------------------------------------(_____)
 * Funcion para separar imagen de la que se utilizará
 * Parametros:
 *      original: Matriz imagen original
 *      editRGB: Matriz para trabajar
 *
 */
void separarMira(const Mat &original, Mat &editRGB){
    Mat maskRGB;
    //Reset image
    editRGB = Mat(0, 0, 0, Scalar( 0,0,0));
    inRange(original, Scalar(0,0,125), Scalar(90,90,255),maskRGB);
    original.copyTo(editRGB,maskRGB);

}


/*
    	  __^__                                      __^__
         ( ___ )------------------------------------( ___ )
          | / |                                      | \ |
          | / |             SET_ENTRANCE             | \ |
          |___|                                      |___|
         (_____)------------------------------------(_____)
 * Funcion para escoger la entrada de acuerdo al angulo
 * Parametros:
 *
 *
 */

void entrance() {

    // adjusting angle
    int newTheta;

    if (-theta < 0){
        newTheta = 360 - (-theta*180)/(M_PI);
    } else {
        newTheta = (-theta*180)/(M_PI);
    }

    if (newTheta < 90) {

        entranceX = 266;
        entranceY = 79;

    } else if (newTheta < 180){

        entranceX = 46;
        entranceY = 31;

    } else if (newTheta < 270){

        entranceX = 10;
        entranceY = 324;

    } else {

        entranceX = 255;
        entranceY = 346;

    }
}


/*
    	  __^__                                      __^__
         ( ___ )------------------------------------( ___ )
          | / |                                      | \ |
          | / |               animation              | \ |
          |___|                                      |___|
         (_____)------------------------------------(_____)

 * Funcion para mostrar la animacion del robot moviéndose
 *
 * Parametros:
 *      none
 *
*/
void animation(){

    unsigned int microsecond = 100000;
    cout << "tamaño del vector x: " << pathX.size() << endl;
    cout << "tamaño del vector y: " << pathY.size() << endl;

    circle(animationImage, Point(pathX.back(), pathY.back()), 5, (0, 255, 255), FILLED);
    //imshow("Phi Graph", phiGraph);
    usleep(microsecond);//sleeps for 1 second
    pathX.pop_back();
    pathY.pop_back();
//    while(!pathX.empty()){
//        cout << "tamaño del vector x: " << pathX.size() << endl;
//        cout << "tamaño del vector y: " << pathY.size() << endl;
//        circle(animationImage, Point(pathX.back(), pathY.back()), 5, Scalar( 0, 255, 255 ), FILLED);
//        if(animationImage.data)
//            cout << "Si hay datos" << endl;
//            imshow("Animated",animationImage);
//        //imshow("Animation",animationImage);
//        usleep(microsecond);//sleeps for 1 second
//        pathX.pop_back();
//        pathY.pop_back();
//    }
//    imshow("Animation",animationImage);
}


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





/*
                                __
                             .'  '.
                         _.-'/  |  \
            ,        _.-"  ,|  /  0 `-.
            |\    .-"       `--""-.__.'=====================-,
            \ '-'`        .___.--._)=========================|
             \            .'      |                          |
              |     /,_.-'        |      PLANIFICADOR        |
            _/   _.'(             |           DE             |
           /  ,-' \  \            |         CAMINOS          |
           \  \    `-'            |                          |
            `-'                   '--------------------------'

 */
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
            //cout << "R: " << (int)pix[2] << " G: " << (int)pix[1]<< " B: " << (int)pix[0]<<endl;
            if(angled){
	            finishingY = y;
	            finishingX = x;

            // Obtener angulo
	            entrance();

	            startingX = entranceX;
	            startingY = entranceY;
            }else{
            	if(startSelected){
	        		finishingY = y;
	        		finishingX = x;
	                startSelected = false;
	        	}else{
	        		startingX = x;
	        		startingY = y;
	                startSelected = true;
	        	}
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


/*
MAIN MAIN MAIN MAIN                    ________
MAIN MAIN MAIN MAIN                /--/        \
              MAIN                |   \______   |
          MAIN                     \ - ---^^- / /
       MAIN                                  ||/
          MAIN                               |||
              MAIN                         .:'':.
MAIN MAIN MAIN MAIN                  /^/^^\/     \
MAIN MAIN MAIN MAIN              0___O_\O_/       |
                                 |               /              O
MAIN                             |       .._    /              //
    MAIN                          \ ____/   |  |              //
         MAIN                               |  |             //
    MAIN      MAIN                   ^^^^^  |  |            ||
        MAIN                      ^^^^^^^^^^|  \         __ /|
    MAIN                         ^^^^^^   ^^|   \       /     \
MAIN                             ^^^^^    __|    \____/        |
                                   ^^^    \    /               |
MAIN           MAIN                         \-/          (_     \
MAIN MAIN MAIN MAIN                          |  |\__________\   |
MAIN MAIN MAIN MAIN                         /|  |          \ \  |
MAIN           MAIN                 _______| |  |     ______\ \  \
                                   /    ____/   |    /    ____/   \
MAIN MAIN MAIN MAIN                \(_ /         \   \(_ /        |
MAIN MAIN MAIN MAIN                    \_(____.../       \_(_____/
            MAIN
       MAIN
  MAIN                                             ( ( (
MAIN MAIN MAIN MAIN                                 ) ) )
MAIN MAIN MAIN MAIN                                ( ( (
                                                 '. ___ .'
                                                '  (> <) '
                                        --------ooO-(_)-Ooo----------
                                                    MAIN

 */

int main(int argc, char *argv[]){
    camera.open(0);
    int thresh = 0;
    bool clicked = false, run = true, ouch = true;
    currentImage = imread("parking2.jpg",IMREAD_COLOR);
    animationImage = imread("parking.jpg",IMREAD_COLOR);
    while (run)
    {
        if(!clicked){
            camera >> currentImageMira;
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

                    /*
                     ███▓▒░░.OPCION DE CAMINOS AUTOMATICA SIN MIRA.░░▒▓███
                     */
                    /*
                     *     oooO
                          (....)     Oooo
                           ...(     (....)
                            ._)      )../
                                     (_/
                        OPCION PARA PLANIFICADOR
                        DE CAMINOS CON MIRA
                     */
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
                    if(ouch)imshow("Filtered",binaryImage);
                    if(animationImage.data && !pathX.empty() && playAnimation) {
                        animation();
                        imshow("Animated", animationImage);
                        if(pathX.size()>1)animationImage = imread("parking.jpg", IMREAD_COLOR);
                    }
                    //if (playAnimation && !pathX.empty()) animation();
                    break;

                    break;

                    /*
                               ____
                             _(____)_
                      ___ooO_(_o__o_)_Ooo___
                         OPCION PARA MIRA
                     */
                case 'f':
                    if(debug)cout << "f";
                    angled = true;
                    separarMira(currentImageMira, segmentedMira);
                    namedWindow("OriginalMira");
                    try{
                        buscaWrappedMira();
                    }
                    catch(runtime_error& e){
                        cout<<"Timed out, trying again."<<endl;
                    }
                    //imshow("OriginalMira",currentImageMira);
                    imshow("SegmentedMira",segmentedMira);


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

                    // 315x362 image size
                    pxy =true;
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
                    if(angled)ouch = false;
                    playAnimation = true;
//                   	if(sel == 'c'){
//                   	    animation();
//                   		//Aquí lo de la mira
//                   		//circle(currentImage, (Point)points[i], 5, Scalar( 0, 0, 255 ), FILLED);
//                   	}
                break;
                // presionar e para capturar el angulo y
                // luego p para dejar de capturarlo
                case 't':
                    sel = 'c';
                    destroyAllWindows();
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
