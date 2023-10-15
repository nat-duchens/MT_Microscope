#include "MT_2_N52.h"
#include <xiApiPlusOcv.hpp>
#include <NIDAQmx.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <chrono>
#include <Windows.h>
#include <QtCore/QStandardPaths>
#include <iostream>
#include <fstream>
#include "stdio.h"
#include <QDebug>
#include <QShortcut>
#include <QAction>

using namespace cv;
using namespace std;
using namespace std::chrono;

//////////Function Declarations/////////////////
void onmouse(int event, int x, int y, int flags, void* param);
Mat FFT_MT(Mat Img);
float RadialVector(Mat Img, float *Profile);
void SetupDisplay();
float64 AI_Buffer[500];
float AI_average;
int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);

/////////////////Globals////////////////////////
Point M_1, M_2, R_1, R_2;
Point M_1S, M_2S, R_1S, R_2S;
int mouse_x, mouse_y, mouse_z;
int Save_mode_on = 0;
int mouse_c; int currentFocus; int lockFocus = 50; int middown;
float M_Stack[128][100]; float R_Stack[128][100];
__int64 now, t0;
int update_rate = 100;//samples/s
float Measuring_resistance = 2;//  in Ohm
int32 num_read;
boolean MP_flag;

MT_2_N52::MT_2_N52(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	QString v = "Rivas Pardo Lab MT_OptiimusV1";  ui.version->setText(v);
	MT_2_N52::setWindowTitle("MT_Optimus");
	QAction *shortcutROI;
	shortcutROI = new QAction("ShortcutROI", this);
	shortcutROI->setShortcut(QKeySequence(Qt::Key_R));
	addAction(shortcutROI);
	
	
	//connect(shortcutROI, &QAction::triggered, this, &MT_2_N52::show_ROI);
	//connect(shortcutROI, &QAction::triggered, ui.ROIButton, &QPushButton::isChecked);
	connect(ui.MeasureButton, &QPushButton::clicked, this, &MT_2_N52::measure);
	ui.MeasureButton->setCheckable(true);
	connect(ui.ROIButton, &QPushButton::clicked, this, &MT_2_N52::show_ROI);
	ui.ROIButton->setCheckable(true);
	connect(ui.stackButton, &QPushButton::clicked, this, &MT_2_N52::get_stack);
	connect(ui.FocusBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &MT_2_N52::Focus_box);
	ui.FocusBox->setMinimum(0); ui.FocusBox->setMaximum(18000); ui.FocusBox->setValue(9000);
	ui.FocusBox->setDecimals(0); ui.FocusBox->setSingleStep(100); ui.FocusBox->setValue(9000); ui.FocusBox->setKeyboardTracking(0);
	connect(ui.MagnetPositionBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &MT_2_N52::MagnetPosition_box);
	ui.MagnetPositionBox->setDecimals(2); ui.MagnetPositionBox->setMinimum(1.4); ui.MagnetPositionBox->setMaximum(9); ui.MagnetPositionBox->setSingleStep(0.1); ui.MagnetPositionBox->setValue(5.8); ui.MagnetPositionBox->setKeyboardTracking(0);
	connect(ui.MagnetPositionBox, SIGNAL(on_MagnetPositionBox_valueChanged), ui.SliderMP, SLOT(setValue));
	connect(ui.SliderMP, SIGNAL(on_SliderMP_valueChanged()), ui.MagnetPositionBox, SLOT(setValue));
	connect(ui.FocusBox, SIGNAL(on_FocusBox_valueChanged), ui.FocusSlider, SLOT(setValue));
	connect(ui.FocusSlider, SIGNAL(on_FocusSlider_valueChanged()), ui.FocusBox, SLOT(setValue));

	ui.MP_textBox->setText("MP (mm)"); MP_flag = true;
	ui.SaveDataButton->setCheckable(true);
	ui.cam_exposure->setMinimum(100); ui.cam_exposure->setMaximum(10000); ui.cam_exposure->setValue(500);//1000 us = 1 ms

	ui.SaveDataButton->setText("Save off");
	ui.View_fast_slow->setChecked(false);
	ShowButtons(0, 1, 0, 0);
	QWidget::move(50, 30);
	connect(ui.execute_pulse, &QPushButton::clicked, this, &MT_2_N52::Send_pulse);
	ui.execute_pulse->setCheckable(true);

	ui.Pulse_protocol->insertPlainText("5.8/15/C;\n");
	ui.Pulse_protocol->insertPlainText("5.8/2/L;\n");
	ui.Pulse_protocol->insertPlainText("2.5/30/C;\n");
	//ui.Pulse_protocol->insertPlainText("5.8/2/L;\n");
	
	ui.Pulse_protocol->moveCursor(QTextCursor::Start);

	cam.Close(); cam.OpenFirst(); cam.SetImageDataFormat(XI_RAW16); cam.StartAcquisition();

	DAQmxCreateTask("", &taskHandle_1); DAQmxCreateAOVoltageChan(taskHandle_1, "Dev1/ao1", "", -10, 10, DAQmx_Val_Volts, "");
	DAQmxTaskControl(taskHandle_1, DAQmx_Val_Task_Reserve); DAQmxStartTask(taskHandle_1);

	DAQmxCreateTask("", &taskHandle_2); DAQmxCreateAIVoltageChan(taskHandle_2, "Dev1/ai0", "", DAQmx_Val_Diff, -2.5, 2.5, DAQmx_Val_Volts, NULL);
	DAQmxCfgSampClkTiming(taskHandle_2, "", 450000.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 100); DAQmxRegisterEveryNSamplesEvent(taskHandle_2, DAQmx_Val_Acquired_Into_Buffer, 100, 0, EveryNCallback, NULL);
	DAQmxTaskControl(taskHandle_2, DAQmx_Val_Task_Reserve); DAQmxStartTask(taskHandle_2);

	t0 = now = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
	ShowButtons(0, 0, 1, 0);
	ui.BeadType->setChecked(false);
	ui.BeadType->setVisible(false);
	ui.expLabel->setVisible(false);
	ui.cam_exposure->setVisible(false);
	ui.FramesPerSecondText->setVisible(false);
	ui.View_fast_slow->setVisible(false);
}

void MT_2_N52::measure()
{
	QString homeLocation = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	QString filename = homeLocation + "/MTDataBIN_optimus";
	string file_str = filename.toStdString();
	const char* file_char = file_str.c_str();
	ofstream data_file(file_char, ios::app | ios::binary ); // Cambianos al ios::binary

	SetupDisplay();
	int pad = { 16 };
	int offset_x = -pad + 16 * round(min(M_1.x, R_1.x) / 16); int offset_y = -pad + 2 * round(min(M_1.y, R_1.y) / 2);
	int image_width = (pad + 16 * round(max(M_2.x, R_2.x) / 16)) - offset_x; int image_height = (pad + 2 * round(max(M_2.y, R_2.y) / 2)) - offset_y;
	cam.SetWidth(image_width); cam.SetHeight(image_height);
	cam.SetExposureTime(ui.cam_exposure->value()); cam.SetGain(10); //Era 10 antes
	//offx must be *16, offy must be *2 for this cam
	cam.SetOffsetX(offset_x); cam.SetOffsetY(offset_y);
	M_1S.x = M_1.x - offset_x; M_1S.y = M_1.y - offset_y; M_2S.x = M_1S.x + 128; M_2S.y = M_1S.y + 128;
	R_1S.x = R_1.x - offset_x; R_1S.y = R_1.y - offset_y; R_2S.x = R_1S.x + 128; R_2S.y = R_1S.y + 128;

	ShowButtons(1, 1, 0, 1);
	ui.FramesPerSecondText->setVisible(true);
	ui.View_fast_slow->setVisible(true);

	Mat RawImg, Img, M_Img, R_Img, T_Img, M_FFT, M_FFT_c, R_FFT, R_FFT_c, hann;
	Rect M_ROI = { M_1S.x ,M_1S.y ,128,128 }; Rect R_ROI = { R_1S.x ,R_1S.y ,128,128 };
	createHanningWindow(hann, Size(128, 128), CV_32F);

	Mat S_Profiles = Mat::zeros(256, 140, CV_8UC3);
	Mat S_Correlation = Mat::zeros(256, 140, CV_8UC3);
	Mat S_Z = Mat::zeros(128, 128, CV_8UC3);


	float M_Profile[100], R_Profile[100], M_Result[128], R_Result[128];
	float  M_Sum, M_Max, M_Min, M_Max_Loc;
	float  R_Sum, R_Max, R_Min, R_Max_Loc;
	double S00, S10, S20, S30, S40, S01, S11, S21, M_a, M_b, M_c, R_a, R_b, R_c, M_Z_pos, R_Z_pos;
	int npt, ctr_M, ctr_R, t2, t1, t3, t4, set_point, c_ref_pos = 0;

	int read_samples = 2;
	float64 Mag_voltage[2];
	float64 HC;
	float64 MagnetHeadResistance = 2; // in Ohms

	int frame = 0; int skip = 80; int lock_focus_average = 1000, P_start, P_resting, frame_start;
	do
	{
		Img = cam.GetNextImageOcvMat();
		Img(M_ROI).copyTo(M_Img); Img(R_ROI).copyTo(R_Img);
		M_Img.convertTo(M_Img, CV_32F); R_Img.convertTo(R_Img, CV_32F);
		M_Img = M_Img - mean(M_Img); multiply(M_Img, hann, M_Img); M_FFT = FFT_MT(M_Img);
		R_Img = R_Img - mean(R_Img); multiply(R_Img, hann, R_Img); R_FFT = FFT_MT(R_Img);
		RadialVector(M_FFT, M_Profile); RadialVector(R_FFT, R_Profile);

		M_Sum = 0; M_Max = 0; M_Min = 0;
		for (int i = 0; i < 100; ++i) { M_Result[i] = 0; M_Sum += M_Profile[i]; }
		for (int i = 0; i < 128; ++i) for (int j = 0; j < 100; ++j) { M_Result[i] += ((M_Stack[i][j]) * (M_Profile[j] - (M_Sum / 100))); }
		for (int i = 0; i < 128; ++i) { if (M_Result[i] < M_Min) M_Min = M_Result[i]; } for (int i = 0; i < 128; ++i) { M_Result[i] -= M_Min; }
		for (int i = 0; i < 128; ++i) { if (M_Result[i] > M_Max) { M_Max = M_Result[i]; M_Max_Loc = i; } } for (int i = 0; i < 128; ++i) { M_Result[i] = 1000 * (M_Result[i] / M_Max) - 800; }

		R_Sum = 0; R_Max = 0; R_Min = 0;
		for (int i = 0; i < 100; ++i) { R_Result[i] = 0; R_Sum += R_Profile[i]; }
		for (int i = 0; i < 128; ++i) for (int j = 0; j < 100; ++j) { R_Result[i] += ((R_Stack[i][j]) * (R_Profile[j] - (R_Sum / 100))); }
		for (int i = 0; i < 128; ++i) { if (R_Result[i] < R_Min) R_Min = R_Result[i]; } for (int i = 0; i < 128; ++i) { R_Result[i] -= R_Min; }
		for (int i = 0; i < 128; ++i) { if (R_Result[i] > R_Max) { R_Max = R_Result[i]; R_Max_Loc = i; } } for (int i = 0; i < 128; ++i) { R_Result[i] = 1000 * (R_Result[i] / R_Max) - 800; }

		npt = 10;

		ctr_M = M_Max_Loc; S00 = 2 * npt; S10 = 0; S20 = 0; S30 = 0; S40 = 0; S01 = 0; S11 = 0; S21 = 0; M_a = 0; M_b = 0; M_c = 0;
		for (int i = ctr_M - npt; i < ctr_M + npt; i++) { S10 = S10 + i; S20 = S20 + i*i; S30 = S30 + i*i*i; S40 = S40 + i*i*i*i; S01 = S01 + M_Result[i]; S11 = S11 + i*M_Result[i]; S21 = S21 + i*i*M_Result[i]; }
		M_a = (S01*S10*S30 - S11*S00*S30 - S01*(S20 *S20) + S11*S10*S20 + S21*S00*S20 - S21*(S10*S10)) / (S00*S20*S40 - (S10*S10)*S40 - S00*(S30*S30) + 2 * S10*S20*S30 - (S20*S20*S20));
		M_b = (S11*S00*S40 - S01*S10*S40 + S01*S20*S30 - S21*S00*S30 - S11*S20*S20 + S21*S10*S20) / (S00*S20*S40 - (S10*S10)*S40 - S00*(S30*S30) + 2 * S10*S20*S30 - (S20*S20*S20));
		M_c = (S01*S20*S40 - S11*S10*S40 - S01*(S30 *S30) + S11*S20*S30 + S21*S10*S30 - S21*(S20*S20)) / (S00*S20*S40 - (S10*S10)*S40 - S00*(S30*S30) + 2 * S10*S20*S30 - (S20*S20*S20));

		ctr_R = R_Max_Loc; S00 = 2 * npt; S10 = 0; S20 = 0; S30 = 0; S40 = 0; S01 = 0; S11 = 0; S21 = 0; R_a = 0; R_b = 0; R_c = 0;
		for (int i = ctr_R - npt; i < ctr_R + npt; i++) { S10 = S10 + i; S20 = S20 + i*i; S30 = S30 + i*i*i; S40 = S40 + i*i*i*i; S01 = S01 + R_Result[i]; S11 = S11 + i*R_Result[i]; S21 = S21 + i*i*R_Result[i]; }
		R_a = (S01*S10*S30 - S11*S00*S30 - S01*(S20 *S20) + S11*S10*S20 + S21*S00*S20 - S21*(S10*S10)) / (S00*S20*S40 - (S10*S10)*S40 - S00*(S30*S30) + 2 * S10*S20*S30 - (S20*S20*S20));
		R_b = (S11*S00*S40 - S01*S10*S40 + S01*S20*S30 - S21*S00*S30 - S11*S20*S20 + S21*S10*S20) / (S00*S20*S40 - (S10*S10)*S40 - S00*(S30*S30) + 2 * S10*S20*S30 - (S20*S20*S20));
		R_c = (S01*S20*S40 - S11*S10*S40 - S01*(S30 *S30) + S11*S20*S30 + S21*S10*S30 - S21*(S20*S20)) / (S00*S20*S40 - (S10*S10)*S40 - S00*(S30*S30) + 2 * S10*S20*S30 - (S20*S20*S20));

		M_Z_pos = -M_b / (2 * M_a); R_Z_pos = -R_b / (2 * R_a);

		/////////////////reference bead lock_focus ////////////////////////////
		c_ref_pos += R_Z_pos;
		if (ui.lockReference->isChecked() == false) { lockFocus = R_Z_pos; }
		if (ui.lockReference->isChecked() == true && frame % lock_focus_average == 0)
		{
			c_ref_pos /= lock_focus_average;
			if (c_ref_pos < lockFocus) { Focus_box(ui.FocusBox->value() + 50); }
			if (c_ref_pos > lockFocus) { Focus_box(ui.FocusBox->value() - 50); }
			c_ref_pos = 0;
		}

		//////////////////////// Get Magnetic Head Current /////////////////////////

		float MagnetPosition = 4 + 2 * AI_average;

		////////////////// Save data z, HC, t /////////////////////////////////
		float	z = 20.0*(R_Z_pos - M_Z_pos)*0.8; //scale corrected for refractive index of water/oil
		qInfo() << z;

		now = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count() - t0;
		if (frame % 1000 == 0) { t1 = t2; t2 = now; int fps = 1e9 / (t2 - t1); QString s = QString::number(fps); ui.FramesPerSecondText->setText(s); }
		if (ui.SaveDataButton->isChecked() == true) { ui.SaveDataButton->setText("Save on"); }
		else { ui.SaveDataButton->setText("Save off"); }
		float time = now;
		//float force = 65*exp(0.9*(0.99- MagnetPosition));// convert from measured MP to force using JACS 2016
		float force = Magnet_Law(MagnetPosition, ui.BeadType->isChecked(), "MP_F");
		if (ui.SaveDataButton->isChecked() == true) { /*data_file << z << " " << MagnetPosition << " " << time << endl;*/ data_file.write(reinterpret_cast<char*>(&z), sizeof(z)); data_file.write(reinterpret_cast<char*>(&MagnetPosition), sizeof(MagnetPosition)); data_file.write(reinterpret_cast<char*>(&time), sizeof(time)); }

		///////////////display images ////////////////////////////////////////
		if (ui.View_fast_slow->isChecked() == false) { skip = 80; }
		else { skip = 2000; }
		if (frame%skip == 0)
		{
			Img.convertTo(Img, CV_32F); normalize(Img, Img, 0, 1, NORM_MINMAX); cvtColor(Img, Img, COLOR_GRAY2BGR);
			rectangle(Img, M_1S, M_2S, Scalar(1, 0.5, 0), 2, 8, 0); rectangle(Img, R_1S, R_2S, Scalar(0, 1, 0), 2, 8, 0);
			imshow("pairs", Img); moveWindow("pairs", 40, 600);

			S_Profiles = Scalar::all(230);
			for (int j = 0; j < 99; j += 1)
			{
				line(S_Profiles, Point(j, 0.01*(int)M_Profile[j]), Point(j + 1, 0.01*(int)M_Profile[j + 1]), Scalar(255, 0, 0), 1, LINE_AA);
				line(S_Profiles, Point(j, 0.01*(int)R_Profile[j]), Point(j + 1, 0.01*(int)R_Profile[j + 1]), Scalar(0, 255, 0), 1, LINE_AA);
			}
			flip(S_Profiles, S_Profiles, 0);
			imshow("Radial Vectors", S_Profiles);

			S_Correlation = Scalar::all(230);
			for (int i = ctr_M - npt; i < ctr_M + npt; i++)
			{
				line(S_Correlation, Point(i, (int)(M_a*i*i + M_b*i + M_c)), Point(i + 1, (int)(M_a*(i + 1)*(i + 1) + M_b*(i + 1) + M_c)), Scalar(0, 0, 255), 1, LINE_AA);
			}
			for (int i = ctr_R - npt; i < ctr_R + npt; i++)
			{
				line(S_Correlation, Point(i, (int)(R_a*i*i + R_b*i + R_c)), Point(i + 1, (int)(R_a*(i + 1)*(i + 1) + R_b*(i + 1) + R_c)), Scalar(0, 0, 255), 1, LINE_AA);
			}

			for (int j = 1; j < 110; j += 1)
			{
				line(S_Correlation, Point(j - 1, (int)M_Result[j]), Point(j + 1, (int)M_Result[j]), Scalar(255, 100, 0), 1, LINE_8);
				line(S_Correlation, Point(j, (int)M_Result[j] - 1), Point(j, (int)M_Result[j] + 1), Scalar(255, 100, 0), 1, LINE_8);
				line(S_Correlation, Point(j - 1, (int)R_Result[j]), Point(j + 1, (int)R_Result[j]), Scalar(100, 255, 0), 1, LINE_8);
				line(S_Correlation, Point(j, (int)R_Result[j] - 1), Point(j, (int)R_Result[j] + 1), Scalar(100, 255, 0), 1, LINE_8);
			}
			flip(S_Correlation, S_Correlation, 0);
			imshow("Correlation", S_Correlation);
			waitKey(1);
		}

		//////////////////////////////////////////////////////////////////////
		frame++;
	} while (ui.MeasureButton->isChecked() == true);
	ShowButtons(1, 0, 1, 0);
	//ui.expLabel->setVisible(false); //Activar en un futuro
	ui.FramesPerSecondText->setVisible(false);
	ui.View_fast_slow->setVisible(false);
	destroyAllWindows();
	return;
}

void MT_2_N52::get_stack()
{
	ui.stackButton->setEnabled(false);
	ui.lockReference->setChecked(false);
	Mat Img, M_Img, R_Img, M_FFT, R_FFT, hann;
	Rect M_ROI = { M_1S.x ,M_1S.y ,128,128 }; Rect R_ROI = { R_1S.x ,R_1S.y ,128,128 };
	int stepsize = 20;
	createHanningWindow(hann, Size(128, 128), CV_32F);

	for (int i = 0; i < 128; ++i) for (int j = 0; j < 100; ++j) { M_Stack[i][j] = 0; }
	for (int i = 0; i < 128; ++i) for (int j = 0; j < 100; ++j) { R_Stack[i][j] = 0; }
	float M_Profile[100], R_Profile[100];
	float M_mean, M_StD, M_Sum, M_SqSum, M_Max, M_Min = 0;
	float R_mean, R_StD, R_Sum, R_SqSum, R_Max, R_Min = 0;

	for (int i = 0; i < 128; i++)
	{
		currentFocus += stepsize;
		Focus_box(currentFocus);
		waitKey(50);
		for (int j = 0; j < 32; j++)
		{
			Img = cam.GetNextImageOcvMat(); Img.convertTo(Img, CV_32F);
			M_Img = Img(M_ROI).clone(); R_Img = Img(R_ROI).clone();
			M_Img = M_Img - mean(M_Img); multiply(M_Img, hann, M_Img); M_FFT = FFT_MT(M_Img);
			R_Img = R_Img - mean(R_Img); multiply(R_Img, hann, R_Img); R_FFT = FFT_MT(R_Img);
			RadialVector(M_FFT, M_Profile); RadialVector(R_FFT, R_Profile);

			M_Sum = 0; M_SqSum = 0; M_StD = 0;
			for (int i = 0; i < 100; ++i) { M_Sum += M_Profile[i]; M_SqSum += pow(M_Profile[i], 2); } M_StD = pow(M_SqSum / 100 - pow(M_Sum / 100, 2), 0.5);
			for (int k = 0; k < 100; k++) M_Stack[i][k] += ((M_Profile[k] - M_Sum / 100) / M_StD);

			R_Sum = 0; R_SqSum = 0; R_StD = 0;
			for (int i = 0; i < 100; ++i) { R_Sum += R_Profile[i]; R_SqSum += pow(R_Profile[i], 2); } R_StD = pow(R_SqSum / 100 - pow(R_Sum / 100, 2), 0.5);
			for (int k = 0; k < 100; k++) R_Stack[i][k] += ((R_Profile[k] - R_Sum / 100) / R_StD);
		}
	}
	currentFocus -= 2560;// total stack excursion 128*stepsize 
	Focus_box(currentFocus);
	ui.stackButton->setEnabled(true);
	return;
}

void SetupDisplay()
{
	namedWindow("Radial Vectors", WINDOW_NORMAL);
	resizeWindow("Radial Vectors", 256, 256);
	moveWindow("Radial Vectors", 40, 310);

	namedWindow("Correlation", WINDOW_NORMAL);
	resizeWindow("Correlation", 256, 256);
	moveWindow("Correlation", 300, 310);

}

Mat FFT_MT(Mat Img)
{
	dft(Img, Img);
	Mat magImg = { Mat::zeros(128,64, CV_32F) };
	for (int i = 0; i < 32; i++) { for (int j = 1; j < 31; j++)magImg.at<float>(i + 64, j) = sqrt(pow(Img.at<float>(i, 2 * j - 1), 2) + pow(Img.at<float>(i, 2 * j), 2)); }
	for (int i = 96; i < 128; i++) { for (int j = 1; j < 31; j++)magImg.at<float>(i - 64, j) = sqrt(pow(Img.at<float>(i, 2 * j - 1), 2) + pow(Img.at<float>(i, 2 * j), 2)); }
	return (Mat(magImg));
}

float RadialVector(Mat Img, float *Profile)
{
	Profile[0] = (Img.at<float>(54, 0) + Img.at<float>(74, 0) + Img.at<float>(54, 1) + Img.at<float>(74, 1) + Img.at<float>(56, 6) + Img.at<float>(72, 6) + Img.at<float>(57, 7) + Img.at<float>(71, 7) + Img.at<float>(58, 8) + Img.at<float>(70, 8) + Img.at<float>(63, 10) + Img.at<float>(64, 10) + Img.at<float>(65, 10) + 0) / 13;
	Profile[1] = (Img.at<float>(54, 2) + Img.at<float>(74, 2) + Img.at<float>(55, 5) + Img.at<float>(73, 5) + Img.at<float>(59, 9) + Img.at<float>(69, 9) + Img.at<float>(62, 10) + Img.at<float>(66, 10) + 0) / 8;
	Profile[2] = (Img.at<float>(54, 3) + Img.at<float>(74, 3) + Img.at<float>(61, 10) + Img.at<float>(67, 10) + 0) / 4;
	Profile[3] = (Img.at<float>(54, 4) + Img.at<float>(74, 4) + Img.at<float>(55, 6) + Img.at<float>(73, 6) + Img.at<float>(56, 7) + Img.at<float>(72, 7) + Img.at<float>(57, 8) + Img.at<float>(71, 8) + Img.at<float>(58, 9) + Img.at<float>(70, 9) + Img.at<float>(60, 10) + Img.at<float>(68, 10) + 0) / 12;
	Profile[4] = (Img.at<float>(53, 0) + Img.at<float>(75, 0) + Img.at<float>(53, 1) + Img.at<float>(75, 1) + Img.at<float>(63, 11) + Img.at<float>(64, 11) + Img.at<float>(65, 11) + 0) / 7;
	Profile[5] = (Img.at<float>(53, 2) + Img.at<float>(75, 2) + Img.at<float>(54, 5) + Img.at<float>(74, 5) + Img.at<float>(56, 8) + Img.at<float>(72, 8) + Img.at<float>(59, 10) + Img.at<float>(69, 10) + Img.at<float>(62, 11) + Img.at<float>(66, 11) + 0) / 10;
	Profile[6] = (Img.at<float>(53, 3) + Img.at<float>(75, 3) + Img.at<float>(55, 7) + Img.at<float>(73, 7) + Img.at<float>(57, 9) + Img.at<float>(71, 9) + Img.at<float>(61, 11) + Img.at<float>(67, 11) + 0) / 8;
	Profile[7] = (Img.at<float>(53, 4) + Img.at<float>(75, 4) + Img.at<float>(54, 6) + Img.at<float>(74, 6) + Img.at<float>(58, 10) + Img.at<float>(70, 10) + Img.at<float>(60, 11) + Img.at<float>(68, 11) + 0) / 8;
	Profile[8] = (Img.at<float>(52, 0) + Img.at<float>(76, 0) + Img.at<float>(52, 1) + Img.at<float>(76, 1) + Img.at<float>(53, 5) + Img.at<float>(75, 5) + Img.at<float>(55, 8) + Img.at<float>(73, 8) + Img.at<float>(56, 9) + Img.at<float>(72, 9) + Img.at<float>(59, 11) + Img.at<float>(69, 11) + Img.at<float>(63, 12) + Img.at<float>(64, 12) + Img.at<float>(65, 12) + 0) / 15;
	Profile[9] = (Img.at<float>(52, 2) + Img.at<float>(76, 2) + Img.at<float>(52, 3) + Img.at<float>(76, 3) + Img.at<float>(54, 7) + Img.at<float>(74, 7) + Img.at<float>(57, 10) + Img.at<float>(71, 10) + Img.at<float>(61, 12) + Img.at<float>(62, 12) + Img.at<float>(66, 12) + Img.at<float>(67, 12) + 0) / 12;
	Profile[10] = (Img.at<float>(53, 6) + Img.at<float>(75, 6) + Img.at<float>(58, 11) + Img.at<float>(70, 11) + 0) / 4;
	Profile[11] = (Img.at<float>(52, 4) + Img.at<float>(76, 4) + Img.at<float>(54, 8) + Img.at<float>(74, 8) + Img.at<float>(55, 9) + Img.at<float>(73, 9) + Img.at<float>(56, 10) + Img.at<float>(72, 10) + Img.at<float>(60, 12) + Img.at<float>(68, 12) + 0) / 10;
	Profile[12] = (Img.at<float>(51, 0) + Img.at<float>(77, 0) + Img.at<float>(51, 1) + Img.at<float>(77, 1) + Img.at<float>(52, 5) + Img.at<float>(76, 5) + Img.at<float>(53, 7) + Img.at<float>(75, 7) + Img.at<float>(57, 11) + Img.at<float>(71, 11) + Img.at<float>(59, 12) + Img.at<float>(69, 12) + Img.at<float>(63, 13) + Img.at<float>(64, 13) + Img.at<float>(65, 13) + 0) / 15;
	Profile[13] = (Img.at<float>(51, 2) + Img.at<float>(77, 2) + Img.at<float>(51, 3) + Img.at<float>(77, 3) + Img.at<float>(61, 13) + Img.at<float>(62, 13) + Img.at<float>(66, 13) + Img.at<float>(67, 13) + 0) / 8;
	Profile[14] = (Img.at<float>(51, 4) + Img.at<float>(77, 4) + Img.at<float>(52, 6) + Img.at<float>(76, 6) + Img.at<float>(53, 8) + Img.at<float>(75, 8) + Img.at<float>(54, 9) + Img.at<float>(74, 9) + Img.at<float>(55, 10) + Img.at<float>(73, 10) + Img.at<float>(56, 11) + Img.at<float>(72, 11) + Img.at<float>(58, 12) + Img.at<float>(70, 12) + Img.at<float>(60, 13) + Img.at<float>(68, 13) + 0) / 16;
	Profile[15] = Profile[16];
	Profile[16] = (Img.at<float>(50, 0) + Img.at<float>(78, 0) + Img.at<float>(50, 1) + Img.at<float>(78, 1) + Img.at<float>(51, 5) + Img.at<float>(77, 5) + Img.at<float>(52, 7) + Img.at<float>(76, 7) + Img.at<float>(57, 12) + Img.at<float>(71, 12) + Img.at<float>(59, 13) + Img.at<float>(69, 13) + Img.at<float>(63, 14) + Img.at<float>(64, 14) + Img.at<float>(65, 14) + 0) / 15;
	Profile[17] = (Img.at<float>(50, 2) + Img.at<float>(78, 2) + Img.at<float>(50, 3) + Img.at<float>(78, 3) + Img.at<float>(51, 6) + Img.at<float>(77, 6) + Img.at<float>(53, 9) + Img.at<float>(75, 9) + Img.at<float>(54, 10) + Img.at<float>(74, 10) + Img.at<float>(55, 11) + Img.at<float>(73, 11) + Img.at<float>(58, 13) + Img.at<float>(70, 13) + Img.at<float>(61, 14) + Img.at<float>(62, 14) + Img.at<float>(66, 14) + Img.at<float>(67, 14) + 0) / 18;
	Profile[18] = (Img.at<float>(50, 4) + Img.at<float>(78, 4) + Img.at<float>(52, 8) + Img.at<float>(76, 8) + Img.at<float>(56, 12) + Img.at<float>(72, 12) + Img.at<float>(60, 14) + Img.at<float>(68, 14) + 0) / 8;
	Profile[19] = (Img.at<float>(50, 5) + Img.at<float>(78, 5) + Img.at<float>(51, 7) + Img.at<float>(77, 7) + Img.at<float>(53, 10) + Img.at<float>(75, 10) + Img.at<float>(54, 11) + Img.at<float>(74, 11) + Img.at<float>(57, 13) + Img.at<float>(71, 13) + Img.at<float>(59, 14) + Img.at<float>(69, 14) + 0) / 12;
	Profile[20] = (Img.at<float>(49, 0) + Img.at<float>(79, 0) + Img.at<float>(49, 1) + Img.at<float>(79, 1) + Img.at<float>(52, 9) + Img.at<float>(76, 9) + Img.at<float>(55, 12) + Img.at<float>(73, 12) + Img.at<float>(63, 15) + Img.at<float>(64, 15) + Img.at<float>(65, 15) + 0) / 11;
	Profile[21] = (Img.at<float>(49, 2) + Img.at<float>(79, 2) + Img.at<float>(49, 3) + Img.at<float>(79, 3) + Img.at<float>(50, 6) + Img.at<float>(78, 6) + Img.at<float>(51, 8) + Img.at<float>(77, 8) + Img.at<float>(56, 13) + Img.at<float>(72, 13) + Img.at<float>(58, 14) + Img.at<float>(70, 14) + Img.at<float>(61, 15) + Img.at<float>(62, 15) + Img.at<float>(66, 15) + Img.at<float>(67, 15) + 0) / 16;
	Profile[22] = (Img.at<float>(49, 4) + Img.at<float>(79, 4) + Img.at<float>(52, 10) + Img.at<float>(76, 10) + Img.at<float>(53, 11) + Img.at<float>(75, 11) + Img.at<float>(54, 12) + Img.at<float>(74, 12) + Img.at<float>(60, 15) + Img.at<float>(68, 15) + 0) / 10;
	Profile[23] = (Img.at<float>(49, 5) + Img.at<float>(79, 5) + Img.at<float>(50, 7) + Img.at<float>(78, 7) + Img.at<float>(51, 9) + Img.at<float>(77, 9) + Img.at<float>(55, 13) + Img.at<float>(73, 13) + Img.at<float>(57, 14) + Img.at<float>(71, 14) + Img.at<float>(59, 15) + Img.at<float>(69, 15) + 0) / 12;
	Profile[24] = (Img.at<float>(48, 0) + Img.at<float>(80, 0) + Img.at<float>(48, 1) + Img.at<float>(80, 1) + Img.at<float>(48, 2) + Img.at<float>(80, 2) + Img.at<float>(50, 8) + Img.at<float>(78, 8) + Img.at<float>(56, 14) + Img.at<float>(72, 14) + Img.at<float>(62, 16) + Img.at<float>(63, 16) + Img.at<float>(64, 16) + Img.at<float>(65, 16) + Img.at<float>(66, 16) + 0) / 15;
	Profile[25] = (Img.at<float>(48, 3) + Img.at<float>(80, 3) + Img.at<float>(49, 6) + Img.at<float>(79, 6) + Img.at<float>(52, 11) + Img.at<float>(76, 11) + Img.at<float>(53, 12) + Img.at<float>(75, 12) + Img.at<float>(58, 15) + Img.at<float>(70, 15) + Img.at<float>(61, 16) + Img.at<float>(67, 16) + 0) / 12;
	Profile[26] = (Img.at<float>(48, 4) + Img.at<float>(80, 4) + Img.at<float>(49, 7) + Img.at<float>(79, 7) + Img.at<float>(51, 10) + Img.at<float>(77, 10) + Img.at<float>(54, 13) + Img.at<float>(74, 13) + Img.at<float>(57, 15) + Img.at<float>(71, 15) + Img.at<float>(60, 16) + Img.at<float>(68, 16) + 0) / 12;
	Profile[27] = (Img.at<float>(48, 5) + Img.at<float>(80, 5) + Img.at<float>(50, 9) + Img.at<float>(78, 9) + Img.at<float>(55, 14) + Img.at<float>(73, 14) + Img.at<float>(59, 16) + Img.at<float>(69, 16) + 0) / 8;
	Profile[28] = (Img.at<float>(47, 0) + Img.at<float>(81, 0) + Img.at<float>(47, 1) + Img.at<float>(81, 1) + Img.at<float>(47, 2) + Img.at<float>(81, 2) + Img.at<float>(48, 6) + Img.at<float>(80, 6) + Img.at<float>(49, 8) + Img.at<float>(79, 8) + Img.at<float>(51, 11) + Img.at<float>(77, 11) + Img.at<float>(52, 12) + Img.at<float>(76, 12) + Img.at<float>(53, 13) + Img.at<float>(75, 13) + Img.at<float>(56, 15) + Img.at<float>(72, 15) + Img.at<float>(58, 16) + Img.at<float>(70, 16) + Img.at<float>(62, 17) + Img.at<float>(63, 17) + Img.at<float>(64, 17) + Img.at<float>(65, 17) + Img.at<float>(66, 17) + 0) / 25;
	Profile[29] = (Img.at<float>(47, 3) + Img.at<float>(81, 3) + Img.at<float>(50, 10) + Img.at<float>(78, 10) + Img.at<float>(54, 14) + Img.at<float>(74, 14) + Img.at<float>(61, 17) + Img.at<float>(67, 17) + 0) / 8;
	Profile[30] = (Img.at<float>(47, 4) + Img.at<float>(81, 4) + Img.at<float>(48, 7) + Img.at<float>(80, 7) + Img.at<float>(49, 9) + Img.at<float>(79, 9) + Img.at<float>(55, 15) + Img.at<float>(73, 15) + Img.at<float>(57, 16) + Img.at<float>(71, 16) + Img.at<float>(60, 17) + Img.at<float>(68, 17) + 0) / 12;
	Profile[31] = (Img.at<float>(47, 5) + Img.at<float>(81, 5) + Img.at<float>(50, 11) + Img.at<float>(78, 11) + Img.at<float>(51, 12) + Img.at<float>(77, 12) + Img.at<float>(52, 13) + Img.at<float>(76, 13) + Img.at<float>(53, 14) + Img.at<float>(75, 14) + Img.at<float>(59, 17) + Img.at<float>(69, 17) + 0) / 12;
	Profile[32] = (Img.at<float>(46, 0) + Img.at<float>(82, 0) + Img.at<float>(46, 1) + Img.at<float>(82, 1) + Img.at<float>(46, 2) + Img.at<float>(82, 2) + Img.at<float>(47, 6) + Img.at<float>(81, 6) + Img.at<float>(48, 8) + Img.at<float>(80, 8) + Img.at<float>(49, 10) + Img.at<float>(79, 10) + Img.at<float>(54, 15) + Img.at<float>(74, 15) + Img.at<float>(56, 16) + Img.at<float>(72, 16) + Img.at<float>(58, 17) + Img.at<float>(70, 17) + Img.at<float>(62, 18) + Img.at<float>(63, 18) + Img.at<float>(64, 18) + Img.at<float>(65, 18) + Img.at<float>(66, 18) + 0) / 23;
	Profile[33] = (Img.at<float>(46, 3) + Img.at<float>(82, 3) + Img.at<float>(48, 9) + Img.at<float>(80, 9) + Img.at<float>(55, 16) + Img.at<float>(73, 16) + Img.at<float>(61, 18) + Img.at<float>(67, 18) + 0) / 8;
	Profile[34] = (Img.at<float>(46, 4) + Img.at<float>(82, 4) + Img.at<float>(47, 7) + Img.at<float>(81, 7) + Img.at<float>(49, 11) + Img.at<float>(79, 11) + Img.at<float>(50, 12) + Img.at<float>(78, 12) + Img.at<float>(51, 13) + Img.at<float>(77, 13) + Img.at<float>(52, 14) + Img.at<float>(76, 14) + Img.at<float>(53, 15) + Img.at<float>(75, 15) + Img.at<float>(57, 17) + Img.at<float>(71, 17) + Img.at<float>(60, 18) + Img.at<float>(68, 18) + 0) / 18;
	Profile[35] = (Img.at<float>(46, 5) + Img.at<float>(82, 5) + Img.at<float>(47, 8) + Img.at<float>(81, 8) + Img.at<float>(48, 10) + Img.at<float>(80, 10) + Img.at<float>(54, 16) + Img.at<float>(74, 16) + Img.at<float>(56, 17) + Img.at<float>(72, 17) + Img.at<float>(59, 18) + Img.at<float>(69, 18) + 0) / 12;
	Profile[36] = (Img.at<float>(45, 0) + Img.at<float>(83, 0) + Img.at<float>(45, 1) + Img.at<float>(83, 1) + Img.at<float>(45, 2) + Img.at<float>(83, 2) + Img.at<float>(46, 6) + Img.at<float>(82, 6) + Img.at<float>(50, 13) + Img.at<float>(78, 13) + Img.at<float>(51, 14) + Img.at<float>(77, 14) + Img.at<float>(58, 18) + Img.at<float>(70, 18) + Img.at<float>(62, 19) + Img.at<float>(63, 19) + Img.at<float>(64, 19) + Img.at<float>(65, 19) + Img.at<float>(66, 19) + 0) / 19;
	Profile[37] = (Img.at<float>(45, 3) + Img.at<float>(83, 3) + Img.at<float>(46, 7) + Img.at<float>(82, 7) + Img.at<float>(47, 9) + Img.at<float>(81, 9) + Img.at<float>(49, 12) + Img.at<float>(79, 12) + Img.at<float>(52, 15) + Img.at<float>(76, 15) + Img.at<float>(55, 17) + Img.at<float>(73, 17) + Img.at<float>(57, 18) + Img.at<float>(71, 18) + Img.at<float>(61, 19) + Img.at<float>(67, 19) + 0) / 16;
	Profile[38] = (Img.at<float>(45, 4) + Img.at<float>(83, 4) + Img.at<float>(48, 11) + Img.at<float>(80, 11) + Img.at<float>(53, 16) + Img.at<float>(75, 16) + Img.at<float>(60, 19) + Img.at<float>(68, 19) + 0) / 8;
	Profile[39] = (Img.at<float>(45, 5) + Img.at<float>(83, 5) + Img.at<float>(46, 8) + Img.at<float>(82, 8) + Img.at<float>(47, 10) + Img.at<float>(81, 10) + Img.at<float>(49, 13) + Img.at<float>(79, 13) + Img.at<float>(50, 14) + Img.at<float>(78, 14) + Img.at<float>(51, 15) + Img.at<float>(77, 15) + Img.at<float>(54, 17) + Img.at<float>(74, 17) + Img.at<float>(56, 18) + Img.at<float>(72, 18) + Img.at<float>(59, 19) + Img.at<float>(69, 19) + 0) / 18;
	Profile[40] = (Img.at<float>(44, 0) + Img.at<float>(84, 0) + Img.at<float>(44, 1) + Img.at<float>(84, 1) + Img.at<float>(44, 2) + Img.at<float>(84, 2) + Img.at<float>(45, 6) + Img.at<float>(83, 6) + Img.at<float>(46, 9) + Img.at<float>(82, 9) + Img.at<float>(48, 12) + Img.at<float>(80, 12) + Img.at<float>(52, 16) + Img.at<float>(76, 16) + Img.at<float>(55, 18) + Img.at<float>(73, 18) + Img.at<float>(58, 19) + Img.at<float>(70, 19) + Img.at<float>(62, 20) + Img.at<float>(63, 20) + Img.at<float>(64, 20) + Img.at<float>(65, 20) + Img.at<float>(66, 20) + 0) / 23;
	Profile[41] = (Img.at<float>(44, 3) + Img.at<float>(84, 3) + Img.at<float>(45, 7) + Img.at<float>(83, 7) + Img.at<float>(47, 11) + Img.at<float>(81, 11) + Img.at<float>(53, 17) + Img.at<float>(75, 17) + Img.at<float>(57, 19) + Img.at<float>(71, 19) + Img.at<float>(61, 20) + Img.at<float>(67, 20) + 0) / 12;
	Profile[42] = (Img.at<float>(44, 4) + Img.at<float>(84, 4) + Img.at<float>(44, 5) + Img.at<float>(84, 5) + Img.at<float>(45, 8) + Img.at<float>(83, 8) + Img.at<float>(46, 10) + Img.at<float>(82, 10) + Img.at<float>(48, 13) + Img.at<float>(80, 13) + Img.at<float>(49, 14) + Img.at<float>(79, 14) + Img.at<float>(50, 15) + Img.at<float>(78, 15) + Img.at<float>(51, 16) + Img.at<float>(77, 16) + Img.at<float>(54, 18) + Img.at<float>(74, 18) + Img.at<float>(56, 19) + Img.at<float>(72, 19) + Img.at<float>(59, 20) + Img.at<float>(60, 20) + Img.at<float>(68, 20) + Img.at<float>(69, 20) + 0) / 24;
	Profile[43] = (Img.at<float>(47, 12) + Img.at<float>(81, 12) + Img.at<float>(52, 17) + Img.at<float>(76, 17) + 0) / 4;
	Profile[44] = (Img.at<float>(43, 0) + Img.at<float>(85, 0) + Img.at<float>(43, 1) + Img.at<float>(85, 1) + Img.at<float>(43, 2) + Img.at<float>(85, 2) + Img.at<float>(44, 6) + Img.at<float>(84, 6) + Img.at<float>(45, 9) + Img.at<float>(83, 9) + Img.at<float>(46, 11) + Img.at<float>(82, 11) + Img.at<float>(53, 18) + Img.at<float>(75, 18) + Img.at<float>(55, 19) + Img.at<float>(73, 19) + Img.at<float>(58, 20) + Img.at<float>(70, 20) + Img.at<float>(62, 21) + Img.at<float>(63, 21) + Img.at<float>(64, 21) + Img.at<float>(65, 21) + Img.at<float>(66, 21) + 0) / 23;
	Profile[45] = (Img.at<float>(43, 3) + Img.at<float>(85, 3) + Img.at<float>(44, 7) + Img.at<float>(84, 7) + Img.at<float>(48, 14) + Img.at<float>(80, 14) + Img.at<float>(49, 15) + Img.at<float>(79, 15) + Img.at<float>(50, 16) + Img.at<float>(78, 16) + Img.at<float>(57, 20) + Img.at<float>(71, 20) + Img.at<float>(61, 21) + Img.at<float>(67, 21) + 0) / 14;
	Profile[46] = (Img.at<float>(43, 4) + Img.at<float>(85, 4) + Img.at<float>(43, 5) + Img.at<float>(85, 5) + Img.at<float>(44, 8) + Img.at<float>(84, 8) + Img.at<float>(45, 10) + Img.at<float>(83, 10) + Img.at<float>(47, 13) + Img.at<float>(81, 13) + Img.at<float>(51, 17) + Img.at<float>(77, 17) + Img.at<float>(54, 19) + Img.at<float>(74, 19) + Img.at<float>(56, 20) + Img.at<float>(72, 20) + Img.at<float>(59, 21) + Img.at<float>(60, 21) + Img.at<float>(68, 21) + Img.at<float>(69, 21) + 0) / 20;
	Profile[47] = (Img.at<float>(43, 6) + Img.at<float>(85, 6) + Img.at<float>(46, 12) + Img.at<float>(82, 12) + Img.at<float>(52, 18) + Img.at<float>(76, 18) + Img.at<float>(58, 21) + Img.at<float>(70, 21) + 0) / 8;
	Profile[48] = (Img.at<float>(42, 0) + Img.at<float>(86, 0) + Img.at<float>(42, 1) + Img.at<float>(86, 1) + Img.at<float>(42, 2) + Img.at<float>(86, 2) + Img.at<float>(44, 9) + Img.at<float>(84, 9) + Img.at<float>(45, 11) + Img.at<float>(83, 11) + Img.at<float>(47, 14) + Img.at<float>(81, 14) + Img.at<float>(48, 15) + Img.at<float>(80, 15) + Img.at<float>(49, 16) + Img.at<float>(79, 16) + Img.at<float>(50, 17) + Img.at<float>(78, 17) + Img.at<float>(53, 19) + Img.at<float>(75, 19) + Img.at<float>(55, 20) + Img.at<float>(73, 20) + Img.at<float>(62, 22) + Img.at<float>(63, 22) + Img.at<float>(64, 22) + Img.at<float>(65, 22) + Img.at<float>(66, 22) + 0) / 27;
	Profile[49] = (Img.at<float>(42, 3) + Img.at<float>(86, 3) + Img.at<float>(42, 4) + Img.at<float>(86, 4) + Img.at<float>(43, 7) + Img.at<float>(85, 7) + Img.at<float>(44, 10) + Img.at<float>(84, 10) + Img.at<float>(46, 13) + Img.at<float>(82, 13) + Img.at<float>(51, 18) + Img.at<float>(77, 18) + Img.at<float>(54, 20) + Img.at<float>(74, 20) + Img.at<float>(57, 21) + Img.at<float>(71, 21) + Img.at<float>(60, 22) + Img.at<float>(61, 22) + Img.at<float>(67, 22) + Img.at<float>(68, 22) + 0) / 20;
	Profile[50] = (Img.at<float>(42, 5) + Img.at<float>(86, 5) + Img.at<float>(43, 8) + Img.at<float>(85, 8) + Img.at<float>(45, 12) + Img.at<float>(83, 12) + Img.at<float>(52, 19) + Img.at<float>(76, 19) + Img.at<float>(56, 21) + Img.at<float>(72, 21) + Img.at<float>(59, 22) + Img.at<float>(69, 22) + 0) / 12;
	Profile[51] = (Img.at<float>(42, 6) + Img.at<float>(86, 6) + Img.at<float>(43, 9) + Img.at<float>(85, 9) + Img.at<float>(44, 11) + Img.at<float>(84, 11) + Img.at<float>(46, 14) + Img.at<float>(82, 14) + Img.at<float>(47, 15) + Img.at<float>(81, 15) + Img.at<float>(48, 16) + Img.at<float>(80, 16) + Img.at<float>(49, 17) + Img.at<float>(79, 17) + Img.at<float>(50, 18) + Img.at<float>(78, 18) + Img.at<float>(53, 20) + Img.at<float>(75, 20) + Img.at<float>(55, 21) + Img.at<float>(73, 21) + Img.at<float>(58, 22) + Img.at<float>(70, 22) + 0) / 22;
	Profile[52] = (Img.at<float>(41, 0) + Img.at<float>(87, 0) + Img.at<float>(41, 1) + Img.at<float>(87, 1) + Img.at<float>(41, 2) + Img.at<float>(87, 2) + Img.at<float>(42, 7) + Img.at<float>(86, 7) + Img.at<float>(45, 13) + Img.at<float>(83, 13) + Img.at<float>(51, 19) + Img.at<float>(77, 19) + Img.at<float>(57, 22) + Img.at<float>(71, 22) + Img.at<float>(62, 23) + Img.at<float>(63, 23) + Img.at<float>(64, 23) + Img.at<float>(65, 23) + Img.at<float>(66, 23) + 0) / 19;
	Profile[53] = (Img.at<float>(41, 3) + Img.at<float>(87, 3) + Img.at<float>(41, 4) + Img.at<float>(87, 4) + Img.at<float>(43, 10) + Img.at<float>(85, 10) + Img.at<float>(44, 12) + Img.at<float>(84, 12) + Img.at<float>(47, 16) + Img.at<float>(81, 16) + Img.at<float>(48, 17) + Img.at<float>(80, 17) + Img.at<float>(52, 20) + Img.at<float>(76, 20) + Img.at<float>(54, 21) + Img.at<float>(74, 21) + Img.at<float>(60, 23) + Img.at<float>(61, 23) + Img.at<float>(67, 23) + Img.at<float>(68, 23) + 0) / 20;
	Profile[54] = (Img.at<float>(41, 5) + Img.at<float>(87, 5) + Img.at<float>(42, 8) + Img.at<float>(86, 8) + Img.at<float>(45, 14) + Img.at<float>(83, 14) + Img.at<float>(46, 15) + Img.at<float>(82, 15) + Img.at<float>(49, 18) + Img.at<float>(79, 18) + Img.at<float>(50, 19) + Img.at<float>(78, 19) + Img.at<float>(56, 22) + Img.at<float>(72, 22) + Img.at<float>(59, 23) + Img.at<float>(69, 23) + 0) / 16;
	Profile[55] = (Img.at<float>(41, 6) + Img.at<float>(87, 6) + Img.at<float>(42, 9) + Img.at<float>(86, 9) + Img.at<float>(43, 11) + Img.at<float>(85, 11) + Img.at<float>(44, 13) + Img.at<float>(84, 13) + Img.at<float>(51, 20) + Img.at<float>(77, 20) + Img.at<float>(53, 21) + Img.at<float>(75, 21) + Img.at<float>(55, 22) + Img.at<float>(73, 22) + Img.at<float>(58, 23) + Img.at<float>(70, 23) + 0) / 16;
	Profile[56] = (Img.at<float>(40, 0) + Img.at<float>(88, 0) + Img.at<float>(40, 1) + Img.at<float>(88, 1) + Img.at<float>(40, 2) + Img.at<float>(88, 2) + Img.at<float>(41, 7) + Img.at<float>(87, 7) + Img.at<float>(46, 16) + Img.at<float>(82, 16) + Img.at<float>(47, 17) + Img.at<float>(81, 17) + Img.at<float>(48, 18) + Img.at<float>(80, 18) + Img.at<float>(57, 23) + Img.at<float>(71, 23) + Img.at<float>(62, 24) + Img.at<float>(63, 24) + Img.at<float>(64, 24) + Img.at<float>(65, 24) + Img.at<float>(66, 24) + 0) / 21;
	Profile[57] = (Img.at<float>(40, 3) + Img.at<float>(88, 3) + Img.at<float>(40, 4) + Img.at<float>(88, 4) + Img.at<float>(41, 8) + Img.at<float>(87, 8) + Img.at<float>(42, 10) + Img.at<float>(86, 10) + Img.at<float>(43, 12) + Img.at<float>(85, 12) + Img.at<float>(45, 15) + Img.at<float>(83, 15) + Img.at<float>(49, 19) + Img.at<float>(79, 19) + Img.at<float>(52, 21) + Img.at<float>(76, 21) + Img.at<float>(54, 22) + Img.at<float>(74, 22) + Img.at<float>(56, 23) + Img.at<float>(72, 23) + Img.at<float>(60, 24) + Img.at<float>(61, 24) + Img.at<float>(67, 24) + Img.at<float>(68, 24) + 0) / 24;
	Profile[58] = (Img.at<float>(40, 5) + Img.at<float>(88, 5) + Img.at<float>(42, 11) + Img.at<float>(86, 11) + Img.at<float>(44, 14) + Img.at<float>(84, 14) + Img.at<float>(50, 20) + Img.at<float>(78, 20) + Img.at<float>(53, 22) + Img.at<float>(75, 22) + Img.at<float>(59, 24) + Img.at<float>(69, 24) + 0) / 12;
	Profile[59] = (Img.at<float>(40, 6) + Img.at<float>(88, 6) + Img.at<float>(41, 9) + Img.at<float>(87, 9) + Img.at<float>(43, 13) + Img.at<float>(85, 13) + Img.at<float>(45, 16) + Img.at<float>(83, 16) + Img.at<float>(46, 17) + Img.at<float>(82, 17) + Img.at<float>(47, 18) + Img.at<float>(81, 18) + Img.at<float>(48, 19) + Img.at<float>(80, 19) + Img.at<float>(51, 21) + Img.at<float>(77, 21) + Img.at<float>(55, 23) + Img.at<float>(73, 23) + Img.at<float>(58, 24) + Img.at<float>(70, 24) + 0) / 20;
	Profile[60] = (Img.at<float>(39, 0) + Img.at<float>(89, 0) + Img.at<float>(39, 1) + Img.at<float>(89, 1) + Img.at<float>(39, 2) + Img.at<float>(89, 2) + Img.at<float>(40, 7) + Img.at<float>(88, 7) + Img.at<float>(41, 10) + Img.at<float>(87, 10) + Img.at<float>(42, 12) + Img.at<float>(86, 12) + Img.at<float>(44, 15) + Img.at<float>(84, 15) + Img.at<float>(49, 20) + Img.at<float>(79, 20) + Img.at<float>(52, 22) + Img.at<float>(76, 22) + Img.at<float>(54, 23) + Img.at<float>(74, 23) + Img.at<float>(57, 24) + Img.at<float>(71, 24) + Img.at<float>(62, 25) + Img.at<float>(63, 25) + Img.at<float>(64, 25) + Img.at<float>(65, 25) + Img.at<float>(66, 25) + 0) / 27;
	Profile[61] = (Img.at<float>(39, 3) + Img.at<float>(89, 3) + Img.at<float>(39, 4) + Img.at<float>(89, 4) + Img.at<float>(40, 8) + Img.at<float>(88, 8) + Img.at<float>(43, 14) + Img.at<float>(85, 14) + Img.at<float>(50, 21) + Img.at<float>(78, 21) + Img.at<float>(56, 24) + Img.at<float>(72, 24) + Img.at<float>(60, 25) + Img.at<float>(61, 25) + Img.at<float>(67, 25) + Img.at<float>(68, 25) + 0) / 16;
	Profile[62] = (Img.at<float>(39, 5) + Img.at<float>(89, 5) + Img.at<float>(41, 11) + Img.at<float>(87, 11) + Img.at<float>(42, 13) + Img.at<float>(86, 13) + Img.at<float>(44, 16) + Img.at<float>(84, 16) + Img.at<float>(45, 17) + Img.at<float>(83, 17) + Img.at<float>(46, 18) + Img.at<float>(82, 18) + Img.at<float>(47, 19) + Img.at<float>(81, 19) + Img.at<float>(48, 20) + Img.at<float>(80, 20) + Img.at<float>(51, 22) + Img.at<float>(77, 22) + Img.at<float>(53, 23) + Img.at<float>(75, 23) + Img.at<float>(59, 25) + Img.at<float>(69, 25) + 0) / 22;
	Profile[63] = (Img.at<float>(39, 6) + Img.at<float>(89, 6) + Img.at<float>(40, 9) + Img.at<float>(88, 9) + Img.at<float>(43, 15) + Img.at<float>(85, 15) + Img.at<float>(49, 21) + Img.at<float>(79, 21) + Img.at<float>(55, 24) + Img.at<float>(73, 24) + Img.at<float>(58, 25) + Img.at<float>(70, 25) + 0) / 12;
	Profile[64] = (Img.at<float>(38, 0) + Img.at<float>(90, 0) + Img.at<float>(38, 1) + Img.at<float>(90, 1) + Img.at<float>(38, 2) + Img.at<float>(90, 2) + Img.at<float>(39, 7) + Img.at<float>(89, 7) + Img.at<float>(40, 10) + Img.at<float>(88, 10) + Img.at<float>(41, 12) + Img.at<float>(87, 12) + Img.at<float>(42, 14) + Img.at<float>(86, 14) + Img.at<float>(50, 22) + Img.at<float>(78, 22) + Img.at<float>(52, 23) + Img.at<float>(76, 23) + Img.at<float>(54, 24) + Img.at<float>(74, 24) + Img.at<float>(57, 25) + Img.at<float>(71, 25) + Img.at<float>(62, 26) + Img.at<float>(63, 26) + Img.at<float>(64, 26) + Img.at<float>(65, 26) + Img.at<float>(66, 26) + 0) / 27;
	Profile[65] = (Img.at<float>(38, 3) + Img.at<float>(90, 3) + Img.at<float>(38, 4) + Img.at<float>(90, 4) + Img.at<float>(39, 8) + Img.at<float>(89, 8) + Img.at<float>(44, 17) + Img.at<float>(84, 17) + Img.at<float>(45, 18) + Img.at<float>(83, 18) + Img.at<float>(46, 19) + Img.at<float>(82, 19) + Img.at<float>(47, 20) + Img.at<float>(81, 20) + Img.at<float>(56, 25) + Img.at<float>(72, 25) + Img.at<float>(60, 26) + Img.at<float>(61, 26) + Img.at<float>(67, 26) + Img.at<float>(68, 26) + 0) / 20;
	Profile[66] = (Img.at<float>(38, 5) + Img.at<float>(90, 5) + Img.at<float>(39, 9) + Img.at<float>(89, 9) + Img.at<float>(40, 11) + Img.at<float>(88, 11) + Img.at<float>(41, 13) + Img.at<float>(87, 13) + Img.at<float>(43, 16) + Img.at<float>(85, 16) + Img.at<float>(48, 21) + Img.at<float>(80, 21) + Img.at<float>(51, 23) + Img.at<float>(77, 23) + Img.at<float>(53, 24) + Img.at<float>(75, 24) + Img.at<float>(55, 25) + Img.at<float>(73, 25) + Img.at<float>(59, 26) + Img.at<float>(69, 26) + 0) / 20;
	Profile[67] = (Img.at<float>(38, 6) + Img.at<float>(90, 6) + Img.at<float>(40, 12) + Img.at<float>(88, 12) + Img.at<float>(42, 15) + Img.at<float>(86, 15) + Img.at<float>(45, 19) + Img.at<float>(83, 19) + Img.at<float>(49, 22) + Img.at<float>(79, 22) + Img.at<float>(52, 24) + Img.at<float>(76, 24) + Img.at<float>(58, 26) + Img.at<float>(70, 26) + 0) / 14;
	Profile[68] = (Img.at<float>(37, 0) + Img.at<float>(91, 0) + Img.at<float>(37, 1) + Img.at<float>(91, 1) + Img.at<float>(37, 2) + Img.at<float>(91, 2) + Img.at<float>(38, 7) + Img.at<float>(90, 7) + Img.at<float>(39, 10) + Img.at<float>(89, 10) + Img.at<float>(41, 14) + Img.at<float>(87, 14) + Img.at<float>(43, 17) + Img.at<float>(85, 17) + Img.at<float>(44, 18) + Img.at<float>(84, 18) + Img.at<float>(46, 20) + Img.at<float>(82, 20) + Img.at<float>(47, 21) + Img.at<float>(81, 21) + Img.at<float>(50, 23) + Img.at<float>(78, 23) + Img.at<float>(54, 25) + Img.at<float>(74, 25) + Img.at<float>(57, 26) + Img.at<float>(71, 26) + Img.at<float>(62, 27) + Img.at<float>(63, 27) + Img.at<float>(64, 27) + Img.at<float>(65, 27) + Img.at<float>(66, 27) + 0) / 31;
	Profile[69] = (Img.at<float>(37, 3) + Img.at<float>(91, 3) + Img.at<float>(37, 4) + Img.at<float>(91, 4) + Img.at<float>(38, 8) + Img.at<float>(90, 8) + Img.at<float>(39, 11) + Img.at<float>(89, 11) + Img.at<float>(40, 13) + Img.at<float>(88, 13) + Img.at<float>(42, 16) + Img.at<float>(86, 16) + Img.at<float>(48, 22) + Img.at<float>(80, 22) + Img.at<float>(51, 24) + Img.at<float>(77, 24) + Img.at<float>(53, 25) + Img.at<float>(75, 25) + Img.at<float>(56, 26) + Img.at<float>(72, 26) + Img.at<float>(60, 27) + Img.at<float>(61, 27) + Img.at<float>(67, 27) + Img.at<float>(68, 27) + 0) / 24;
	Profile[70] = (Img.at<float>(37, 5) + Img.at<float>(91, 5) + Img.at<float>(38, 9) + Img.at<float>(90, 9) + Img.at<float>(41, 15) + Img.at<float>(87, 15) + Img.at<float>(44, 19) + Img.at<float>(84, 19) + Img.at<float>(45, 20) + Img.at<float>(83, 20) + Img.at<float>(49, 23) + Img.at<float>(79, 23) + Img.at<float>(55, 26) + Img.at<float>(73, 26) + Img.at<float>(59, 27) + Img.at<float>(69, 27) + 0) / 16;
	Profile[71] = (Img.at<float>(37, 6) + Img.at<float>(91, 6) + Img.at<float>(38, 10) + Img.at<float>(90, 10) + Img.at<float>(39, 12) + Img.at<float>(89, 12) + Img.at<float>(40, 14) + Img.at<float>(88, 14) + Img.at<float>(42, 17) + Img.at<float>(86, 17) + Img.at<float>(43, 18) + Img.at<float>(85, 18) + Img.at<float>(46, 21) + Img.at<float>(82, 21) + Img.at<float>(47, 22) + Img.at<float>(81, 22) + Img.at<float>(50, 24) + Img.at<float>(78, 24) + Img.at<float>(52, 25) + Img.at<float>(76, 25) + Img.at<float>(54, 26) + Img.at<float>(74, 26) + Img.at<float>(58, 27) + Img.at<float>(70, 27) + 0) / 24;
	Profile[72] = (Img.at<float>(36, 0) + Img.at<float>(92, 0) + Img.at<float>(36, 1) + Img.at<float>(92, 1) + Img.at<float>(36, 2) + Img.at<float>(92, 2) + Img.at<float>(37, 7) + Img.at<float>(91, 7) + Img.at<float>(41, 16) + Img.at<float>(87, 16) + Img.at<float>(48, 23) + Img.at<float>(80, 23) + Img.at<float>(57, 27) + Img.at<float>(71, 27) + Img.at<float>(62, 28) + Img.at<float>(63, 28) + Img.at<float>(64, 28) + Img.at<float>(65, 28) + Img.at<float>(66, 28) + 0) / 19;
	Profile[73] = (Img.at<float>(36, 3) + Img.at<float>(92, 3) + Img.at<float>(36, 4) + Img.at<float>(92, 4) + Img.at<float>(37, 8) + Img.at<float>(91, 8) + Img.at<float>(38, 11) + Img.at<float>(90, 11) + Img.at<float>(39, 13) + Img.at<float>(89, 13) + Img.at<float>(40, 15) + Img.at<float>(88, 15) + Img.at<float>(43, 19) + Img.at<float>(85, 19) + Img.at<float>(44, 20) + Img.at<float>(84, 20) + Img.at<float>(45, 21) + Img.at<float>(83, 21) + Img.at<float>(49, 24) + Img.at<float>(79, 24) + Img.at<float>(51, 25) + Img.at<float>(77, 25) + Img.at<float>(53, 26) + Img.at<float>(75, 26) + Img.at<float>(56, 27) + Img.at<float>(72, 27) + Img.at<float>(60, 28) + Img.at<float>(61, 28) + Img.at<float>(67, 28) + Img.at<float>(68, 28) + 0) / 30;
	Profile[74] = (Img.at<float>(36, 5) + Img.at<float>(92, 5) + Img.at<float>(37, 9) + Img.at<float>(91, 9) + Img.at<float>(41, 17) + Img.at<float>(87, 17) + Img.at<float>(42, 18) + Img.at<float>(86, 18) + Img.at<float>(46, 22) + Img.at<float>(82, 22) + Img.at<float>(47, 23) + Img.at<float>(81, 23) + Img.at<float>(55, 27) + Img.at<float>(73, 27) + Img.at<float>(59, 28) + Img.at<float>(69, 28) + 0) / 16;
	Profile[75] = (Img.at<float>(36, 6) + Img.at<float>(92, 6) + Img.at<float>(36, 7) + Img.at<float>(92, 7) + Img.at<float>(37, 10) + Img.at<float>(91, 10) + Img.at<float>(38, 12) + Img.at<float>(90, 12) + Img.at<float>(39, 14) + Img.at<float>(89, 14) + Img.at<float>(40, 16) + Img.at<float>(88, 16) + Img.at<float>(48, 24) + Img.at<float>(80, 24) + Img.at<float>(50, 25) + Img.at<float>(78, 25) + Img.at<float>(52, 26) + Img.at<float>(76, 26) + Img.at<float>(54, 27) + Img.at<float>(74, 27) + Img.at<float>(57, 28) + Img.at<float>(58, 28) + Img.at<float>(70, 28) + Img.at<float>(71, 28) + 0) / 24;
	Profile[76] = (Img.at<float>(35, 0) + Img.at<float>(93, 0) + Img.at<float>(35, 1) + Img.at<float>(93, 1) + Img.at<float>(35, 2) + Img.at<float>(93, 2) + Img.at<float>(36, 8) + Img.at<float>(92, 8) + Img.at<float>(38, 13) + Img.at<float>(90, 13) + Img.at<float>(42, 19) + Img.at<float>(86, 19) + Img.at<float>(43, 20) + Img.at<float>(85, 20) + Img.at<float>(44, 21) + Img.at<float>(84, 21) + Img.at<float>(45, 22) + Img.at<float>(83, 22) + Img.at<float>(51, 26) + Img.at<float>(77, 26) + Img.at<float>(56, 28) + Img.at<float>(72, 28) + Img.at<float>(62, 29) + Img.at<float>(63, 29) + Img.at<float>(64, 29) + Img.at<float>(65, 29) + Img.at<float>(66, 29) + 0) / 27;
	Profile[77] = (Img.at<float>(35, 3) + Img.at<float>(93, 3) + Img.at<float>(35, 4) + Img.at<float>(93, 4) + Img.at<float>(37, 11) + Img.at<float>(91, 11) + Img.at<float>(39, 15) + Img.at<float>(89, 15) + Img.at<float>(41, 18) + Img.at<float>(87, 18) + Img.at<float>(46, 23) + Img.at<float>(82, 23) + Img.at<float>(49, 25) + Img.at<float>(79, 25) + Img.at<float>(53, 27) + Img.at<float>(75, 27) + Img.at<float>(60, 29) + Img.at<float>(61, 29) + Img.at<float>(67, 29) + Img.at<float>(68, 29) + 0) / 20;
	Profile[78] = (Img.at<float>(35, 5) + Img.at<float>(93, 5) + Img.at<float>(35, 6) + Img.at<float>(93, 6) + Img.at<float>(36, 9) + Img.at<float>(92, 9) + Img.at<float>(37, 12) + Img.at<float>(91, 12) + Img.at<float>(38, 14) + Img.at<float>(90, 14) + Img.at<float>(40, 17) + Img.at<float>(88, 17) + Img.at<float>(47, 24) + Img.at<float>(81, 24) + Img.at<float>(50, 26) + Img.at<float>(78, 26) + Img.at<float>(52, 27) + Img.at<float>(76, 27) + Img.at<float>(55, 28) + Img.at<float>(73, 28) + Img.at<float>(58, 29) + Img.at<float>(59, 29) + Img.at<float>(69, 29) + Img.at<float>(70, 29) + 0) / 24;
	Profile[79] = (Img.at<float>(35, 7) + Img.at<float>(93, 7) + Img.at<float>(36, 10) + Img.at<float>(92, 10) + Img.at<float>(39, 16) + Img.at<float>(89, 16) + Img.at<float>(41, 19) + Img.at<float>(87, 19) + Img.at<float>(42, 20) + Img.at<float>(86, 20) + Img.at<float>(43, 21) + Img.at<float>(85, 21) + Img.at<float>(44, 22) + Img.at<float>(84, 22) + Img.at<float>(45, 23) + Img.at<float>(83, 23) + Img.at<float>(48, 25) + Img.at<float>(80, 25) + Img.at<float>(54, 28) + Img.at<float>(74, 28) + Img.at<float>(57, 29) + Img.at<float>(71, 29) + 0) / 22;
	Profile[80] = (Img.at<float>(34, 0) + Img.at<float>(94, 0) + Img.at<float>(34, 1) + Img.at<float>(94, 1) + Img.at<float>(34, 2) + Img.at<float>(94, 2) + Img.at<float>(35, 8) + Img.at<float>(93, 8) + Img.at<float>(36, 11) + Img.at<float>(92, 11) + Img.at<float>(37, 13) + Img.at<float>(91, 13) + Img.at<float>(38, 15) + Img.at<float>(90, 15) + Img.at<float>(40, 18) + Img.at<float>(88, 18) + Img.at<float>(46, 24) + Img.at<float>(82, 24) + Img.at<float>(49, 26) + Img.at<float>(79, 26) + Img.at<float>(51, 27) + Img.at<float>(77, 27) + Img.at<float>(53, 28) + Img.at<float>(75, 28) + Img.at<float>(56, 29) + Img.at<float>(72, 29) + Img.at<float>(62, 30) + Img.at<float>(63, 30) + Img.at<float>(64, 30) + Img.at<float>(65, 30) + Img.at<float>(66, 30) + 0) / 31;
	Profile[81] = (Img.at<float>(34, 3) + Img.at<float>(94, 3) + Img.at<float>(34, 4) + Img.at<float>(94, 4) + Img.at<float>(35, 9) + Img.at<float>(93, 9) + Img.at<float>(39, 17) + Img.at<float>(89, 17) + Img.at<float>(47, 25) + Img.at<float>(81, 25) + Img.at<float>(55, 29) + Img.at<float>(73, 29) + Img.at<float>(60, 30) + Img.at<float>(61, 30) + Img.at<float>(67, 30) + Img.at<float>(68, 30) + 0) / 16;
	Profile[82] = (Img.at<float>(34, 5) + Img.at<float>(94, 5) + Img.at<float>(34, 6) + Img.at<float>(94, 6) + Img.at<float>(36, 12) + Img.at<float>(92, 12) + Img.at<float>(37, 14) + Img.at<float>(91, 14) + Img.at<float>(38, 16) + Img.at<float>(90, 16) + Img.at<float>(40, 19) + Img.at<float>(88, 19) + Img.at<float>(41, 20) + Img.at<float>(87, 20) + Img.at<float>(42, 21) + Img.at<float>(86, 21) + Img.at<float>(43, 22) + Img.at<float>(85, 22) + Img.at<float>(44, 23) + Img.at<float>(84, 23) + Img.at<float>(45, 24) + Img.at<float>(83, 24) + Img.at<float>(48, 26) + Img.at<float>(80, 26) + Img.at<float>(50, 27) + Img.at<float>(78, 27) + Img.at<float>(52, 28) + Img.at<float>(76, 28) + Img.at<float>(58, 30) + Img.at<float>(59, 30) + Img.at<float>(69, 30) + Img.at<float>(70, 30) + 0) / 32;
	Profile[83] = (Img.at<float>(34, 7) + Img.at<float>(94, 7) + Img.at<float>(35, 10) + Img.at<float>(93, 10) + Img.at<float>(36, 13) + Img.at<float>(92, 13) + Img.at<float>(39, 18) + Img.at<float>(89, 18) + Img.at<float>(46, 25) + Img.at<float>(82, 25) + Img.at<float>(51, 28) + Img.at<float>(77, 28) + Img.at<float>(54, 29) + Img.at<float>(74, 29) + Img.at<float>(57, 30) + Img.at<float>(71, 30) + 0) / 16;
	Profile[84] = (Img.at<float>(33, 0) + Img.at<float>(95, 0) + Img.at<float>(33, 1) + Img.at<float>(95, 1) + Img.at<float>(33, 2) + Img.at<float>(95, 2) + Img.at<float>(34, 8) + Img.at<float>(94, 8) + Img.at<float>(35, 11) + Img.at<float>(93, 11) + Img.at<float>(37, 15) + Img.at<float>(91, 15) + Img.at<float>(38, 17) + Img.at<float>(90, 17) + Img.at<float>(42, 22) + Img.at<float>(86, 22) + Img.at<float>(47, 26) + Img.at<float>(81, 26) + Img.at<float>(49, 27) + Img.at<float>(79, 27) + Img.at<float>(53, 29) + Img.at<float>(75, 29) + Img.at<float>(56, 30) + Img.at<float>(72, 30) + Img.at<float>(62, 31) + Img.at<float>(63, 31) + Img.at<float>(64, 31) + Img.at<float>(65, 31) + Img.at<float>(66, 31) + 0) / 29;
	Profile[85] = (Img.at<float>(33, 3) + Img.at<float>(95, 3) + Img.at<float>(33, 4) + Img.at<float>(95, 4) + Img.at<float>(34, 9) + Img.at<float>(94, 9) + Img.at<float>(36, 14) + Img.at<float>(92, 14) + Img.at<float>(40, 20) + Img.at<float>(88, 20) + Img.at<float>(41, 21) + Img.at<float>(87, 21) + Img.at<float>(43, 23) + Img.at<float>(85, 23) + Img.at<float>(44, 24) + Img.at<float>(84, 24) + Img.at<float>(50, 28) + Img.at<float>(78, 28) + Img.at<float>(55, 30) + Img.at<float>(73, 30) + Img.at<float>(60, 31) + Img.at<float>(61, 31) + Img.at<float>(67, 31) + Img.at<float>(68, 31) + 0) / 24;
	Profile[86] = (Img.at<float>(33, 5) + Img.at<float>(95, 5) + Img.at<float>(33, 6) + Img.at<float>(95, 6) + Img.at<float>(34, 10) + Img.at<float>(94, 10) + Img.at<float>(35, 12) + Img.at<float>(93, 12) + Img.at<float>(37, 16) + Img.at<float>(91, 16) + Img.at<float>(38, 18) + Img.at<float>(90, 18) + Img.at<float>(39, 19) + Img.at<float>(89, 19) + Img.at<float>(45, 25) + Img.at<float>(83, 25) + Img.at<float>(46, 26) + Img.at<float>(82, 26) + Img.at<float>(48, 27) + Img.at<float>(80, 27) + Img.at<float>(52, 29) + Img.at<float>(76, 29) + Img.at<float>(54, 30) + Img.at<float>(74, 30) + Img.at<float>(58, 31) + Img.at<float>(59, 31) + Img.at<float>(69, 31) + Img.at<float>(70, 31) + 0) / 28;
	Profile[87] = (Img.at<float>(33, 7) + Img.at<float>(95, 7) + Img.at<float>(35, 13) + Img.at<float>(93, 13) + Img.at<float>(36, 15) + Img.at<float>(92, 15) + Img.at<float>(41, 22) + Img.at<float>(87, 22) + Img.at<float>(42, 23) + Img.at<float>(86, 23) + Img.at<float>(49, 28) + Img.at<float>(79, 28) + Img.at<float>(51, 29) + Img.at<float>(77, 29) + Img.at<float>(57, 31) + Img.at<float>(71, 31) + 0) / 16;
	Profile[88] = (Img.at<float>(32, 0) + Img.at<float>(96, 0) + Img.at<float>(32, 1) + Img.at<float>(96, 1) + Img.at<float>(32, 2) + Img.at<float>(96, 2) + Img.at<float>(33, 8) + Img.at<float>(95, 8) + Img.at<float>(34, 11) + Img.at<float>(94, 11) + Img.at<float>(37, 17) + Img.at<float>(91, 17) + Img.at<float>(39, 20) + Img.at<float>(89, 20) + Img.at<float>(40, 21) + Img.at<float>(88, 21) + Img.at<float>(43, 24) + Img.at<float>(85, 24) + Img.at<float>(44, 25) + Img.at<float>(84, 25) + Img.at<float>(47, 27) + Img.at<float>(81, 27) + Img.at<float>(53, 30) + Img.at<float>(75, 30) + Img.at<float>(56, 31) + Img.at<float>(72, 31) + Img.at<float>(62, 32) + Img.at<float>(63, 32) + Img.at<float>(64, 32) + Img.at<float>(65, 32) + Img.at<float>(66, 32) + 0) / 31;
	Profile[89] = (Img.at<float>(32, 3) + Img.at<float>(96, 3) + Img.at<float>(32, 4) + Img.at<float>(96, 4) + Img.at<float>(33, 9) + Img.at<float>(95, 9) + Img.at<float>(34, 12) + Img.at<float>(94, 12) + Img.at<float>(35, 14) + Img.at<float>(93, 14) + Img.at<float>(36, 16) + Img.at<float>(92, 16) + Img.at<float>(38, 19) + Img.at<float>(90, 19) + Img.at<float>(45, 26) + Img.at<float>(83, 26) + Img.at<float>(48, 28) + Img.at<float>(80, 28) + Img.at<float>(50, 29) + Img.at<float>(78, 29) + Img.at<float>(52, 30) + Img.at<float>(76, 30) + Img.at<float>(55, 31) + Img.at<float>(73, 31) + Img.at<float>(60, 32) + Img.at<float>(61, 32) + Img.at<float>(67, 32) + Img.at<float>(68, 32) + 0) / 28;
	Profile[90] = (Img.at<float>(32, 5) + Img.at<float>(96, 5) + Img.at<float>(32, 6) + Img.at<float>(96, 6) + Img.at<float>(33, 10) + Img.at<float>(95, 10) + Img.at<float>(37, 18) + Img.at<float>(91, 18) + Img.at<float>(40, 22) + Img.at<float>(88, 22) + Img.at<float>(41, 23) + Img.at<float>(87, 23) + Img.at<float>(42, 24) + Img.at<float>(86, 24) + Img.at<float>(46, 27) + Img.at<float>(82, 27) + Img.at<float>(54, 31) + Img.at<float>(74, 31) + Img.at<float>(58, 32) + Img.at<float>(59, 32) + Img.at<float>(69, 32) + Img.at<float>(70, 32) + 0) / 22;
	Profile[91] = (Img.at<float>(32, 7) + Img.at<float>(96, 7) + Img.at<float>(34, 13) + Img.at<float>(94, 13) + Img.at<float>(35, 15) + Img.at<float>(93, 15) + Img.at<float>(36, 17) + Img.at<float>(92, 17) + Img.at<float>(38, 20) + Img.at<float>(90, 20) + Img.at<float>(39, 21) + Img.at<float>(89, 21) + Img.at<float>(43, 25) + Img.at<float>(85, 25) + Img.at<float>(44, 26) + Img.at<float>(84, 26) + Img.at<float>(47, 28) + Img.at<float>(81, 28) + Img.at<float>(49, 29) + Img.at<float>(79, 29) + Img.at<float>(51, 30) + Img.at<float>(77, 30) + Img.at<float>(57, 32) + Img.at<float>(71, 32) + 0) / 24;
	Profile[92] = (Img.at<float>(31, 0) + Img.at<float>(97, 0) + Img.at<float>(31, 1) + Img.at<float>(97, 1) + Img.at<float>(31, 2) + Img.at<float>(97, 2) + Img.at<float>(32, 8) + Img.at<float>(96, 8) + Img.at<float>(33, 11) + Img.at<float>(95, 11) + Img.at<float>(34, 14) + Img.at<float>(94, 14) + Img.at<float>(35, 16) + Img.at<float>(93, 16) + Img.at<float>(37, 19) + Img.at<float>(91, 19) + Img.at<float>(45, 27) + Img.at<float>(83, 27) + Img.at<float>(48, 29) + Img.at<float>(80, 29) + Img.at<float>(50, 30) + Img.at<float>(78, 30) + Img.at<float>(53, 31) + Img.at<float>(75, 31) + Img.at<float>(56, 32) + Img.at<float>(72, 32) + Img.at<float>(62, 33) + Img.at<float>(63, 33) + Img.at<float>(64, 33) + Img.at<float>(65, 33) + Img.at<float>(66, 33) + 0) / 31;
	Profile[93] = (Img.at<float>(31, 3) + Img.at<float>(97, 3) + Img.at<float>(31, 4) + Img.at<float>(97, 4) + Img.at<float>(32, 9) + Img.at<float>(96, 9) + Img.at<float>(33, 12) + Img.at<float>(95, 12) + Img.at<float>(36, 18) + Img.at<float>(92, 18) + Img.at<float>(39, 22) + Img.at<float>(89, 22) + Img.at<float>(40, 23) + Img.at<float>(88, 23) + Img.at<float>(41, 24) + Img.at<float>(87, 24) + Img.at<float>(42, 25) + Img.at<float>(86, 25) + Img.at<float>(46, 28) + Img.at<float>(82, 28) + Img.at<float>(52, 31) + Img.at<float>(76, 31) + Img.at<float>(55, 32) + Img.at<float>(73, 32) + Img.at<float>(60, 33) + Img.at<float>(61, 33) + Img.at<float>(67, 33) + Img.at<float>(68, 33) + 0) / 28;
	Profile[94] = (Img.at<float>(31, 5) + Img.at<float>(97, 5) + Img.at<float>(31, 6) + Img.at<float>(97, 6) + Img.at<float>(32, 10) + Img.at<float>(96, 10) + Img.at<float>(33, 13) + Img.at<float>(95, 13) + Img.at<float>(34, 15) + Img.at<float>(94, 15) + Img.at<float>(35, 17) + Img.at<float>(93, 17) + Img.at<float>(37, 20) + Img.at<float>(91, 20) + Img.at<float>(38, 21) + Img.at<float>(90, 21) + Img.at<float>(43, 26) + Img.at<float>(85, 26) + Img.at<float>(44, 27) + Img.at<float>(84, 27) + Img.at<float>(47, 29) + Img.at<float>(81, 29) + Img.at<float>(49, 30) + Img.at<float>(79, 30) + Img.at<float>(51, 31) + Img.at<float>(77, 31) + Img.at<float>(54, 32) + Img.at<float>(74, 32) + Img.at<float>(58, 33) + Img.at<float>(59, 33) + Img.at<float>(69, 33) + Img.at<float>(70, 33) + 0) / 32;
	Profile[95] = (Img.at<float>(31, 7) + Img.at<float>(97, 7) + Img.at<float>(32, 11) + Img.at<float>(96, 11) + Img.at<float>(36, 19) + Img.at<float>(92, 19) + Img.at<float>(45, 28) + Img.at<float>(83, 28) + Img.at<float>(53, 32) + Img.at<float>(75, 32) + Img.at<float>(57, 33) + Img.at<float>(71, 33) + 0) / 12;
	Profile[96] = (Img.at<float>(30, 0) + Img.at<float>(98, 0) + Img.at<float>(30, 1) + Img.at<float>(98, 1) + Img.at<float>(30, 2) + Img.at<float>(98, 2) + Img.at<float>(31, 8) + Img.at<float>(97, 8) + Img.at<float>(33, 14) + Img.at<float>(95, 14) + Img.at<float>(34, 16) + Img.at<float>(94, 16) + Img.at<float>(38, 22) + Img.at<float>(90, 22) + Img.at<float>(39, 23) + Img.at<float>(89, 23) + Img.at<float>(40, 24) + Img.at<float>(88, 24) + Img.at<float>(41, 25) + Img.at<float>(87, 25) + Img.at<float>(42, 26) + Img.at<float>(86, 26) + Img.at<float>(48, 30) + Img.at<float>(80, 30) + Img.at<float>(50, 31) + Img.at<float>(78, 31) + Img.at<float>(56, 33) + Img.at<float>(72, 33) + Img.at<float>(62, 34) + Img.at<float>(63, 34) + Img.at<float>(64, 34) + Img.at<float>(65, 34) + Img.at<float>(66, 34) + 0) / 33;
	Profile[97] = (Img.at<float>(30, 3) + Img.at<float>(98, 3) + Img.at<float>(30, 4) + Img.at<float>(98, 4) + Img.at<float>(30, 5) + Img.at<float>(98, 5) + Img.at<float>(31, 9) + Img.at<float>(97, 9) + Img.at<float>(32, 12) + Img.at<float>(96, 12) + Img.at<float>(35, 18) + Img.at<float>(93, 18) + Img.at<float>(37, 21) + Img.at<float>(91, 21) + Img.at<float>(43, 27) + Img.at<float>(85, 27) + Img.at<float>(46, 29) + Img.at<float>(82, 29) + Img.at<float>(52, 32) + Img.at<float>(76, 32) + Img.at<float>(55, 33) + Img.at<float>(73, 33) + Img.at<float>(59, 34) + Img.at<float>(60, 34) + Img.at<float>(61, 34) + Img.at<float>(67, 34) + Img.at<float>(68, 34) + Img.at<float>(69, 34) + 0) / 28;
	Profile[98] = (Img.at<float>(30, 6) + Img.at<float>(98, 6) + Img.at<float>(31, 10) + Img.at<float>(97, 10) + Img.at<float>(32, 13) + Img.at<float>(96, 13) + Img.at<float>(33, 15) + Img.at<float>(95, 15) + Img.at<float>(34, 17) + Img.at<float>(94, 17) + Img.at<float>(36, 20) + Img.at<float>(92, 20) + Img.at<float>(44, 28) + Img.at<float>(84, 28) + Img.at<float>(47, 30) + Img.at<float>(81, 30) + Img.at<float>(49, 31) + Img.at<float>(79, 31) + Img.at<float>(51, 32) + Img.at<float>(77, 32) + Img.at<float>(54, 33) + Img.at<float>(74, 33) + Img.at<float>(58, 34) + Img.at<float>(70, 34) + 0) / 24;
	Profile[99] = (Img.at<float>(30, 7) + Img.at<float>(98, 7) + Img.at<float>(31, 11) + Img.at<float>(97, 11) + Img.at<float>(35, 19) + Img.at<float>(93, 19) + Img.at<float>(37, 22) + Img.at<float>(91, 22) + Img.at<float>(38, 23) + Img.at<float>(90, 23) + Img.at<float>(39, 24) + Img.at<float>(89, 24) + Img.at<float>(40, 25) + Img.at<float>(88, 25) + Img.at<float>(41, 26) + Img.at<float>(87, 26) + Img.at<float>(42, 27) + Img.at<float>(86, 27) + Img.at<float>(45, 29) + Img.at<float>(83, 29) + Img.at<float>(53, 33) + Img.at<float>(75, 33) + Img.at<float>(57, 34) + Img.at<float>(71, 34) + 0) / 24;
	/*
	Function KickAss_Generator_3()
	variable Xc, Yc, SI, j, i, k, radius
	variable count = 0
	Make / O / N = (65, 128) Wx = 0, Wy = 0
	Wx[][] = Wx[x][y] + x
	Wy[][] = Wy[x][y] + y
	Xc = 0
	Yc = 64

	for (k = 40; k<140; k += 1)
	count = 0
	string Sums = "Profile.at<float>(" + num2str(k) + ")=("
	for (i = 0; i<65; i += 1)
	for (j = 0; j<128; j += 1)

	radius = round(sqrt((Wx[i][j])*(Wx[i][j]) + (Wy[i][j] - Yc)*(Wy[i][j] - Yc)) * 4)
	if (radius == k)
	Sums += "Img.at<float>(" + num2str(j) + "," + num2str(i) + ")+"
	count += 1
	endif
	endfor
	endfor
	Sums += "0)/" + num2str(count) + ";"
	print "  "
	print / LEN = 5000 Sums
	endfor
	End
	*/
	return (*Profile);
}

void MT_2_N52::show_ROI()
{
	
	cam.SetOffsetX(0); cam.SetOffsetY(0);
	cam.SetWidth(1280); cam.SetHeight(1024);
	cam.SetExposureTime(ui.cam_exposure->value()); cam.SetGain(8);
	Mat Img = Mat::zeros(1280, 1024, CV_32F);
	imshow("Full Image", Img);
	moveWindow("Full Image", 600, 0);
	setMouseCallback("Full Image", onmouse, NULL);
	ShowButtons(0, 0, 1, 0);
	Focus_box(9000);
	ui.ROIButton->setEnabled(true);
	ui.BeadType->setVisible(false);
	ui.expLabel->setVisible(true);
	ui.expLabel->setText("Exposure");
	ui.cam_exposure->setVisible(true);
	ui.MagnetPositionBox->setDecimals(2); ui.MagnetPositionBox->setMinimum(1.4); ui.MagnetPositionBox->setMaximum(9); ui.MagnetPositionBox->setSingleStep(0.1); ui.MagnetPositionBox->setValue(5.8); ui.MagnetPositionBox->setKeyboardTracking(0);
	ui.MP_textBox->setText("MP(mm)"); MP_flag = true;
	do
	{
		Img = cam.GetNextImageOcvMat(); Img.convertTo(Img, CV_32F); normalize(Img, Img, 0, 1, NORM_MINMAX);
		rectangle(Img, M_1, M_2, Scalar(1, 1, 1), 1, 8, 0); rectangle(Img, R_1, R_2, Scalar(1, 1, 1), 1, 8, 0);
		if (mouse_c == 2) { rectangle(Img, Point(min(M_1.x, R_1.x), min(M_1.y, R_1.y)), Point(max(M_2.x, R_2.x), max(M_2.y, R_2.y)), Scalar(1, 1, 1), 2, 8, 0); ui.ROIButton->setEnabled(true); }
		imshow("Full Image", Img);
		waitKey(20);
		if (middown == 5) { ui.ROIButton->setChecked(false); }
	} while (ui.ROIButton->isChecked() == true); // while (ui.ROIButton->isChecked() == true);
	//ui.ROIButton->setChecked(false);
	waitKey(20);
	destroyWindow("Full Image");
	if (M_1.x > 0 && M_1.y > 0 && M_2.x > 0 && M_2.y > 0 && R_1.x > 0 && R_1.y > 0 && R_2.x > 0 && R_2.y > 0) {  //Creado para no poder apretar measure, si esque no se habian selecionado los puntos primero
		ShowButtons(1, 0, 1, 0); // cambie esto " ShowButtons(1, 0, 0, 0);"
	}

	//ShowButtons(1, 0, 0, 0);
	//ui.ROIButton->setEnabled(true);
	//ui.ROIButton->setChecked(false);
	middown = 0;
	if (middown == 5) { ui.ROIButton->setChecked(true); }
	ui.BeadType->setVisible(false);
	ui.cam_exposure->setVisible(false);
	ui.expLabel->setVisible(false); //Hay que implementar
	ui.MagnetPositionBox->setDecimals(2); ui.MagnetPositionBox->setSingleStep(0.1); ui.MagnetPositionBox->setValue(5.8); ui.MagnetPositionBox->setKeyboardTracking(0);
	Update_bead_type();
	ui.MP_textBox->setText("MP (mm)"); MP_flag = true;
	return;
}

void onmouse(int event, int x, int y, int flags, void* param)
{
	mouse_x = x; mouse_y = y;
	if (event == EVENT_LBUTTONDOWN) { M_1.y = y - 64; M_1.x = x - 64; M_2.y = y + 64; M_2.x = x + 64; mouse_c = 0; }
	if (event == EVENT_RBUTTONDOWN) { R_1.y = y - 64; R_1.x = x - 64; R_2.y = y + 64; R_2.x = x + 64; mouse_c = 1; }
	if (event == EVENT_MBUTTONDOWN) { mouse_c = 2; middown = 5; }
	return;
}

void MT_2_N52::MagnetPosition_box(double F)
{
	if (MP_flag == true)
	{
		double MP = F;
		DAQmxClearTask(taskHandle_0); ui.execute_pulse->setChecked(false);
		DAQmxCreateTask("", &taskHandle_0);
		DAQmxCreateAOVoltageChan(taskHandle_0, "Dev1/ao0", "", -5, 5, DAQmx_Val_Volts, "");
		DAQmxStartTask(&taskHandle_0);
		float64	DAC_Pulse[1] = { 2.0 - MP / 2 };;
		DAQmxWriteAnalogF64(taskHandle_0, 1, true, 1, DAQmx_Val_GroupByChannel, DAC_Pulse, NULL, NULL);
	}
	/*if (MP_flag == false)
	{
		double MP= Magnet_Law(F, ui.BeadType->isChecked(), "F_MP");
		DAQmxClearTask(taskHandle_0); ui.execute_pulse->setChecked(false);
		DAQmxCreateTask("", &taskHandle_0);
		DAQmxCreateAOVoltageChan(taskHandle_0, "Dev1/ao0", "", -5, 5, DAQmx_Val_Volts, "");
		DAQmxStartTask(&taskHandle_0);
		float64	DAC_Pulse[1] = { 2.0 - MP / 2 };;
		DAQmxWriteAnalogF64(taskHandle_0, 1, true, 1, DAQmx_Val_GroupByChannel, DAC_Pulse, NULL, NULL);
	}*/
	return;
}

void MT_2_N52::Focus_box(int value)
{
	ui.FocusBox->setValue(value);
	currentFocus = value;
	float64	FP = value / 1800.00;//  for a slider range 0-18000 nm
	//float64		FP = value / 744.00 ;//  for a slider range 0-18000 nm// Shubhasis
	DAQmxWriteAnalogScalarF64(taskHandle_1, 0, 0.1, FP, NULL);
	return;
}

void MT_2_N52::Send_pulse()
{

	float duration = 0;

	int j, k = 0;

	QStringList pulses = { "" }; pulses = ui.Pulse_protocol->toPlainText().split(";");
	for (int i = 0; i<pulses.size() - 1; i++)
	{
		QStringList pulse = pulses.at(i).split("/");
		duration += pulse.at(1).toFloat();
	}

	int n_points = duration*update_rate;
	float64* DAC_pulse = new float64[n_points];
	double MagPos;

	for (int i = 0; i < pulses.size() - 1; i++)
	{
		QStringList pulse = pulses.at(i).split("/");
		char t_type = pulse.at(2).toStdString()[0];
		MagPos = pulse.at(0).toDouble();
		switch (t_type)
		{
		case 'C':
			for (j = k; j < (pulse.at(1).toFloat() * update_rate) + k; j++) { DAC_pulse[j] = pulse.at(0).toFloat(); }
			break;
		case 'L':
			QStringList pulse_next = pulses.at(i + 1).split("/"); float64* F_array = new float64[pulse.at(1).toFloat() * update_rate]; float F_start = pulse.at(0).toFloat(); float F_end = pulse_next.at(0).toFloat();
			for (j = 0; j < (pulse.at(1).toFloat()*update_rate); j++) { F_array[j] = F_start + j*((F_end - F_start) / pulse.at(1).toFloat()) / update_rate; DAC_pulse[j + k] = F_array[j]; }
			delete[] F_array;
			break;
		}
		k += pulse.at(1).toFloat()*update_rate;
		
	}

	for (int i = 0; i < n_points; i++)
	{
		//DAC_pulse[i] = Magnet_Law(DAC_pulse[i], ui.BeadType->isChecked(), "F_MP");
		DAC_pulse[i] = 2 - DAC_pulse[i] / 2; ////////////////// VOICE COIL SCALING HERE ///////////////////////
		
	}

	if (ui.execute_pulse->isChecked())
	{
		DAQmxClearTask(taskHandle_0);
		DAQmxCreateTask("", &taskHandle_0);
		DAQmxCreateAOVoltageChan(taskHandle_0, "Dev1/ao0", "", -2.5, 2.5, DAQmx_Val_Volts, "");
		DAQmxCfgSampClkTiming(taskHandle_0, NULL, update_rate, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 60000000);
		DAQmxWriteAnalogF64(taskHandle_0, n_points, true, 1, DAQmx_Val_GroupByChannel, DAC_pulse, NULL, NULL);
		DAQmxStartTask(taskHandle_0);
		

	}
	else
	{
		for (int i = 0; i < n_points; i++) { DAC_pulse[i] = 2 - Magnet_Law(ui.MagnetPositionBox->value(), ui.BeadType->isChecked(), "F_MP") / 2;}
		DAQmxClearTask(taskHandle_0);
		DAQmxCreateTask("", &taskHandle_0);
		DAQmxCreateAOVoltageChan(taskHandle_0, "Dev1/ao0", "", -2.5, 2.5, DAQmx_Val_Volts, "");
		DAQmxCfgSampClkTiming(taskHandle_0, NULL, update_rate, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 60000000);
		DAQmxWriteAnalogF64(taskHandle_0, n_points, true, 1, DAQmx_Val_GroupByChannel, DAC_pulse, NULL, NULL);
		DAQmxStartTask(taskHandle_0);
		DAQmxStopTask(&taskHandle_0);
	}
	return;
}

void MT_2_N52::ShowButtons(bool measure, bool savedata, bool ROI, bool stack)
{
	ui.MeasureButton->setEnabled(measure);
	ui.SaveDataButton->setEnabled(savedata);
	ui.ROIButton->setEnabled(ROI);
	ui.stackButton->setEnabled(stack);
	return;
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32       read = 0;
	DAQmxReadAnalogF64(taskHandle, 100, 10.0, DAQmx_Val_GroupByScanNumber, AI_Buffer, 100, &read, NULL);
	AI_average = 0; for (int i = 0; i < 100; i++) { AI_average += AI_Buffer[i]; } AI_average /= 100;
	return 0;
}

double MT_2_N52::Magnet_Law (double value, bool bead, QString direction)
{
	double MP;double F;

	if (bead == TRUE)	// M270 beads
	{
		if (direction == "F_MP") { MP = value; return MP;} // { MP = 0.99 - (1 / 0.9)*log(value / 65); return MP; }
	 
		if (direction == "MP_F") { F = 65 * exp(0.9*(0.99 - value)); return F; }
	}
	if (bead == FALSE)  // M450 beads
	{
		if (direction == "F_MP") { MP =value; return MP; } // { MP =-(1 / 0.69)*log(value / 360); return MP; }
		if (direction == "MP_F") { F = 360 * exp(- value*0.69); return F; }
	}
}

void MT_2_N52::Update_bead_type() 
{
	if (ui.BeadType->isChecked() == true) {ui.MagnetPositionBox->setMinimum(1.4); ui.MagnetPositionBox->setMaximum(9); }
	if (ui.BeadType->isChecked() == false) {ui.MagnetPositionBox->setMinimum(1.4); ui.MagnetPositionBox->setMaximum(9); }
	MagnetPosition_box(ui.MagnetPositionBox->value());
}

void MT_2_N52::on_MagnetPositionBox_valueChanged(double arg1)
{
	ui.SliderMP->setValue(int(arg1 * 10));
}


void MT_2_N52::on_SliderMP_valueChanged(int value)
{
	const int dpi = 10;
	double val = value;
	ui.MagnetPositionBox->setValue(double(val / dpi));
}

void MT_2_N52::on_FocusSlider_valueChanged(int value)
{
	
	double val = value;
	ui.FocusBox->setValue(double(val));

}

void MT_2_N52::on_FocusBox_valueChanged(double arg)
{
	ui.FocusSlider->setValue(int(arg));

}


/*
void MT_2_N52::on_FocusBox_valueChanged(double arg2)
{
	ui.FocusSlider->setValue(int(arg2));
}

void MT_2_N52::on_FocusSlider_valueChanged(int value2)
{
	const int dpi2 = 10;
	double val1 = value2;
	ui.FocusBox->setValue(double(val1 / dpi2));

}
*/
