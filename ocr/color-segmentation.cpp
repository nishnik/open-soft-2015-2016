#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
//#include "stdafx.h"
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include <opencv2/objdetect/objdetect.hpp>
#include <fstream>
#include <math.h>
#include <vector>
#include <map>
#include <string>
#include </home/aytas32/OpenSoft/inter_hall_16/ocr/peakdetect.cpp>

using namespace cv;
using namespace std;
typedef unsigned char uchar;
int run(int argc, char **argv); // for peakdetect
// expands black mask using a looser threshold for saturation to be considered black (eg. 20%)
void traverse(Mat &bmask, Mat &visited, Mat &imgHSV, int i, int j, Mat &bmaskNew, Mat &whiteMask, int lowSatThresh) {
  int iArr[] = {-2, -1, 0, 1, 2};
  for (int k = 0; k < 5; k++) {
    for (int l = 0; l < 5; l++) {
      int ii = i+iArr[k], jj = j+iArr[l];
      if (ii < 0 || ii >= bmask.rows || jj < 0 || jj >= bmask.cols)
        continue;
      if (ii == i && jj == j)
        continue;
      Vec3b hsv = imgHSV.at<Vec3b>(ii,jj);
      if (visited.at<uchar>(ii,jj))
        continue;
      visited.at<uchar>(ii,jj) = 1;
      // add special conditions for brown and blue to counter anti aliasing
      if ((hsv[1] <= lowSatThresh || fabs(hsv[0]-30) < 20 || fabs(hsv[0]-200) < 20 || fabs(hsv[0]-216) < 8) && whiteMask.at<uchar>(ii,jj) ) {
        visited.at<uchar>(ii,jj) = 1;
        bmaskNew.at<uchar>(ii,jj) = 0;
        traverse(bmask, visited, imgHSV, ii, jj, bmaskNew, whiteMask, lowSatThresh);
      }
    }
  }
}
Mat expandBMask(Mat bmask, Mat imgHSV, Mat whiteMask, int lowSatThresh) {
  Mat visited = Mat::zeros(bmask.size(), CV_8U);
  Mat bmaskNew = Mat::ones(bmask.size(), CV_8U);
  for (int i = 0; i < bmask.rows; ++i)
  {
    for (int j = 0; j < bmask.cols; ++j)
    {
      if (bmask.at<uchar>(i,j) == 0)
        bmaskNew.at<uchar>(i,j) = 0;
      if (bmask.at<uchar>(i,j) == 0 && visited.at<uchar>(i,j) == 0) {
        visited.at<uchar>(i,j) = 1;
        traverse(bmask, visited, imgHSV, i, j, bmaskNew, whiteMask, lowSatThresh);
      }
    }
  }
  return bmaskNew;
}

void histRGB(Mat src){
  vector<Mat> bgr_planes;
  split( src, bgr_planes );

  /// Establish the number of bins
  int histSize = 256;

  /// Set the ranges ( for B,G,R) )
  float range[] = { 0, 256 } ;
  const float* histRange = { range };

  bool uniform = true; bool accumulate = false;

  Mat b_hist, g_hist, r_hist;

  /// Compute the histograms:
  calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );

  /*imshow("blue hist",b_hist);
  imshow("red hist",r_hist);
  imshow("green hist",g_hist);*/

  // Draw the histograms for B, G and R
  int hist_w = 512; int hist_h = 400;
  int bin_w = cvRound( (double) hist_w/histSize );

  Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

  /// Normalize the result to [ 0, histImage.rows ]
  normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
  normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
  normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

  /// Draw for each channel
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

  /// Display
  namedWindow("calcHist Demo", CV_WINDOW_AUTOSIZE );
  imshow("calcHist Demo", histImage );
  //namedWindow("blue hist", CV_WINDOW_AUTOSIZE );
 

}

int main(int argc, char const *argv[])
{
  if (argc != 3) {
    printf("usage: ./color-segmentation <graph-img-cropped> <out-basename>\n");
    return 0;
  }
  cout<<"sdhfukdshf\n";
  /*string path=argv[1];
  char Path[]="../ocr/build/graph-box ";
  strcat(Path,path.c_str()); 
  strcat(Path," ../ocr/clippedImages/img.png");
  cout<<Path<<endl;
  system(Path);
  system("../ocr/build/graph-box  ../ocr/clippedImages/img.png ../ocr/clippedImages/img.png");*/
  
  Mat img = imread(argv[1], CV_LOAD_IMAGE_COLOR);
  Mat imgHSV;
  cvtColor(img, imgHSV, CV_BGR2HSV);
  Mat mask(img.rows, img.cols, CV_8U, Scalar(1)), bmask = mask.clone(); // to mask away all colorless pixels
  // bmask = black mask. will dilate to remove some anti-aliased points
  for (int i = 0; i < mask.rows; ++i)
  {
    for (int j = 0; j < mask.cols; ++j)
    {
      mask.at<uchar>(i,j) = 1;
      bmask.at<uchar>(i,j) = 1;
      Vec3b cRGB = img.at<Vec3b>(i,j);
      Vec3b cHSV = imgHSV.at<Vec3b>(i,j);
      if (cRGB[0] >= 210 && cRGB[1] >= 210 && cRGB[2] >= 210) {//210
        mask.at<uchar>(i,j) = 0;
      } else if (cRGB[0] <= 120 && cRGB[1] <= 120 && cRGB[2] <= 120) {
        bmask.at<uchar>(i,j) = 0;
      }  // if saturation is low i.e less than 10%, it could be black/grey, we don't need that.
      else if (cHSV[1] <= .03*255) {
        bmask.at<uchar>(i,j) = 0;
      }      
    }
  }
  //anti aliasing correction
  // using search method
  //again adding all black pixels to bmask for higher threshold of saturation
  bmask = expandBMask(bmask, imgHSV, mask, 0.11*255);//0.11
  bool got=false;
  for (int i = 0; i < mask.rows; ++i)
  {
    for (int j = 0; j < mask.cols; ++j)
    {
      Vec3b cRGB = img.at<Vec3b>(i,j);
      Vec3b cHSV = imgHSV.at<Vec3b>(i,j);
       if (cHSV[1] <= .06*255)// || (cHSV[0]<166+5 && cHSV[0]>166-5)) //because 166 was the text hue//0.06
      {
        bmask.at<uchar>(i,j) = 0;
      }      
    }
  }
  
  mask = mask & bmask;
  
  // we will now classify with H value only.
  // make histogram on H
  vector<int> hist(256, 0);
  for (int i = 0; i < mask.rows; ++i)
  {
    for (int j = 0; j < mask.cols; ++j)
    {
      if (!mask.at<uchar>(i,j))
        continue;
      hist[imgHSV.at<Vec3b>(i,j)[0]]++;
    }
  }

  

  int maxH = 0;
  for (int i = 0; i < 256; ++i)
  {
    maxH = max(maxH, hist[i]);
  }
  
  
  ////--------------------------------------------------------------------------------------------------------------------------------------
  // make string for running peakdetect
  int noLegends=6;//read from file of legendDetection
  int noPlots=0;
  int deltaParam=3;
  vector<float> maxima;
  int tick=5;

  while(noPlots!=noLegends){

    if(noPlots<noLegends){
      // deltaParam--;
      tick--;
    }
    else if(noPlots>noLegends){
      // deltaParam++;
      tick++;
    }
        

    cout<<"PLOT DATA: "<<noLegends<<" "<<noPlots<<" "<<deltaParam<<" "<<tick<<"\n";
     getchar();

    for(int k = 0 ; k < 1; k++){

      int nhist[260] = {0};
      for(int i = 5 ; i < 250 ;i++){
        nhist[i] = 0;
        for(int j = -tick ; j <= tick ; j++){//-4 4
          nhist[i] += (1/(abs(j)/3.0 + 1.0))*hist[i + j];
        }
      }
      for(int i = 5 ; i < 250 ; i++){
        hist[i] = nhist[i];
      }
    }
    // cout<<"maxH : "<<maxH<<endl;
     maxH = 0;
    ////////////////////////////file peak detect
    FILE* fp = fopen("res.csv","w");
    for(int i=0;i<256;i++){
      fprintf(fp,"%d,%d\n",i,hist[i]);
    }
    fclose(fp);

    {
      
      maxima.clear();
      
      string cmd = "dummy -i res.csv -d 1e";
      cmd+=to_string(deltaParam)+" -o out.csv";
      std::vector<char *> args;
      std::istringstream iss(cmd);


      std::string token;
      while(iss >> token) {
        char *arg = new char[token.size() + 1];
        copy(token.begin(), token.end(), arg);
        arg[token.size()] = '\0';
        args.push_back(arg);
      }

      // now exec with &args[0], and then:
      run(args.size(), &args[0]);


      for(size_t i = 0; i < args.size(); i++)
        delete[] args[i];
    }
    // system("./peakdetect -i res.csv -d 1e2 -o out.csv");//1e2
    ifstream FP("out.csv");
    int i,j;
    string as;
    
    while(getline(FP,as)){
      // cout<<stof(as)<<"\n";
      maxima.push_back(stof(as));
      //hist[stof(as)]=10000;
    }
    noPlots=maxima.size()/2;
    if(maxima[noPlots+1]<=20)
      --noPlots;


  }
  cout<<"OUT\n";
  
  ///-------------------------------------------------------------------------------------------------------------------------------------
  // first half of maxima array is max, second half is min
  maxima.push_back(256);
  

  
  for (int i = 0; i < 256; ++i)
  {
    maxH = max(maxH, hist[i]);
  }
  std::vector<int> Hpeaks;
   for (int i = 0; i < 256; ++i)
    {
      if(hist[i]>0.5*maxH)
        Hpeaks.push_back(i);
    }
  

  // normalize histogram
  int hist_w = 512; int hist_h = 400;
  for (int i = 0; i < 256; ++i)
  {
    hist[i] = (hist[i] *hist_h)/maxH;
  }

  int maxNo=maxima.size()/2;
  for(int i=0;i<maxima.size()/2 ;i++){
    hist[maxima[i]]=hist_h;
  }
  
  

  int bin_w = cvRound( (double) hist_w/256. );
  Mat histImg(hist_h, hist_w, CV_8U, Scalar(0));
  for (int i = 1; i < 256; i++) {
    line( histImg, Point( bin_w*(i-1), hist_h - cvRound(hist[i-1]) ) ,
                       Point( bin_w*(i), hist_h - cvRound(hist[i]) ),
                       Scalar( 255), 2, 8, 0  );
  }
  imshow("histogram", histImg);
  // draw all pixels with H < 50 degrees. Troublesome in 1.png?
  
  Mat yellow;
  cvtColor(imgHSV,yellow,CV_HSV2BGR);
  Mat unColor=yellow.clone();
  //Mat yellow = img.clone();
  for (int i = 0; i < yellow.rows; ++i)
  {
    for (int j = 0; j < yellow.cols; ++j)
    {
      if (!mask.at<uchar>(i,j)){
        yellow.at<Vec3b>(i,j) = Vec3b(0,0,0);
      }
      if(mask.at<uchar>(i,j)){
        unColor.at<Vec3b>(i,j)=Vec3b(255,255,255);
      }
      
    }
  }

  

  Mat plots=yellow.clone();//Mat::zeros(yellow.rows,yellow.cols,CV_8UC1);
  //merging folding of red
  int k=maxNo,t=0;
  plots=Scalar(0);
  if(maxima[maxNo+1]<=20){
    printf("%d\n", (int)maxima.size()/2-1);
    k++;
    t=1;
    plots=Scalar(0);
    for(int i=0;i<imgHSV.rows;i++){

      for(int j=0;j<imgHSV.cols;j++){

        if (mask.at<uchar>(i,j) && imgHSV.at<Vec3b>(i,j)[0]>=maxima[maxNo] && imgHSV.at<Vec3b>(i,j)[0]<maxima[maxNo+1]){
          plots.at<Vec3b>(i,j) = 255;
        }
      }
    }
    for(int i=0;i<imgHSV.rows;i++){

      for(int j=0;j<imgHSV.cols;j++){

        if (mask.at<uchar>(i,j) && imgHSV.at<Vec3b>(i,j)[0]>=maxima[2*maxNo-1] && imgHSV.at<Vec3b>(i,j)[0]<maxima[2*maxNo]){
          plots.at<Vec3b>(i,j) = 255;
        }
      }
    }
    char name[1000];
    sprintf(name, "%s-0.png", argv[2]);
    imwrite(name,plots);
  }
  else
    printf("%d\n", (int)maxima.size()/2);
  
  for(;k<2*maxNo-t;k++){
    plots=Scalar(0);
    for(int i=0;i<imgHSV.rows;i++){

      for(int j=0;j<imgHSV.cols;j++){

        if (mask.at<uchar>(i,j) && imgHSV.at<Vec3b>(i,j)[0]>=maxima[k] && imgHSV.at<Vec3b>(i,j)[0]<maxima[k+1]){
          plots.at<Vec3b>(i,j) = yellow.at<Vec3b>(i,j);
        }
      }
    }
    char name[1000];
    sprintf(name, "%s-%d.png", argv[2], (k-maxNo));
    imwrite(name,plots);
   
  }

  Mat imgBW = Mat(imgHSV.rows,imgHSV.cols,CV_8UC1);
  for(int i=0;i<imgHSV.rows;i++){
    for(int j=0;j<imgHSV.cols;j++){
      imgBW.at<uchar>(i,j)=imgHSV.at<Vec3b>(i,j)[1]*2;
    }
  }
  imshow("weird", yellow);
  imwrite("weird2.png", yellow);
 
  
  return 0;
}