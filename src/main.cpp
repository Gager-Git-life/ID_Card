# include "Image.hpp"
# include "UltraFace.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, char **argv){

    try{

        
        //0.人脸检测模型文件地址以及人脸检测模型类的初始化
        string ultraface_mnn_path = "model/version-slim/slim-320.mnn";
        UltraFace ultraface(ultraface_mnn_path, 320, 240, 4, 0.65); // config model input

        string file_path = "images/w24.jpg";
        Image_Process Img_process;

        //1.图像缩放旋转
        cv::Mat img = cv::imread(file_path , 1);
        if(img.empty()){
            puts("[INFO]>>> 输入图像读取有误，可能是地址错误");
            return 0;
        }
        if(img.size().height > img.size().width){
            img = Img_process.Rotate_image(img, 90);
        }
        cv::Mat resize_img = Img_process.Resize_pic(img, 428);
        // cv::imshow("resize", resize_img);
        // cv::waitKey(0);

        //2.图像预处理得到二值化的图像
        auto start = chrono::steady_clock::now();
        cv::Mat img_dilation = Img_process.Img_preprocess(resize_img);
        auto end = chrono::steady_clock::now();
        chrono::duration<double> elapsed = end - start;
        cout << "[INFO]>>> 预处理耗时: " << elapsed.count() << " s" << endl;
        // cv::imshow("dilation", img_dilation);
        // cv::waitKey(0);

        //3.利用二值化图像获取文本对应的矩形区域，并进行初步过滤干扰框
        start = chrono::steady_clock::now();
        vector<box_coll> regions = Img_process.find_all_region(resize_img, img_dilation);
        end = chrono::steady_clock::now();
        elapsed = end - start;
        cout << "[INFO]>>> 寻找矩形框耗时: " << elapsed.count() << " s" << endl;

        //4.分析第三步来的矩形区域获取出可能的完整身份证，人像，身份证号码区域
        //  并通过找到的身份证，人像，号码框进一步过滤干扰框
        start = chrono::steady_clock::now();
        Img_process.Regions_analysis(resize_img, regions, 1, "images");
        end = chrono::steady_clock::now();
        elapsed = end - start;
        cout << "[INFO]>>> 模式分析耗时: " << elapsed.count() << " s" << endl;

        //5.得到第四步获取的身份证，人像，号码box
        box_coll face_box = Img_process.get_box("face");
        box_coll card_box = Img_process.get_box("card");
        box_coll numb_box = Img_process.get_box("number");

        //人脸检测
        if(face_box.box_area != 0){

            vector<FaceInfo> face_info;
            int begin_x, width_, begin_y, heigh_;
            begin_x = face_box.box_point[0].x;
            width_ = face_box.box_width;
            begin_y = face_box.box_point[0].y;
            heigh_ = face_box.box_heigh;
            cv::Rect rect(begin_x, begin_y, width_, heigh_);
            cv::Mat crop_img = resize_img(rect).clone();
            start = chrono::steady_clock::now();
            ultraface.detect(crop_img, face_info);
            end = chrono::steady_clock::now();
            elapsed = end - start;
            printf("[INFO]>>> 发现人脸数:%d, 检测耗时:%fs\n", face_info.size(), elapsed.count());
            // for (auto face : face_info) {
            //     cv::Point pt1, pt2;
            //     pt1 = cv::Point(face.x1, face.y1);
            //     pt2 = cv::Point(face.x2, face.y2);
            //     cv::rectangle(crop_img, pt1, pt2, cv::Scalar(0, 255, 0), 2);

            // }
            // cv::imshow("face++", crop_img);
            // cv::waitKey(0);
        }

        // 6.寻找最靠左边的五个文字框（姓名， 性别， 出书， 住址， 身份证号码）作为
        //  对应行的基准框，用于下一步
        start = chrono::steady_clock::now();
        vector<box_text> regions_five = Img_process.find_left_chinese(regions);
        end = chrono::steady_clock::now();
        elapsed = end - start;
        cout << "[INFO]>>> 寻找基准框耗时: " << elapsed.count() << " s" << endl;

        //7.如果检索到了五个基准文本框则进一步通过结合第五步得到的身份证，人像或者号码
        //  框，通过计算重叠比例得到最佳的身份证信息框
        if(regions_five.size() == 5){

            if(card_box.box_area != 0 ){
                start = chrono::steady_clock::now();
                vector<cv::Rect> infer_boxs = Img_process.find_better_match_with(regions_five, regions, "card", 0.7);
                end = chrono::steady_clock::now();
                elapsed = end - start;
                cout << "[INFO]>>> card match 耗时: " << elapsed.count() << " s" << endl;

                for(int i=0; i<infer_boxs.size(); i++){

                    cv::Rect rect_ = infer_boxs[i];
                    string name = "card-aa";
                    cv::Mat crop_img = resize_img(rect_).clone();
                    cv::imshow(name, crop_img);
                    cv::waitKey(0);
                }
            }
            if(face_box.box_area != 0){
                start = chrono::steady_clock::now();
                vector<cv::Rect> infer_boxs = Img_process.find_better_match_with(regions_five, regions, "face", 0.5);
                end = chrono::steady_clock::now();
                elapsed = end - start;
                cout << "[INFO]>>> face match 耗时: " << elapsed.count() << " s" << endl;
                for(int i=0; i<infer_boxs.size(); i++){

                    cv::Rect rect_ = infer_boxs[i];
                    printf("[INFO]>>> rect.x:%d, rect.y:%d, w:%d, h:%d \n", rect_.x, rect_.y, rect_.width, rect_.height);
                    string name = "face-bb" ;
                    cv::Mat crop_img = resize_img(rect_).clone();
                    cv::imshow(name, crop_img);
                    cv::waitKey(0);
                }
            }
            if(numb_box.box_width != 0){
                start = chrono::steady_clock::now();
                vector<cv::Rect> infer_boxs = Img_process.find_better_match_with(regions_five, regions, "number", 0.6);
                end = chrono::steady_clock::now();
                elapsed = end - start;
                cout << "[INFO]>>> number match 耗时: " << elapsed.count() << " s" << endl;
                for(int i=0; i<infer_boxs.size(); i++){

                    cv::Rect rect_ = infer_boxs[i];
                    printf("[INFO]>>> rect.x:%d, rect.y:%d, w:%d, h:%d \n", rect_.x, rect_.y, rect_.width, rect_.height);
                    string name = "numb-cc" ;
                    cv::Mat crop_img = resize_img(rect_).clone();
                    cv::imshow(name, crop_img);
                    cv::waitKey(0);
                }
            }
            else{
                printf("[INFO]>>> 无card 无face情况");
                // vector<vector<box_text>> all_box_text =  Img_process.find_all_by_left(regions_five, regions);
            }
        }

        // if(face_box.box_area != 0 && numb_box.box_area != 0){
        //     printf("[INFO]>>> width:%d, scale:%f \n", numb_box.box_width, 1.*(face_box.box_point[3].x - numb_box.box_point[0].x) / numb_box.box_width);
        // }
    }
    catch(char *msg){
        puts("[ERROR]>>> main except --> main");
    }


}
