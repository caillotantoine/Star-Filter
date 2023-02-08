#include <iostream>
#include <string>
#include <cmath>

#include <opencv4/opencv2/opencv.hpp>

using namespace std;

int threshold_bin = 128;
cv::Mat img_gray;
cv::Mat BinImg;
cv::Mat errodedImg;

cv::Mat inputImage;
cv::Mat backgroundIMG;

class StarKernel {
    private:
        vector<vector<char>> kernel;

        void fillKernel(int diameter, float thickness) {
            float radius = float(diameter) / 2.0 - 0.5;
            float trueThick = (thickness - 1.0) / 4.0;

            for(int i = 0; i < diameter; i++)
            {
                for(int j = 0; j < diameter; j++)
                {
                    float dist = sqrt((i-radius) * (i-radius) + (j-radius) * (j-radius));

                    if (dist > radius - (0.5 + trueThick))
                    {
                        if (dist < radius + (0.5 + trueThick))
                        {
                            this->kernel.at(i).at(j) = 127;
                        }
                        else
                        {
                            this->kernel.at(i).at(j) = -128;
                        }
                    }    
                }
            }
        }

    public:
        StarKernel () {};
        StarKernel(int diameter, float thickness) {
            this->regenerate(diameter, thickness);
        }

        void regenerate(int diameter, float thickness) {
            this->kernel.clear();
            this->kernel.resize(diameter, vector<char> (diameter, 0));
            this->fillKernel(diameter, thickness);
        }

        void print() {
            for (vector<vector<char>>::iterator line=this->kernel.begin(); line!=this->kernel.end(); line++)
            {
                for (vector<char>::iterator row=line->begin(); row!=line->end(); row++)
                {
                    cout << *row << " \t";
                }
                cout << endl;
            }
        }
};

cv::Mat getBackground(cv::Mat original)
{
    cv::Mat backgroundImg = original.clone();
    int erosion_size = 5;

    // cv::copyTo(original, backgroundImg);

    // cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, 
    //     cv::Size(2*erosion_size+1, 2*erosion_size+1),
    //     cv::Point(erosion_size, erosion_size));
    // for(int i = 0; i<1; i++)
    // {
    //     cv::blur(backgroundImg, backgroundImg, cv::Size(3, 3));
    //     // cv::erode(backgroundImg, backgroundImg, element);
    // }
    cv::GaussianBlur(backgroundImg, backgroundImg, cv::Size(31, 31), 5.0);
    return backgroundImg;
}

cv::Mat cleanImage(cv::Mat original, cv::Mat background, cv::Mat mask)
{
    cv::Mat cleanImage = original.clone();;
    // cleanImage = original.clone();

    // cv::copyTo(cleanImage, background, mask);
    // cv::copyTo(original, cleanImage);

    // cout << "Clean Image Type " << cleanImage.type() << endl; 

    for(size_t i = 0; i<cleanImage.rows; i++)
    {
        for(size_t j = 0; j<cleanImage.cols; j++)
        {
            if(mask.at<unsigned char>(j, i) >= 1)
            {
                cleanImage.at<cv::Vec3b>(j, i) = background.at<cv::Vec3b>(j, i); 
            }
        }
    }
    return cleanImage;
}

cv::Mat recleanImage(cv::Mat cleaned)
{
    cv::Mat bluredPreCleaned = cleaned.clone();
    cv::GaussianBlur(cleaned, bluredPreCleaned, cv::Size(11, 11), 15.0);
    
    cv::Mat cleanedGray, cleanedBin;
    cv::cvtColor(cleaned, cleanedGray, cv::COLOR_BGR2GRAY);
    cv::threshold(cleanedGray, cleanedBin, 100, 255, cv::THRESH_BINARY);
    int erosion_size = 1;
    cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, 
        cv::Size(2*erosion_size+1, 2*erosion_size+1),
        cv::Point(erosion_size, erosion_size));
    cv::erode(cleanedBin, cleanedBin, element);
    erosion_size = 2;
    element = cv::getStructuringElement(cv::MORPH_ELLIPSE, 
        cv::Size(2*erosion_size+1, 2*erosion_size+1),
        cv::Point(erosion_size, erosion_size));
    cv::dilate(cleanedBin, cleanedBin, element);
    cv::bitwise_not(cleanedBin, cleanedBin);

    return cleanImage(cleaned, bluredPreCleaned, cleanedBin);
}

void on_trackbar(int, void*)
{
    double threshold = 255 - (threshold_bin);
    cv::threshold(img_gray, BinImg, threshold, 255.0, cv::THRESH_BINARY);
    // cv::imshow("Binarize image", BinImg);
    int erosion_size = 3;
    cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, 
        cv::Size(2*erosion_size+1, 2*erosion_size+1),
        cv::Point(erosion_size, erosion_size));

    cv::erode(BinImg, errodedImg, element);

    // erosion_size++;
    element = cv::getStructuringElement(cv::MORPH_ELLIPSE, 
        cv::Size(2*erosion_size+1, 2*erosion_size+1),
        cv::Point(erosion_size, erosion_size));
    cv::dilate( errodedImg, errodedImg, element );

    cv::Mat invertedImg;
    
    cv::bitwise_not(errodedImg, invertedImg);

    cv::Mat toDelete;

    cv::bitwise_and(BinImg, invertedImg, toDelete);

    // cv::imshow("Erroded Image", toDelete);

    // cv::imshow("Binarize image", toDelete);

    cv::Mat cleaned;
    cleaned = cleanImage(inputImage, backgroundIMG, toDelete);

    cv::Mat cleanedDenoised;
    cleanedDenoised = recleanImage(cleaned);
    // cv::fastNlMeansDenoisingColored(cleaned, cleanedDenoised, 10.0);
    cv::imshow("Final image", cleanedDenoised);


    std::vector<cv::Mat> toDeleteChan;
    for(int x = 0; x<3; x++)
        toDeleteChan.push_back(toDelete);
    cv::Mat toDeleteRGB;
    cv::merge(toDeleteChan, toDeleteRGB);
    
    cv::Mat toDispUp, toDispDown, toDisp;
    cv::hconcat(inputImage, cleaned, toDispUp);
    cv::hconcat(backgroundIMG, toDeleteRGB, toDispDown);
    cv::vconcat(toDispUp, toDispDown, toDisp);
    // cv::hconcat(toDisp, cleanedDenoised, toDisp);
    cv::imshow("Binarize image", toDisp);
}



int main(int argc, char** argv )
{
    cout << "Hello world!" << endl;

    cv::namedWindow("Binarize image", cv::WINDOW_AUTOSIZE); // Create Window

    // const int starDiameter = 17;
    // StarKernel kernel(starDiameter, 2);
    // kernel.print();
    // vector<vector<uchar>> kernel = generateKernel(starDiameter, 2);

    
    // inputImage = cv::imread("C2023ZTF_B.jpg", cv::IMREAD_COLOR);
    inputImage = cv::imread("Bode.jpg", cv::IMREAD_COLOR);

    cv::cvtColor(inputImage, img_gray, cv::COLOR_BGR2GRAY);

    backgroundIMG = getBackground(inputImage);
    // cv::imshow("Eroded color", backgroundIMG);

    cv::createTrackbar("Threshold", "Binarize image", &threshold_bin, 155, on_trackbar);
    on_trackbar(0, NULL);

   

    

    cv::waitKey(0);
    return 0;
}