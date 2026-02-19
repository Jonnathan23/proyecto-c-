#pragma once

#include <MainWindow.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace Utils {

bool isChecked(Ui::MainWindow *ui);

cv::Mat aplyFilter(Volumetrics volumetrics, cv::Mat processedSlice ,std::string effectName);

bool generateStatistics(Volumetrics &volumetrics,const cv::Mat &slice,const QString &outputFolder,QWidget *parent);

} // namespace Utils