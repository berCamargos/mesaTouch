#include "cv.h"
#include "highgui.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


using namespace cv;
// A Simple Camera Capture Framework 
CvCapture* capture;

typedef struct pos {
    int x;
    int y;
    float tx;
    float ty;
} Pos;

typedef struct media{
    //esse eh para a media, ele recebe a media das posicoes e o numero de pontos usados para isso.
    Pos posicao;
    int num;
    bool ativo;
} Media;

typedef struct circulo{
    vector<Pos> posicoes;
} Circulo;

void criarFundo(Mat& src,int num,int tipo){
    vector<Mat> imagens;
    int i;
    if (tipo==1){
        src = imread("mesaT/picture_27.png",CV_LOAD_IMAGE_GRAYSCALE );
        imagens.push_back(imread("mesaT/picture_27.png", CV_LOAD_IMAGE_GRAYSCALE ));
        imagens.push_back(imread("mesaT/picture_28.png", CV_LOAD_IMAGE_GRAYSCALE ));
        imagens.push_back(imread("mesaT/picture_29.png", CV_LOAD_IMAGE_GRAYSCALE ));
        imagens.push_back(imread("mesaT/picture_30.png", CV_LOAD_IMAGE_GRAYSCALE ));
        imagens.push_back(imread("mesaT/picture_31.png", CV_LOAD_IMAGE_GRAYSCALE ));
        imagens.push_back(imread("mesaT/picture_32.png", CV_LOAD_IMAGE_GRAYSCALE ));
        imagens.push_back(imread("mesaT/picture_33.png", CV_LOAD_IMAGE_GRAYSCALE ));
        imagens.push_back(imread("mesaT/picture_34.png", CV_LOAD_IMAGE_GRAYSCALE ));
        imagens.push_back(imread("mesaT/picture_35.png", CV_LOAD_IMAGE_GRAYSCALE ));
    }
    else{
        for(i=0;i<num;i++){
            src = cvQueryFrame(capture);
            cvtColor(src,src,CV_RGB2GRAY);
            imagens.push_back(src);
        }
    }
    int nRows = src.rows;
    int nCols = src.cols;
    std::cout << nRows << " - " << nCols << ".>" << imagens.size()  <<"\n";
    int j,z,u,fundoMedia,quantidade;
    
    for( i = 0; i < nRows; ++i)
    {
        for ( j = 0; j < nCols; ++j)
        {
            fundoMedia = 0;
            for (z=0;z<imagens.size();z++){
                fundoMedia = fundoMedia + int(imagens[z].at<uchar>(i,j));
            }
            src.at<uchar>(i,j) = uchar(fundoMedia/imagens.size());
        }
    }
    blur( src, src, Size( 10,10 ), Point(-1,-1) );
}

Circulo removerFundo(Mat& src,Mat& fundo){
    int fundoMedia;
    int srcMedia;
    int nRows = src.rows;
    int nCols = src.cols;
    int lista[nRows][nCols];
    int quantidade;
    Pos posTemp;
    Circulo circuloTemp;
    vector<Circulo> circulos;
    int k,l,i,j,z,u;
    bool achei;
    Mat teste = src;
    blur( src, src, Size( 10,10 ), Point(-1,-1) );
    for( i = 0; i < nRows; ++i){
        for ( j = 0; j < nCols; ++j){
            
            fundoMedia = int(fundo.at<uchar>(i,j));
            srcMedia = int(src.at<uchar>(i,j));
            if ((fundoMedia - srcMedia)<8 && (fundoMedia - srcMedia)>-8 ){
                src.at<uchar>(i,j)= uchar(0);
            }
            if (not((fundoMedia - srcMedia)<8 && (fundoMedia - srcMedia)>-8)){
                posTemp.x = i;
                posTemp.y = j;
                circuloTemp.posicoes.push_back(posTemp);
            }
        }
    }
    circulos.push_back(circuloTemp);
    return circuloTemp;
}

vector<Media> acharPontosMedia(Circulo circulos,int raio){
    vector<Media> medias,mediasFinais;
    Media mediaTemp;
    Pos posTemp;
    int i,j,distancia,h;
    bool jaTem;
    for (i=0;i<circulos.posicoes.size();i++){
        jaTem = false;
        for (j=0;j<medias.size();j++){
            distancia = sqrt(pow((circulos.posicoes[i].x-medias[j].posicao.tx), 2) + pow((circulos.posicoes[i].y-medias[j].posicao.ty), 2) );
            if (distancia<raio){
                medias[j].posicao.tx = (medias[j].posicao.tx*medias[j].num + circulos.posicoes[i].x)/(medias[j].num+1);
                medias[j].posicao.ty = (medias[j].posicao.ty*medias[j].num + circulos.posicoes[i].y)/(medias[j].num+1);
                medias[j].posicao.x = medias[j].posicao.tx;
                medias[j].posicao.y = medias[j].posicao.ty;
                medias[j].num = medias[j].num + 1;
                jaTem = true;
                break;
            }
            if (distancia == raio){//isso significa que esta no mesma parte (no mesma palma da mao) e portanto o objeto eh maior que um dedo
                break;
            }
        }
        if (not jaTem){
            mediaTemp.ativo = true;
            if (distancia == raio){
                medias.erase(medias.begin()+j);
                //medias[j].ativo = false;
                mediaTemp.ativo = false;
            }
            mediaTemp.num = 1;
            posTemp.tx = circulos.posicoes[i].x;
            posTemp.ty = circulos.posicoes[i].y;
            posTemp.x = posTemp.tx;
            posTemp.y = posTemp.ty;
            mediaTemp.posicao = posTemp;
            medias.push_back(mediaTemp);
        }
    }
    for (j=0;j<medias.size();j++){
        if (medias[j].ativo){
            mediasFinais.push_back(medias[j]);
        }
    }
    return mediasFinais;
}

Circulo acharPontosConjunto(Circulo pontos){
    int i,j,k,l;
    vector<Circulo> circulos;
    Circulo circuloTemp;
    Pos posTemp;
    bool achei;
    for (i=0;i<pontos.posicoes.size();i++){
        achei = false;
        for(k=0; k<circulos.size(); k++){
            for (l=(circulos[k].posicoes.size()-1); l>=0; l--){
                if ( (circulos[k].posicoes[l].x-pontos.posicoes[i].x)<2  && (circulos[k].posicoes[l].x-pontos.posicoes[i].x)>-2 ){ 
                    if ( (circulos[k].posicoes[l].y-pontos.posicoes[i].y)<2 && (circulos[k].posicoes[l].y-pontos.posicoes[i].y)>-2 ){
                        posTemp.x = pontos.posicoes[i].x;
                        posTemp.y = pontos.posicoes[i].y;
                        circulos[k].posicoes.push_back(posTemp);
                        achei = true;
                        break;
                    }
                }
            }
            if (achei){
                break;
            }
        }
        if (achei==false){
            circuloTemp.posicoes.clear();
            posTemp.x = pontos.posicoes[i].x;
            posTemp.y = pontos.posicoes[i].y;
            circuloTemp.posicoes.push_back(posTemp);
            circulos.push_back(circuloTemp);
        }
    }
    std::cout << circulos.size() << "\n";
    Circulo medias;
    for (i=0;i<circulos.size();i++){
        if (circulos[i].posicoes.size()>25){
            posTemp.x = 0;
            posTemp.y = 0;
            for (k=0;k<circulos[i].posicoes.size();k++){
                posTemp.x = posTemp.x + circulos[i].posicoes[k].x;
                posTemp.y = posTemp.y + circulos[i].posicoes[k].y;  
            }
            posTemp.x = posTemp.x/circulos[i].posicoes.size();
            posTemp.y = posTemp.y/circulos[i].posicoes.size();
            medias.posicoes.push_back(posTemp);
        }
    }
    return medias;
}

Circulo geral(Mat& src,Mat& fundo){
    Mat ver,teste;
    ver = imread("mesaT/picture_1.png", 1 );
    Circulo pontos;
    vector<Media> medias;
    criarFundo(fundo,10,1);
    src = imread("mesaT/picture_1.png", CV_LOAD_IMAGE_GRAYSCALE );
    teste = imread("mesaT/picture_1.png", CV_LOAD_IMAGE_GRAYSCALE );
    std::cout << "fidsfndcsd\n";
    pontos = removerFundo(src,fundo);
    std::cout << "safasfjfdsifjs\n";
    //medias = acharPontosConjunto(pontos);
    cvtColor(src,src,CV_GRAY2BGR);
    /*for (int i=0;i<medias.posicoes.size();i++){
        src.at<cv::Vec3b>(medias.posicoes[i].x,medias.posicoes[i].y)[2] = uchar(255);
        src.at<cv::Vec3b>(medias.posicoes[i].x,medias.posicoes[i].y)[1] = uchar(0);
        src.at<cv::Vec3b>(medias.posicoes[i].x,medias.posicoes[i].y)[0] = uchar(0);

    }*/
    medias = acharPontosMedia(pontos,25);
    for (int i=0;i<medias.size();i++){
        if (medias[i].num>0){
            src.at<cv::Vec3b>(medias[i].posicao.x,medias[i].posicao.y)[2] = uchar(255);
            src.at<cv::Vec3b>(medias[i].posicao.x,medias[i].posicao.y)[1] = uchar(0);
            src.at<cv::Vec3b>(medias[i].posicao.x,medias[i].posicao.y)[0] = uchar(0);
        }
        std::cout << medias[i].posicao.x << " , " << medias[i].posicao.y << " -> "<< medias[i].num<<"\n";

    }
    std::cout << medias.size() << "\n";
    imshow("solucao2",teste);
    imshow("solucao", src );
    return pontos;
}

int main() {
    int a=0,h=0;
    float x,y;
    capture = cvCaptureFromCAM( CV_CAP_ANY );
    if ( !capture ) {
      fprintf( stderr, "ERROR: capture is NULL \n" );
      getchar();
      return -1;
    }

    Display *dpy;
    Window root_window;
    dpy = XOpenDisplay(0);
    root_window = XRootWindow(dpy, 0);
    XSelectInput(dpy, root_window, KeyReleaseMask);
    
    Mat fundo,src,tela;
    Circulo pontos;
    vector<Media> medias;
    tela = imread("gnu.png",CV_LOAD_IMAGE_GRAYSCALE);
    imshow("tela",tela);
    src = imread("mesaT/picture_1.png", CV_LOAD_IMAGE_GRAYSCALE );
    std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n---------------------------------------------------------------\n\n\n";
    while(a!=27){
        std::cout << "1-criarFundo\n2-removerFundo\n3-acharPontosMedia\n4-mostrarResultado\n";
        std::cout << a << "\n";
        a = waitKey(0);
        double t = (double)getTickCount(); 
        if(a==49){
            std::cout << "\n\ncriarFundo\n\n";
            criarFundo(fundo,10,1);
            imshow("tela",fundo);
        }
        if(a==50){
            std::cout << "\n\nremoverFundo\n\n";
            //src = cvQueryFrame(capture);
            //cvtColor(src,src,CV_RGB2GRAY);
            pontos = removerFundo(src,fundo);
            imshow("tela", src );
        }
        if (a==51){
            std::cout << "\n\nacharPontosMedia\n\n";
            medias = acharPontosMedia(pontos,15);
        }
        if (a==52){
            std::cout << "\n\nmostrarPontos\n\n";
            cvtColor(src,src,CV_GRAY2BGR);
            std::cout << medias.size() << "\n";
            //for(int w=0;w<15;w++){
            //    for(int y=0;y<15;y++){
            //        src.at<cv::Vec3b>(w,y)[2] = uchar(255);
            //    }
            //}
            for (int i=0;i<medias.size();i++){
                std::cout << i << " - " << medias[i].posicao.x << " , " << medias[i].posicao.y << " -> "<< medias[i].num<< " ->" << medias[i].ativo << "\n";
                src.at<cv::Vec3b>(medias[i].posicao.x,medias[i].posicao.y)[2] = uchar(255);
                src.at<cv::Vec3b>(medias[i].posicao.x,medias[i].posicao.y)[1] = uchar(0);
                src.at<cv::Vec3b>(medias[i].posicao.x,medias[i].posicao.y)[0] = uchar(0);
            }  
            imshow("tela", src );   
            x = float(medias[0].posicao.x)/640*1366;
            y = float(medias[0].posicao.y)/480*768;
            std::cout << x << "," << y << "\n"; 
            XWarpPointer(dpy, None, root_window, 0, 0, 0, 0, x,y);
            XFlush(dpy);
  
        }
        if (a==53){
            //criarFundo(fundo,10,1);
            src = imread("mesaT/picture_1.png", CV_LOAD_IMAGE_GRAYSCALE );
            medias = acharPontosMedia(removerFundo(src,fundo),15);
            for (int i=0;i<medias.size();i++){
               std::cout << i << " - " << medias[i].posicao.x << " , " << medias[i].posicao.y << " -> "<< medias[i].num<<" ->"    <<medias[i].ativo << "\n";
            }
            x = float(medias[0].posicao.x)/640*1366;
            y = float(medias[0].posicao.y)/480*768;
            //std::cout << x << "," << y << "\n"; 
            XWarpPointer(dpy, None, root_window, 0, 0, 0, 0, x,y);
            XFlush(dpy);

        }
        
        if (a==54){
            criarFundo(fundo,10,1);
            while(true){
                src = imread("mesaT/picture_1.png", CV_LOAD_IMAGE_GRAYSCALE );
                //src = cvQueryFrame(capture);
                //cvtColor(src,src,CV_RGB2GRAY);
                medias = acharPontosMedia(removerFundo(src,fundo),15);
                for (int i=0;i<medias.size();i++){
                    std::cout << i << " - " << medias[i].posicao.x << " , " << medias[i].posicao.y << " -> "<< medias[i].num<<" ->"<<medias[i].ativo << "\n";
                }
                h=h+1;
                std::cout << h << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
            }
        }
        t = ((double)getTickCount() - t)/getTickFrequency();
        std::cout << "Times passed in seconds: " << t << "\n";
        std::cout << "\n\n\n---------------------------------------------------------------\n\n\n";
    }
}
