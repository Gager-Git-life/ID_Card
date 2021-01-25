
#ifndef Imgae_hpp
#define Image_hpp

#pragma once

#include <opencv2/opencv.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
// #include <cmath>

using namespace std;

typedef struct box_coll
{
    int box_index;
    vector<cv::Point> box_point;
    float box_rotate = 0.;
    float box_scale = 0.;
    int box_area = 0;
    int box_width = 0;
    int box_heigh = 0;
} box_coll;

typedef struct box_text
{
    int box_index;
    int left_top_x;
    int left_top_y;
    int width;
    int heigh;
} box_text;

typedef struct scale_card
{
    float scale_x0;
    float scale_x1;
    float scale_y0;
    float scale_y1;
} scale_card;

class Image_Process
{
public:
    Image_Process();
    ~Image_Process();
    cv::Size resize_shape = cv::Size(428, 270);
    cv::Mat Rotate_image(cv::Mat src, float angle);
    cv::Mat Resize_pic(cv::Mat img, int dst_size);
    cv::Mat Img_preprocess(cv::Mat);
    cv::Mat denoise(cv::Mat binary);
    vector<box_coll> find_all_region(cv::Mat ori_img, cv::Mat img);
    vector<cv::Point> box_sort(vector<cv::Point> box, float roate);
    void display(const cv::Mat img, box_text box, string name, int times);
    void Regions_analysis(cv::Mat img, vector<box_coll> &regions, int model, string path);
    void display_point_box(const cv::Mat img, vector<cv::Point> box_point, string name, int times);
    void display_point_boxs(const cv::Mat img, vector<vector<cv::Point>> boxs, string name, cv::Scalar color, int times);
    void display_text_box(const cv::Mat img, vector<box_text> boxs, string name, cv::Scalar color, int times);
    void display_text_boxs(const cv::Mat img, vector<vector<box_text>> boxs, string name, cv::Scalar color, int times);

    vector<box_text> find_left_chinese(vector<box_coll> regions);
    vector<vector<cv::Point>> find_all_chines_by(string name);
    vector<vector<box_text>> find_all_by_left(vector<box_text> sort_regions, vector<box_coll> regions);
    cv::Rect combine_rect(vector<cv::Rect> rects);
    vector<cv::Rect> find_better_match_with(vector<box_text> regions_five, vector<box_coll> regions, string name, float scale);

    box_coll get_box(string name)
    {
        if (name == "face")
        {
            return face_box;
        }
        else if (name == "card")
        {
            return card_box;
        }
        else if (name == "number")
        {
            return number_box;
        }
    };

private:
    cv::Mat resize_img_copy;
    int ori_img_heigh;
    int ori_img_width;
    int ori_img_area;

    int resize_img_w;
    int resize_img_h;
    int resize_img_area;

    float box_rotate;
    int box_heigh;
    int box_width;
    int box_area;
    float box_scale;

    box_coll card_box, face_box, number_box;
};

#endif