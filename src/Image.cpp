#include "Image.hpp"

using namespace std;
using namespace cv;

Image_Process::Image_Process(){
    puts("[INFO]>>> Image类初始化完成");
}

Image_Process::~Image_Process(){
    puts("[INFO]>>> Image类释放");
}   

bool cmp_area(const box_coll &box1, const box_coll &box2){


    if(box1.box_area != box2.box_area){
        return box1.box_area < box2.box_area;
    }
}

bool cmp_x(const cv::Point &point1, const cv::Point &point2){

    if(point1.x != point2.x){
        return point1.x < point2.x;
    }
}

bool cmp_y(const cv::Point &point1, const cv::Point &point2){

    if(point1.y != point2.y){
        return point1.y < point2.y;
    }
}

bool cmp_box_x(const box_text &box1, const box_text &box2){
    if(box1.left_top_x != box2.left_top_x){
        return box1.left_top_x < box2.left_top_x;
    }

}
bool cmp_box_y(const box_text &box1, const box_text &box2){
    if(box1.left_top_y != box2.left_top_y){
        return box1.left_top_y < box2.left_top_y;
    }

}

void Image_Process::display(const cv::Mat img, box_text box, string name, int times){

    cv::Mat rect_img = img.clone();
    cv::Point p1 = cv::Point(box.left_top_x, box.left_top_y);
    cv::Point p2 = cv::Point(box.left_top_x + box.width, box.left_top_y + box.heigh);
    cv::rectangle(rect_img, p1, p2, cv::Scalar(0, 0, 255), 1);
    cv::imshow(name, rect_img);
    cv::waitKey(times);
}

void Image_Process::display_point_box(const cv::Mat img, vector<cv::Point> box_point, string name, int times){

    int begin_x, width, begin_y, heigh;
    begin_x = box_point[0].x;
    width = box_point[2].x - begin_x;
    begin_y = box_point[0].y;
    heigh = box_point[2].y - begin_y;

    cv::Rect rect(begin_x, begin_y, width, heigh);
    cv::Mat crop_img = img(rect).clone();
    cv::imshow(name, crop_img);
    cv::waitKey(times);

}

void Image_Process::display_point_boxs(const cv::Mat img, vector<vector<cv::Point>> boxs, string name, cv::Scalar color, int times){

    cv::Mat rect_img = img.clone();

    for(auto box_point:boxs){
        cv::Point p1 = cv::Point(box_point[0].x, box_point[0].y);
        cv::Point p2 = cv::Point(box_point[2].x, box_point[2].y);
        cv::rectangle(rect_img, p1, p2, color, 1);
    }
    cv::imshow(name, rect_img);
    cv::waitKey(times);

}

void Image_Process::display_text_box(const cv::Mat img, vector<box_text> boxs, string name, cv::Scalar color, int times){

    cv::Mat rect_img = img.clone();
    for(auto box_point:boxs){
        cv::Point p1 = cv::Point(box_point.left_top_x, box_point.left_top_y);
        cv::Point p2 = cv::Point(box_point.left_top_x + box_point.width, box_point.left_top_y + box_point.heigh);
        cv::rectangle(rect_img, p1, p2, color, 1);
    }
    cv::imshow(name, rect_img);
    cv::waitKey(times);
}

void Image_Process::display_text_boxs(const cv::Mat img, vector<vector<box_text>> boxs, string name, cv::Scalar color, int times){

    cv::Mat rect_img = img.clone();
    for(auto box:boxs){
        for(auto box_point:box){
            cv::Point p1 = cv::Point(box_point.left_top_x, box_point.left_top_y);
            cv::Point p2 = cv::Point(box_point.left_top_x + box_point.width, box_point.left_top_y + box_point.heigh);
            cv::rectangle(rect_img, p1, p2, color, 1);
        }
    }
    cv::imshow(name, rect_img);
    cv::waitKey(times);

}

bool filter_box(int max_heigh, int max_width, cv::Mat box){
    
    /*
    ##通过设定最大边框来过滤超出边界的box
        max_heigh:允许的边框最大高度
        max_width:允许的边框最大宽度
        box:Mat类型的矩形框
    */
    int v, x_min, x_max, y_min, y_max;
    vector<int> v_x, v_y;
    for(int i=0; i<box.rows; i++){
        int *ptr = box.ptr<int>(i);
        for(int j=0; j<box.cols+1; j++){
            v = ptr[j];
            if(j == 0){
                v_x.push_back(v);
            }
            else{
                v_y.push_back(v);
            }
        }
    }
    sort(v_x.begin(), v_x.end());
    x_min = v_x[0];
    reverse(v_x.begin(), v_x.end());
    x_max = v_x[0];

    sort(v_y.begin(), v_y.end());
    y_min = v_y[0];
    reverse(v_y.begin(), v_y.end());
    y_max = v_y[0];

    if(x_min <= 0 || x_max >= max_width){
        return false;
    }
    if(y_min <= 0 || y_max >= max_heigh){
        return false;
    }

    return true;
}


cv::Mat f2int_box(cv::Mat box){
    /*
    浮点型Mat数据转int型数据
        box:Mat类型的矩形框
    */

    vector<int> outarray;
    cv::Mat new_box;
    
    for(int i=0; i<box.rows; i++){
        float *ptr = box.ptr<float>(i);
        for(int j=0; j<box.cols; j++){
            float value = box.at<float>(i, j);
            outarray.push_back((int)value);
        }
    }
    new_box = cv::Mat(outarray);
    new_box = new_box.reshape(2, 4).clone();
    return new_box;

}

vector<cv::Point> Image_Process::box_sort(vector<cv::Point> box, float rotate){

    /*
    通过旋转矩形的旋转角度返回固定的box各个定点
    定点存储顺序为：左上->右上->右下->左下
        box:待排序的原始输入point容器
        rotate:box对应的旋转角度
    */

    vector<cv::Point> sort_box;
    cv::Point left_bottom;
    cv::Point left_top;
    cv::Point right_top;
    cv::Point right_bottom;

    if(rotate > -2){
        left_bottom = box[0];
        left_top = box[1];
        right_top = box[2];
        right_bottom = box[3];
    }
    else if(rotate < -80){
        left_bottom = box[1];
        left_top = box[2];
        right_top = box[3];
        right_bottom = box[0];   
    }
    else{
        sort(box.begin(), box.end(), cmp_x);
        int min_x = box[0].x;
        int max_x = box[box.size()-1].x;
        sort(box.begin(), box.end(), cmp_y);
        int min_y = box[0].y;
        int max_y = box[box.size()-1].y;
        left_bottom = cv::Point(min_x, max_y);
        left_top = cv::Point(min_x, min_y);
        right_top = cv::Point(max_x, min_y);
        right_bottom = cv::Point(max_x, max_y);
    }
    sort_box.push_back(left_top);
    sort_box.push_back(right_top);
    sort_box.push_back(right_bottom);
    sort_box.push_back(left_bottom);

    return sort_box;
}



void filter_by_box(vector<box_coll> &regions, box_coll filter_box, string type_name){

    /*
    通过计算 原始矩形框与筛选矩形框的交集 / 原始矩形框 获取重叠
    比例来筛选原始矩形框
        regions:所有矩形框的集合
        filter_box:用于筛选的矩形框
        type_name:选择筛选内部或者外部矩形框
    */

    vector<cv::Point> box;
    float m_iou;
    int width, heigh, left_top_x, left_top_y;
    cv::Rect U, rect1, f_rect;

    try{

        f_rect = cv::Rect(filter_box.box_point[0].x, filter_box.box_point[0].y, filter_box.box_width, filter_box.box_heigh);

        for(int i=0; i < regions.size(); i++){
            width = regions[i].box_width;
            heigh = regions[i].box_heigh;
            box = regions[i].box_point;
            left_top_x = box[0].x;
            left_top_y = box[0].y;

            rect1 = cv::Rect(left_top_x, left_top_y, width, heigh);
            U = f_rect & rect1;
            m_iou = U.area()*1.0/rect1.area();
            if(type_name == "outside"){
                if(m_iou < 0.5){
                    // printf("[INFO]>>> 过滤外部:%d\n", regions[i].box_index);
                    regions[i].box_index = -1;
                }
            }
            else{
                if(m_iou > 0.7){
                    // printf("[INFO]>>> 过滤内部:%d\n", regions[i].box_index);
                    regions[i].box_index = -1;
                }
            }
        }
    }
    catch(cv::Exception e){
        puts("[INFO]>>> func except --> filter_by_box");
    }

}

cv::Mat Image_Process::Resize_pic(cv::Mat img, int dst_size){

    /*
    图像按照宽度等比例缩放
        img:Mat类型的原始图像数据
        dst_size:输出目标图像宽
    */

    cv::Mat resize_img;
    ori_img_width = img.size().width;
    ori_img_heigh = img.size().height;
    ori_img_area = ori_img_width * ori_img_heigh;

    resize_img_w = dst_size;
    float scale = static_cast<float>(resize_img_w) / ori_img_width;
    resize_img_h = (int)(scale * ori_img_heigh);
    resize_img_area  = resize_img_w * resize_img_h;
    cv::Size new_size = cv::Size(resize_img_w, resize_img_h);
    cout << "[INFO]>>> 缩放后尺寸:" << new_size << endl;
    
    cv::resize(img, resize_img, new_size);
    resize_img_copy = resize_img.clone();
    return resize_img;

}

cv::Mat Image_Process::Rotate_image(cv::Mat src, float angle){
    /*
    图像旋转当图像宽小于高时
        src:原始输入图像
        angle:旋转角度
    */
	cv::Mat dst;
	try{			    	
		//输出图像的尺寸与原图一样    
		cv::Size dst_sz(src.cols, src.rows);
 
		//指定旋转中心      
		cv::Point2f center(static_cast<float>(src.cols / 2.), static_cast<float>(src.rows / 2.));
 
		//获取旋转矩阵（2x3矩阵）      
		cv::Mat rot_mat = cv::getRotationMatrix2D(center, angle, 1.0);
		//设置选择背景边界颜色   
		/*cv::Scalar borderColor = Scalar(0, 238, 0);*/
		/*cv::warpAffine(src, dst, rot_mat, src.size(), INTER_LINEAR, BORDER_CONSTANT, borderColor);*/
		//复制边缘填充
		cv::warpAffine(src, dst, rot_mat, dst_sz, cv::INTER_LINEAR, cv::BORDER_REPLICATE);
	}
	catch (cv::Exception e){
	}
	
	return dst;
}

cv::Mat Image_Process::denoise(cv::Mat binary){
    /*
    当某个像素点值为255,但是其上下左右像素值都是0的话,
    认做为孤立像素点,将其值也设置为0
    binary:二值图像
    */
    for(int i=1; i < binary.rows-1; i++){
        for(int j=1; j < binary.cols-1; j++){
            if(binary.at<int>(i, j) == 255){
                if(binary.at<int>(i-1, j) == binary.at<int>(i+1, j) == binary.at<int>(i, j-1) == binary.at<int>(i, j+1) == 0){
                    binary.at<int>(i, j) = 0;
                }
            }
        }
    }
    return binary;
}

cv::Mat Image_Process::Img_preprocess(cv::Mat img){
    /*
    图像处理pipeline得到文本人像框区域
        img:缩放至(428x270)的图像
    */

    cv::Mat img_dilation;

    try{

        // # 1.滤波去除底纹
        cv::Mat img_denoise;
        cv::fastNlMeansDenoisingColored(img, img_denoise, 8, 8, 7, 21); 

        // # 2.转换成灰度图
        cv::Mat img_gray;
        cv::cvtColor(img_denoise, img_gray, cv::COLOR_RGB2GRAY);

        // # 3.反向二值化图像获取文本人像区域
        cv::Mat img_binary_inv;
        cv::adaptiveThreshold(img_gray, img_binary_inv, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 9, 3);

        // # 4.自定义的降噪代码,将孤立噪声像素点去除
        img_binary_inv = denoise(img_binary_inv);

        // # 5.膨胀操作,将图像变成一个个矩形框，找到文本以及人像对应的区域
        cv::Mat ele = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(10, 5));  
        
        cv::dilate(img_binary_inv, img_dilation, ele);
    }
    catch(cv::Exception e){
        puts("[ERROR]>>> func except --> Img_preprocess");

    }

    return img_dilation;
}


vector<box_coll> Image_Process::find_all_region(cv::Mat ori_img, cv::Mat img){
    /*
    找到所有可能的文本框区域并过滤掉噪声区域
    ori_img: size为(428x270)的原始小图
    img: 同为(428x270)的二值图
    */
    vector<box_coll> card_region;
    vector<vector<cv::Point>> contours;

    int save_regions = 0;

    try{
        cv::Mat rect_img = ori_img.clone();
        cv::findContours(img, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
        vector<vector<cv::Point>> box_points(contours.size());

        for(int i=0; i<contours.size(); i++){
            cv::RotatedRect rect = cv::minAreaRect(contours[i]);
            cv::Mat box;
            cv::Mat new_box;
            cv::boxPoints(rect, box);
            // cout << "box:" << box << endl;

            new_box = f2int_box(box);
            // cout << "new_box:" << new_box << endl;
            bool flags = filter_box(resize_img_h, resize_img_w, new_box);
            

            box_rotate = rect.angle;
            box_width = rect.size.height;
            box_heigh = rect.size.width;
            box_area = rect.size.area();
            box_scale = box_width > box_heigh ? (static_cast<float>(box_width) / static_cast<float>(box_heigh)):(static_cast<float>(box_heigh) / static_cast<float>(box_width));
            float box_rotate_ = box_rotate > -45 ? fabs(box_rotate):(90 + box_rotate);

            if(flags && box_area >= 150 && (box_heigh > 8 && box_width > 8 ) && box_scale < 16.){
                for(int m=0; m<4; m++){
                    int n = (m + 1) % 4;
                    cv::Point p1 = cv::Point(new_box.at<int>(m, 0), new_box.at<int>(m, 1));
                    cv::Point p2 = cv::Point(new_box.at<int>(n, 0), new_box.at<int>(n, 1));
                    box_points[i].push_back(p1);
                    // cv::line(rect_img, p1, p2, cv::Scalar(0, 0, 255), 1);
                }

                // string text = to_string(save_regions);
                // cv::Point left_top = cv::Point(new_box.at<int>(0,0), new_box.at<int>(0, 1));
                // cv::putText(rect_img, text, left_top, cv::FONT_HERSHEY_COMPLEX, 0.4, cv::Scalar(255, 0, 0), 1, 8, 0);

                box_coll box_msg;
                box_msg.box_index = save_regions;
                box_msg.box_point = box_points[i];
                box_msg.box_rotate = box_rotate;
                box_msg.box_area = box_area;
                card_region.push_back(box_msg);
                save_regions += 1;
            }
            
        }

        // cv::imshow("rect", rect_img);
        // cv::waitKey(0);
    }
    catch(cv::Exception e){
        puts("[ERROR]>>> func except --> find_all_region");

    }
    return card_region;
}


void Image_Process::Regions_analysis(cv::Mat img, vector<box_coll> &regions, int model, string path){

    /*
    分析计算经过初步筛选所得到的所有矩形区域，找到
    完整身份证，人像，身份证号码的矩形区域，并通过身份证和人像框
    进一步筛除干扰矩形框，将干扰框的index索引置为-1
        img:resize的原始彩色图像，只是用于调试时显示
        regions:所有可能的矩形区域信息
        model:模式为0及旋转图像时不处理，为以后升级做的预留
        path:用于存储检索出来的完整身份证，人像，身份证号码图像
            只调试时使用
    */

    int id, width, heigh, area;
    float rotate, scale;
    vector<cv::Point> box;
    vector<cv::Point> sort_box;
    cv::Point p1, p2, left_top;

    try{
        if(model == 0){
            cout << "旋转图片暂时不处理" << endl;
        }
        cv::Mat rect_img = img.clone();
        sort(regions.begin(), regions.end(), cmp_area);
        reverse(regions.begin(), regions.end());
        
        for(int i=0; i<regions.size(); i++){
            id = regions[i].box_index;
            rotate = regions[i].box_rotate;

            //将原本的box置换成统一排序规则的box
            box = regions[i].box_point;
            sort_box = box_sort(box, rotate);
            regions[i].box_point = sort_box;

            // 显示坐标排序过后的box
            p1 = cv::Point(sort_box[0].x, sort_box[0].y);
            p2 = cv::Point(sort_box[2].x, sort_box[2].y);
            cv::rectangle(rect_img, p1, p2, cv::Scalar(0, 0, 255), 1);

            string text = to_string(id);
            left_top = cv::Point(sort_box[0].x, sort_box[0].y);
            cv::putText(rect_img, text, left_top, cv::FONT_HERSHEY_COMPLEX, 0.35, cv::Scalar(255, 0, 0), 1, 8, 0);

            // 存储准确的矩形框信息
            width = regions[i].box_width = sort_box[2].x - sort_box[0].x;
            heigh = regions[i].box_heigh = sort_box[2].y - sort_box[0].y;
            scale = regions[i].box_scale = width > heigh ? static_cast<float>(width) / heigh : static_cast<float>(heigh) / width;
            area = regions[i].box_area = width * heigh;
            // printf("id:%d, area:%d, x:%d, y:%d, scale:%f, rotate:%f width:%d, heigh:%d\n", id, area, p1.x, p1.y, scale, rotate, width, heigh);

            // # 0.身份证号码边框检测, 可信度比较高
            if(scale > 8. && scale < 16. && sort_box[2].x > 0.7*resize_img_w && sort_box[2].y > 0.7*resize_img_h){
                if(area > number_box.box_area && area < 1e4){
                    // cout << "发现number" << endl;
                    number_box.box_index = id;
                    number_box.box_area = area;
                    number_box.box_scale = scale;
                    number_box.box_point = sort_box;
                    number_box.box_rotate = rotate;
                    number_box.box_width = width;
                    number_box.box_heigh = heigh;
                }
            }
            // # 1.找到完整身份证边框
            if(static_cast<float>(area) / resize_img_area > 0.4 && abs(scale - 1.58) < 0.06){
                if(card_box.box_area == 0 || area < card_box.box_area){
                    // cout << "发现card" << endl;
                    card_box.box_index = id;
                    card_box.box_area = area;
                    card_box.box_scale = scale;
                    card_box.box_point = sort_box;
                    card_box.box_rotate = rotate;
                    card_box.box_width= width;
                    card_box.box_heigh = heigh;
                }
                else{
                    continue;
                }
            }
            // # 2.在检测到完整的身份证边框前提下检测到了人像框
            else if(card_box.box_area != 0){
                if(static_cast<float>(area) / card_box.box_area > 0.12 && static_cast<float>(area) / card_box.box_area < 0.2 && abs(scale - 1.1) < 0.3){
                    if(face_box.box_area == 0 || abs(scale) - 1.1 <= face_box.box_scale){
                        // cout << "存在card前提下发现face" << endl;
                        face_box.box_index = id;
                        face_box.box_area = area;
                        face_box.box_scale = scale;
                        face_box.box_point = sort_box;
                        face_box.box_rotate = rotate;
                        face_box.box_width = width;
                        face_box.box_heigh = heigh;
                    }
                }
                else{
                    continue;
                }
            }
            // # 3.在没有检测到完整身份证边框的前提下检测到人相框
            else if(abs(scale - 1.1) < 0.35 && area > 1e4 && i <= 4 && sort_box[3].x > 0.5 * resize_img_w && sort_box[3].y > 0.5 * resize_img_h){
                if(face_box.box_area == 0 || area > face_box.box_area){
                    // cout << "发现face" << endl;
                    face_box.box_index = id;
                    face_box.box_area = area;
                    face_box.box_scale = scale;
                    face_box.box_point = sort_box;
                    face_box.box_rotate = rotate;
                    face_box.box_width = width;
                    face_box.box_heigh = heigh;
                }
                else{
                    continue;
                }
            }
        }

        cv::imshow("new_rect", rect_img);
        cv::waitKey(0);

        if(card_box.box_area != 0){
            // display_point_box(img, card_box.box_point, "card", 0);
            filter_by_box(regions, card_box, "outside");

            vector<vector<cv::Point>> all_box_point = find_all_chines_by("card");
            display_point_boxs(img, all_box_point, "from card", cv::Scalar(0, 0, 255), 0);
        }
        if(face_box.box_area != 0){
            // display_point_box(img, face_box.box_point, "face", 0);
            filter_by_box(regions, face_box, "inside");

            vector<vector<cv::Point>> all_box_point = find_all_chines_by("face");
            display_point_boxs(img, all_box_point, "from face", cv::Scalar(255, 0, 0), 0);
        }
        if(number_box.box_area != 0){
            // display_point_box(img, number_box.box_point, "number", 0);

            vector<vector<cv::Point>> all_box_point = find_all_chines_by("number");
            display_point_boxs(img, all_box_point, "from number", cv::Scalar(255, 0, 0), 0);
            ;
        }
    }
    catch(cv::Exception e){
        puts("[ERROR]>>> func except --> Regions_analysis");

    }

}

vector<vector<cv::Point>> Image_Process::find_all_chines_by(string name){

    /*
    通过指定推测类型来推测姓名，性别，名族，出生，住址等关键信息
        name:用于推测的类型，可为（"card", "face", "number"）
    */

    vector<vector<cv::Point>> all_box_point;

    try{
        if(name == "card"){

            int card_x = card_box.box_point[0].x;
            int card_y = card_box.box_point[0].y;
            // printf("[INFO]>>> x0:%d, y0:%d", card_x, card_y);
            int card_w = card_box.box_width;
            int card_h = card_box.box_heigh;
            // printf("[INFO]>>> w:%d, h:%d", card_w, card_h);

            vector<scale_card> scales;
            scale_card name_scale = {0.1720, 0.6173, 0.1167, 0.2228};
            scale_card sex_scale = {0.1720, 0.2393, 0.2553, 0.3517};
            scale_card nation_scale = {0.3805, 0.6173, 0.2553, 0.3517};
            scale_card brith_scale = {0.1720, 0.6173, 0.3714, 0.4631};
            scale_card addres_scale = {0.1720, 0.6173, 0.5026, 0.7816};
            // scale_card number_scale = {0.3234, 0.8976, 0.8176, 0.9244};
            scales.push_back(name_scale);
            scales.push_back(sex_scale);
            scales.push_back(nation_scale);
            scales.push_back(brith_scale);
            scales.push_back(addres_scale);
            // scales.push_back(number_scale);

            // cout << "scales size:" << scales.size() << endl;

            for(auto scale:scales){
                int x0, x1, y0, y1;
                vector<cv::Point> Chinese_box;
                x0 = int(card_x + card_w * scale.scale_x0);
                y0 = int(card_y + card_h * scale.scale_y0);
                x1 = int(card_x + card_w * scale.scale_x1);
                y1 = int(card_y + card_h * scale.scale_y1);
                Chinese_box.push_back(cv::Point(x0, y0));
                Chinese_box.push_back(cv::Point(x1, y0));
                Chinese_box.push_back(cv::Point(x1, y1));
                Chinese_box.push_back(cv::Point(x0, y1));
                all_box_point.push_back(Chinese_box);
            }

        }
        else if(name == "face"){

            vector<cv::Point> Chinese_box;
            float scale_x[] = {1.36, 1.00};

            int face_x = face_box.box_point[3].x;
            int face_y = face_box.box_point[3].y;
            // printf("[INFO]>>> x0:%d, y0:%d \n", face_x, face_y);
            int face_w = face_box.box_width;
            int face_h = face_box.box_heigh;
            // printf("[INFO]>>> w:%d, h:%d \n", face_w, face_h);

            float scale_y[] = {1.04, 0.82, 0.82, 0.62, 0.38};
            float h_list[] = {0.15, 0.15, 0.15, 0.15, 0.15};

            for(int i=0; i<5; i++){
                int x0, x1, y0, y1;
                Chinese_box.clear();
                if(i != 2){
                    x0 = (int)(face_x - scale_x[0] * face_w);
                }
                else{
                    x0 = (int)(face_x - scale_x[1] * face_w);
                }
                x0 = x0 > 0 ? x0:0;
                if(i < 4){
                    y0 = (int)(face_y - scale_y[i] * face_h);
                    y1 = (int)(y0 + h_list[i] * face_w);
                    if(i != 1){
                        x1 = face_x;  
                    }
                    else{
                        x1 = x0 + 1.5*(y1 - y0);
                    }
                }
                else if(i == 4){
                    y0 = (int)(face_y - scale_y[i] * face_h);
                    x1 = face_x;
                    y1 = face_y;
                }
                y0 -= 3;
                // y1 += 3;
                Chinese_box.push_back(cv::Point(x0, y0));
                Chinese_box.push_back(cv::Point(x1, y0));
                Chinese_box.push_back(cv::Point(x1, y1));
                Chinese_box.push_back(cv::Point(x0, y1));
                all_box_point.push_back(Chinese_box);
            }
        }
        else if(name == "number"){

            vector<cv::Point> Chinese_box;
            float scale_x[] = {0.285, 0.285, 0.10, 0.285, 0.285};
            float scale_y[] = {0.79, 0.64, 0.64, 0.52, 0.35};
            float h_list[] = {1.1, 1.1, 1.1, 1.1, 2.0};

            int numb_x = number_box.box_point[0].x;
            int numb_y = number_box.box_point[0].y;
            int numb_w = number_box.box_width;
            int numb_h = number_box.box_heigh;

            for(int i=0; i<5; i++){
                int x0, x1, y0, y1;
                Chinese_box.clear();
                x0 = (int)(numb_x - scale_x[i] * numb_w);
                x0 = x0 > 0 ? x0:0;
                if(i < 4){
                    y0 = (int)(numb_y - scale_y[i] * numb_w);
                    y1 = (int)(y0 + h_list[i] * numb_h) + 3;
                    if(i != 1){
                        x1 = numb_x + (int)(0.49 * number_box.box_width);  
                    }
                    else{
                        x1 = x0 + 1.5*(y1 - y0);
                    }
                }
                else if(i == 4){
                    y0 = (int)(numb_y - scale_y[i] * numb_w);
                    x1 = numb_x + (int)(0.49 * number_box.box_width); ;
                    y1 = numb_y - (int)(0.11 * numb_w);
                }
                y0 -= 3;
                // y1 += 3;
                Chinese_box.push_back(cv::Point(x0, y0));
                Chinese_box.push_back(cv::Point(x1, y0));
                Chinese_box.push_back(cv::Point(x1, y1));
                Chinese_box.push_back(cv::Point(x0, y1));
                all_box_point.push_back(Chinese_box);
            }

        }

    }
    catch(cv::Exception e){
        puts("[ERROR]>>> func except --> find_all_chines_by");

    }
    return all_box_point;

}


vector<box_text> Image_Process::find_left_chinese(vector<box_coll> regions){

    /*
    找到最左边的五个基准文本框的位置，用于进一步收集每一个基准框所对应行的
    其余文本框.
        regions:所有的矩形区域
    */


    vector<box_text> text_regions; 
    vector<box_text> sort_regions;
    box_text text;
    int x0, y0, w, h;

    try{
        for(int i=0; i<regions.size(); i++){
            int id = regions[i].box_index;
            if(id == -1){
                continue;
            }
            int area = regions[i].box_area;
            if(area > 0.2 * resize_img_area || i < 3){
                continue;
            }
            
            x0 = regions[i].box_point[0].x;
            y0 = regions[i].box_point[0].y;
            w = regions[i].box_width;
            h = regions[i].box_heigh;
            if(static_cast<float>(h) / w >= 1.5){
                continue;
            }
            
            text.box_index = id;
            text.left_top_x = x0;
            text.left_top_y = y0;
            text.width = w;
            text.heigh = h;
            text_regions.push_back(text);
        }

        sort(text_regions.begin(), text_regions.end(), cmp_box_x);

        //找到符合条件的五个基准框
        int counts;
        int find_index = -1;
        for(int i=0; i<4; i++){
            counts = 0;
            for(int j=i+1; j<text_regions.size(); j++){
                // cout << text_regions[i].left_top_x - text_regions[j].left_top_x << ", " << endl;
                if(abs(text_regions[i].left_top_x - text_regions[j].left_top_x) > 12){
                    break;
                }
                else{
                    counts ++;
                }
            }
            if(counts >= 4){
                find_index = i;
                break;
            }
        }
        if(find_index != -1){
            for(int i=find_index; i<find_index+5; i++){
                sort_regions.push_back(text_regions[i]);
            }
        }

        if(sort_regions.size() != 5){
            cout << "[INFO]>>> 检索失败" << endl;
        }
        else{
            cout << "[INFO]>>> 检索成功" << endl;
            sort(sort_regions.begin(), sort_regions.end(), cmp_box_y);
        }
        for(auto text:sort_regions){
            printf("[INFO]>>> id:%d, x0:%d, y0:%d, w:%d, h:%d \n", text.box_index, text.left_top_x, text.left_top_y, text.width, text.heigh);
        }
    }
    catch(cv::Exception e){
        puts("[ERROR]>>> func except --> find_left_chinese");

    }

    return sort_regions;
}

vector<vector<box_text>> Image_Process::find_all_by_left(vector<box_text> sort_regions, vector<box_coll> regions){

    /*
    通过检索到的五个基准框进一步收集所有的关键文本信息框
        sort_regions:检索到的五个基准框
        regions:所有可能的矩形区域
    */

    vector<vector<box_text>> out_ppp;
    vector<box_text> temp_box;
    box_text temp_box_text;
    int x0, y0, w, h;
    int x_end, x_card, x_face;
    int y_end, y_card, y_face;

    try{
        if(card_box.box_area != 0 && face_box.box_area != 0){
            x_card = card_box.box_point[0].x + (int)(0.61 * card_box.box_width);
            x_face = face_box.box_point[3].x;
            x_end = int(0.6*x_face + 0.4*x_card);
        }
        else if(card_box.box_area != 0){
            x_end = card_box.box_point[0].x + (int)(0.61 * card_box.box_width);
        }
        else if(face_box.box_area != 0){
            x_end = face_box.box_point[3].x;
        }

        if(number_box.box_area != 0){
            y_end = number_box.box_point[0].y;
        }
        else if(card_box.box_area != 0){
            y_end = card_box.box_point[0].x + 0.8 * card_box.box_heigh;
        }
        
        for(int i=0; i < sort_regions.size(); i++){
            temp_box.clear();
            temp_box.push_back(sort_regions[i]);
            out_ppp.push_back(temp_box);

            for(int j=0; j<regions.size(); j++){
                if(regions[j].box_index == sort_regions[i].box_index){
                    continue;
                }
                else if(regions[j].box_index == -1){
                    continue;
                }
                else{
                    x0 = regions[j].box_point[0].x;
                    y0 = regions[j].box_point[0].y;
                    w = regions[j].box_width;
                    h = regions[j].box_heigh;
                    // printf("[INFO]>>> i:%d, j:%d, x0:%d, y0:%d, w:%d, h:%d\n", i, j, x0, y0, w, h);
                    //由于地址框可能有多行所以特别处理
                    if(i != 3){
                        if(x0 - x_end >= 0 || x0 < sort_regions[i].left_top_x){
                            continue;
                        }
                        else if(abs(y0 - sort_regions[i].left_top_y) > 10 || abs(y0 + h - sort_regions[i].left_top_y - sort_regions[i].heigh) > 10){
                            continue;
                        }
                        else{
                            temp_box_text.box_index = regions[j].box_index;
                            temp_box_text.left_top_x = regions[j].box_point[0].x;
                            temp_box_text.left_top_y = regions[j].box_point[0].y;
                            temp_box_text.width = regions[j].box_width;
                            temp_box_text.heigh = regions[j].box_heigh;
                            out_ppp[i].push_back(temp_box_text);
                        }
                    }
                    else{
                        if(x0 - x_end >= 0 || x0 < sort_regions[i].left_top_x){
                            continue;
                        }
                        else if(y0 < sort_regions[i].left_top_y - 10 || (y0 + h - y_end ) > 0){
                            continue;
                        }
                        else{
                            temp_box_text.box_index = regions[j].box_index;
                            temp_box_text.left_top_x = regions[j].box_point[0].x;
                            temp_box_text.left_top_y = regions[j].box_point[0].y;
                            temp_box_text.width = regions[j].box_width;
                            temp_box_text.heigh = regions[j].box_heigh;
                            out_ppp[i].push_back(temp_box_text);
                        }
                        
                    }

                }

            }
        }
        cout << "[INFO]>>> 基准框个数:" << out_ppp.size() << endl;
        for(int i=0; i<out_ppp.size(); i++){
            printf("[INFO]>>> 第%d个基准框包含框个数:%d\n", i, out_ppp[i].size());
        }
    }
    catch(cv::Exception e){
        puts("[ERROR]>>> func except --> find_all_by_left");

    }
    return out_ppp;
}

cv::Rect Image_Process::combine_rect(vector<cv::Rect>rects){

    /*
    将多个矩形框进行合并得到一个能包含所有的子矩形框且面积最小的矩形框
    用于将识别到的多个文字框合并
        rects:待合并的子矩阵框集合
    */

    cv::Rect out_rect;
    vector<int> v_x, v_y;
    int min_x, min_y, max_x, max_y;

    try{
        for(int i=0; i<rects.size(); i++){
            v_x.push_back(rects[i].x);
            v_x.push_back(rects[i].x + rects[i].width);
            v_y.push_back(rects[i].y);
            v_y.push_back(rects[i].y + rects[i].height);
        }
        sort(v_x.begin(), v_x.end(), [](int a, int b)-> bool{if(a != b){return a < b;}});
        min_x = v_x[0];
        reverse(v_x.begin(), v_x.end());
        max_x = v_x[0];

        sort(v_y.begin(), v_y.end(), [](int a, int b)-> bool{if(a != b){return a < b;}});
        min_y = v_y[0];
        reverse(v_y.begin(), v_y.end());
        max_y = v_y[0];

        if(min_x >= max_x || min_y >= max_y){
            puts("[INFO]>>> combine_rect error !!!");
        }
        printf("[INFO]>>> 联合box--> min_x:%d, min_y:%d, max_x:%d, max_y:%d \n", min_x, min_y, max_x, max_y);
        out_rect = cv::Rect(min_x, min_y, max_x - min_x, max_y - min_y);
    }
    catch(cv::Exception e){
        puts("[ERROR]>>> func except --> combine_rect");

    }
    return out_rect;
}

vector<cv::Rect> Image_Process::find_better_match_with(vector<box_text> regions_five, vector<box_coll> regions, string name, float scale){

    /*
    结合通过基准框得到的文本信息框和根据身份证，人像，身份证号码推测得到的不太精确的
    相对应文本信息框进行m_iou比较, 从而进一步确定姓名，性别等关键区域。通过改变
    阈值可以增加或者降低检测难度。
        regions_five:检索得到的五个基准框
        regions:所有可能的矩形框
        name:iou比较类型
        scale:iou的比例阈值
    */

    vector<cv::Point> temp_box_point;
    vector<vector<cv::Rect>> temp_row_boxs;
    vector<cv::Rect> temp_infer_box;
    vector<cv::Rect> out_row_boxs;
    cv::Rect rect1, rect2, U;
    box_text box;

    try{
        vector<vector<box_text>> all_box_text =  find_all_by_left(regions_five, regions);
        vector<vector<cv::Point>> all_box_point = find_all_chines_by(name);
        // if(im_show){
        //     display_point_boxs(resize_img_copy, all_box_point, "point", cv::Scalar(0, 0, 255), 0);
        //     display_text_boxs(resize_img_copy, all_box_text, "text", cv::Scalar(255, 0, 0), 0);
        // }

        if(all_box_point.size() == all_box_text.size()){
          
            for(int i=0; i< all_box_point.size(); i++){

                temp_box_point = all_box_point[i];
                rect1 = cv::Rect(temp_box_point[0].x, temp_box_point[0].y, temp_box_point[2].x - temp_box_point[0].x, temp_box_point[2].y - temp_box_point[0].y);
                // cv::Mat crop_img = resize_img_copy(rect1).clone();
                // cv::imshow("aa", crop_img);
                // cv::waitKey(0);

                int j = (i>=2) ? i-1:i;
                for(int n=0; n<all_box_text[j].size(); n++){
                    box = all_box_text[j][n];
                    rect2 = cv::Rect(box.left_top_x, box.left_top_y, box.width, box.heigh);
                    U = rect1 & rect2;
                    float m_iou = U.area()*1.0/rect2.area();
                    if(m_iou >= scale){
                        temp_infer_box.push_back(rect2);

                    }
                    // printf("[INFO]>>> 第%d个基准框，第%d个子框面积:%d, 交集占比:%f\n", i, n, rect2.area(),  m_iou);

                    // crop_img = resize_img_copy(rect2).clone();
                    // cv::imshow("bb", crop_img);
                    // cv::waitKey(0);
                }
                if(temp_infer_box.empty()){
                    temp_infer_box.push_back(rect1);
                }
                temp_row_boxs.push_back(temp_infer_box);
                temp_infer_box.clear();
            }

            cv::Rect row_rect;
            for(int i=0; i<temp_row_boxs.size(); i++){
                temp_infer_box = temp_row_boxs[i];
                // printf("[INFO]>>> 第%d行包含子框个数:%d\n", i, temp_infer_box.size());
                if((i==0 || i==3 || i==2 ) && temp_infer_box.size() > 1){
                    sort(temp_infer_box.begin(), temp_infer_box.end(), [](Rect rect1, Rect rect2)-> bool{if(rect1.x != rect2.x){return rect1.x < rect2.x;}});
                    // printf("[INFO]>>> 第%d行, %d\n", i, temp_infer_box.size());
                    row_rect = combine_rect(temp_infer_box);
                }
                else if(i==4 && temp_infer_box.size() > 1){
                    sort(temp_infer_box.begin(), temp_infer_box.end(), [](Rect rect1, Rect rect2)-> bool{if(rect1.y != rect2.y){return rect1.y < rect2.y;}});
                    // printf("[INFO]>>> 第%d行, %d\n", i, temp_infer_box.size());
                    row_rect = combine_rect(temp_infer_box);
                }
                else{
                    row_rect = temp_infer_box[0];
                }
                out_row_boxs.push_back(row_rect);
            }    
        }
        else{
            cout << "[INFO]>>> box_point 和 box_text 长度不匹配" << endl;
        }
    }
    catch(char *msg){
        puts("[ERROR]>>> func except --> find_better_match_with");

    }

    return out_row_boxs;
}