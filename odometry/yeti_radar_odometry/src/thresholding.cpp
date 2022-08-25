//#include <opencv2>
#include <iostream>

using namespace cv;
using namespace std;
static inline bool exists(const std::string& name) {
    struct stat buffer;
    return !(stat (name.c_str(), &buffer) == 0);
}

// assumes file names are EPOCH times which can be sorted numerically
struct less_than_img {
    inline bool operator() (const std::string& img1, const std::string& img2) {
        std::vector<std::string> parts;
        boost::split(parts, img1, boost::is_any_of("."));
        int64 i1 = std::stoll(parts[0]);
        boost::split(parts, img2, boost::is_any_of("."));
        int64 i2 = std::stoll(parts[0]);
        return i1 < i2;
    }
};

void get_file_names(std::string path, std::vector<std::string> &files, std::string extension) {
    DIR *dirp = opendir(path.c_str());
    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL) {
        if (exists(dp->d_name)) {
            if (!extension.empty()) {
                std::vector<std::string> parts;
                boost::split(parts, dp->d_name, boost::is_any_of("."));
                if (parts[parts.size() - 1].compare(extension) != 0)
                    continue;
            }
            files.push_back(dp->d_name);
        }
    }
    // Sort files in ascending order of time stamp
    std::sort(files.begin(), files.end(), less_than_img());
}

void load_radar(std::string path, std::vector<int64_t> &timestamps, std::vector<double> &azimuths,
    std::vector<bool> &valid, cv::Mat &fft_data, int navtech_version) {
    int encoder_size = 5600;
    cv::Mat raw_example_data = cv::imread(path, cv::IMREAD_GRAYSCALE);
    // RADAR polar image dimension = 3371 x 400 with metadata
    // without metadata it is 3360 x 400
    // N = 400 size of timestamps, azimuths, valid = 400
    int N = raw_example_data.rows;
    timestamps = std::vector<int64_t>(N, 0);
    azimuths = std::vector<double>(N, 0);
    valid = std::vector<bool>(N, true);
    int range_bins = 3768;
    if (navtech_version == CIR204)
        range_bins = 3360;
    fft_data = cv::Mat::zeros(N, range_bins, CV_32F);
#pragma omp parallel
    for (int i = 0; i < N; ++i) {
        uchar* byteArray = raw_example_data.ptr<uchar>(i);
        timestamps[i] = *((int64_t *)(byteArray));
        azimuths[i] = *((uint16_t *)(byteArray + 8)) * 2 * M_PI / double(encoder_size);
        valid[i] = byteArray[10] == 255;
        for (int j = 42; j < range_bins; j++) { //j=42
            fft_data.at<float>(i, j) = (float)*(byteArray + 11 + j) / 255.0;
        }
    }
}

int main()
{
    // Loading the actual image
    Mat image, gray;
    Mat thresh1, thresh2;
    
    string data_dir = "D:\Multicoreware\odometry\oxford_polar\cartesian\";
    std::vector<std::string> radar_files;
    get_file_names(datadir, radar_files);
    for (uint i = 0; i < radar_files.size() - 1; ++i) 
    {
        load_radar(datadir + "/" + radar_files[i], times, azimuths, valid, fft_data, CIR204); // use CIR204 for MulRan dataset
        cv::imshow("fft_data",fft_data);

    }

}