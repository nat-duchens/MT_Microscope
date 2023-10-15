#pragma once

#include <QtWidgets/QMainWindow>
#include <xiApiPlusOcv.hpp>
#include <NIDAQmx.h>
#include "ui_MT_2_N52.h"
#include <QFile>
#include <QFileDialog>


class MT_2_N52 : public QMainWindow
{
	Q_OBJECT
public:
	MT_2_N52(QWidget *parent = Q_NULLPTR);
	xiAPIplusCameraOcv cam;
	TaskHandle	taskHandle_0 = 0;
	TaskHandle	taskHandle_1 = 0;
	TaskHandle  taskHandle_2 = 0;
	QGraphicsScene *scene;
	void ShowButtons(bool preview, bool init, bool ROI, bool stack);
	double Magnet_Law(double value, bool bead, QString direction);
	void Update_bead_type();
	public slots:

	void measure();
	void show_ROI();
	void get_stack();
	void Focus_box(int value);
	void MagnetPosition_box(double F);
	void Send_pulse();
	void on_MagnetPositionBox_valueChanged(double arg1);
	void on_SliderMP_valueChanged(int value);
	void on_FocusSlider_valueChanged(int value);
	void on_FocusBox_valueChanged(double arg);
private:
	Ui::MT_2_N52Class ui;
};

