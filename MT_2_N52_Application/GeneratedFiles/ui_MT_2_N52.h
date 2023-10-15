/********************************************************************************
** Form generated from reading UI file 'MT_2_N52.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MT_2_N52_H
#define UI_MT_2_N52_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MT_2_N52Class
{
public:
    QAction *actionshorcutROI;
    QWidget *centralWidget;
    QLabel *version;
    QLabel *label_2;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_4;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer_2;
    QLabel *MP_textBox;
    QDoubleSpinBox *MagnetPositionBox;
    QSpacerItem *verticalSpacer;
    QSlider *SliderMP;
    QFrame *line;
    QHBoxLayout *horizontalLayout_3;
    QVBoxLayout *verticalLayout_2;
    QSpacerItem *verticalSpacer_4;
    QLabel *label_3;
    QDoubleSpinBox *FocusBox;
    QSpacerItem *verticalSpacer_3;
    QSlider *FocusSlider;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QCheckBox *lockReference;
    QFrame *line_2;
    QPlainTextEdit *Pulse_protocol;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *ROIButton;
    QPushButton *MeasureButton;
    QPushButton *stackButton;
    QPushButton *SaveDataButton;
    QHBoxLayout *horizontalLayout_6;
    QSpacerItem *horizontalSpacer_2;
    QLabel *FramesPerSecondText;
    QCheckBox *BeadType;
    QLabel *expLabel;
    QSpinBox *cam_exposure;
    QCheckBox *View_fast_slow;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_7;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *execute_pulse;
    QSpacerItem *horizontalSpacer_4;

    void setupUi(QMainWindow *MT_2_N52Class)
    {
        if (MT_2_N52Class->objectName().isEmpty())
            MT_2_N52Class->setObjectName(QString::fromUtf8("MT_2_N52Class"));
        MT_2_N52Class->setWindowModality(Qt::ApplicationModal);
        MT_2_N52Class->setEnabled(true);
        MT_2_N52Class->resize(347, 293);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MT_2_N52Class->sizePolicy().hasHeightForWidth());
        MT_2_N52Class->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QString::fromUtf8("mecano.ico"), QSize(), QIcon::Normal, QIcon::Off);
        MT_2_N52Class->setWindowIcon(icon);
        MT_2_N52Class->setIconSize(QSize(46, 48));
        actionshorcutROI = new QAction(MT_2_N52Class);
        actionshorcutROI->setObjectName(QString::fromUtf8("actionshorcutROI"));
        centralWidget = new QWidget(MT_2_N52Class);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        version = new QLabel(centralWidget);
        version->setObjectName(QString::fromUtf8("version"));
        version->setGeometry(QRect(20, 260, 321, 16));
        version->setAlignment(Qt::AlignCenter);
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(9, 9, 331, 16));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        label_2->setFont(font);
        label_2->setLineWidth(0);
        layoutWidget = new QWidget(centralWidget);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(9, 28, 331, 231));
        verticalLayout_3 = new QVBoxLayout(layoutWidget);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        MP_textBox = new QLabel(layoutWidget);
        MP_textBox->setObjectName(QString::fromUtf8("MP_textBox"));
        MP_textBox->setFrameShape(QFrame::NoFrame);
        MP_textBox->setFrameShadow(QFrame::Plain);
        MP_textBox->setTextFormat(Qt::RichText);

        verticalLayout->addWidget(MP_textBox);

        MagnetPositionBox = new QDoubleSpinBox(layoutWidget);
        MagnetPositionBox->setObjectName(QString::fromUtf8("MagnetPositionBox"));
        MagnetPositionBox->setAlignment(Qt::AlignCenter);
        MagnetPositionBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        MagnetPositionBox->setDecimals(1);
        MagnetPositionBox->setMinimum(1.400000000000000);
        MagnetPositionBox->setMaximum(9.000000000000000);
        MagnetPositionBox->setSingleStep(0.100000000000000);

        verticalLayout->addWidget(MagnetPositionBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        horizontalLayout->addLayout(verticalLayout);

        SliderMP = new QSlider(layoutWidget);
        SliderMP->setObjectName(QString::fromUtf8("SliderMP"));
        SliderMP->setMinimum(14);
        SliderMP->setMaximum(90);
        SliderMP->setValue(55);
        SliderMP->setOrientation(Qt::Vertical);

        horizontalLayout->addWidget(SliderMP);


        horizontalLayout_4->addLayout(horizontalLayout);

        line = new QFrame(layoutWidget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setLineWidth(2);
        line->setMidLineWidth(1);
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        horizontalLayout_4->addWidget(line);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_4);

        label_3 = new QLabel(layoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setFrameShape(QFrame::NoFrame);
        label_3->setFrameShadow(QFrame::Plain);
        label_3->setTextFormat(Qt::RichText);
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        verticalLayout_2->addWidget(label_3);

        FocusBox = new QDoubleSpinBox(layoutWidget);
        FocusBox->setObjectName(QString::fromUtf8("FocusBox"));
        FocusBox->setMouseTracking(false);
        FocusBox->setAlignment(Qt::AlignCenter);
        FocusBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        FocusBox->setDecimals(0);
        FocusBox->setMaximum(100.000000000000000);
        FocusBox->setSingleStep(100.000000000000000);
        FocusBox->setValue(0.000000000000000);

        verticalLayout_2->addWidget(FocusBox);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_3);


        horizontalLayout_3->addLayout(verticalLayout_2);

        FocusSlider = new QSlider(layoutWidget);
        FocusSlider->setObjectName(QString::fromUtf8("FocusSlider"));
        FocusSlider->setMaximum(18000);
        FocusSlider->setSingleStep(10);
        FocusSlider->setPageStep(10);
        FocusSlider->setValue(9000);
        FocusSlider->setOrientation(Qt::Vertical);

        horizontalLayout_3->addWidget(FocusSlider);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label = new QLabel(layoutWidget);
        label->setObjectName(QString::fromUtf8("label"));
        QFont font1;
        font1.setPointSize(8);
        label->setFont(font1);

        horizontalLayout_2->addWidget(label);

        lockReference = new QCheckBox(layoutWidget);
        lockReference->setObjectName(QString::fromUtf8("lockReference"));
        lockReference->setLayoutDirection(Qt::RightToLeft);

        horizontalLayout_2->addWidget(lockReference);


        horizontalLayout_3->addLayout(horizontalLayout_2);


        horizontalLayout_4->addLayout(horizontalLayout_3);

        line_2 = new QFrame(layoutWidget);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setLineWidth(2);
        line_2->setMidLineWidth(1);
        line_2->setFrameShape(QFrame::VLine);
        line_2->setFrameShadow(QFrame::Sunken);

        horizontalLayout_4->addWidget(line_2);

        Pulse_protocol = new QPlainTextEdit(layoutWidget);
        Pulse_protocol->setObjectName(QString::fromUtf8("Pulse_protocol"));
        QFont font2;
        font2.setFamily(QString::fromUtf8("MS Sans Serif"));
        font2.setPointSize(10);
        font2.setBold(true);
        font2.setWeight(75);
        Pulse_protocol->setFont(font2);
        Pulse_protocol->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        horizontalLayout_4->addWidget(Pulse_protocol);


        verticalLayout_3->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        ROIButton = new QPushButton(layoutWidget);
        ROIButton->setObjectName(QString::fromUtf8("ROIButton"));
        QPalette palette;
        QBrush brush(QColor(0, 0, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
        QBrush brush1(QColor(120, 120, 120, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush1);
        ROIButton->setPalette(palette);
        ROIButton->setFont(font);

        horizontalLayout_5->addWidget(ROIButton);

        MeasureButton = new QPushButton(layoutWidget);
        MeasureButton->setObjectName(QString::fromUtf8("MeasureButton"));
        QPalette palette1;
        QBrush brush2(QColor(0, 170, 0, 255));
        brush2.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::ButtonText, brush2);
        palette1.setBrush(QPalette::Inactive, QPalette::ButtonText, brush2);
        palette1.setBrush(QPalette::Disabled, QPalette::ButtonText, brush1);
        MeasureButton->setPalette(palette1);
        MeasureButton->setFont(font);
        MeasureButton->setAutoFillBackground(false);
        MeasureButton->setCheckable(false);
        MeasureButton->setFlat(false);

        horizontalLayout_5->addWidget(MeasureButton);

        stackButton = new QPushButton(layoutWidget);
        stackButton->setObjectName(QString::fromUtf8("stackButton"));
        QPalette palette2;
        QBrush brush3(QColor(170, 0, 0, 255));
        brush3.setStyle(Qt::SolidPattern);
        palette2.setBrush(QPalette::Active, QPalette::ButtonText, brush3);
        palette2.setBrush(QPalette::Inactive, QPalette::ButtonText, brush3);
        palette2.setBrush(QPalette::Disabled, QPalette::ButtonText, brush1);
        stackButton->setPalette(palette2);
        stackButton->setFont(font);
        stackButton->setLayoutDirection(Qt::LeftToRight);

        horizontalLayout_5->addWidget(stackButton);

        SaveDataButton = new QPushButton(layoutWidget);
        SaveDataButton->setObjectName(QString::fromUtf8("SaveDataButton"));
        QPalette palette3;
        QBrush brush4(QColor(170, 0, 127, 255));
        brush4.setStyle(Qt::SolidPattern);
        palette3.setBrush(QPalette::Active, QPalette::ButtonText, brush4);
        palette3.setBrush(QPalette::Inactive, QPalette::ButtonText, brush4);
        palette3.setBrush(QPalette::Disabled, QPalette::ButtonText, brush1);
        SaveDataButton->setPalette(palette3);
        SaveDataButton->setFont(font);

        horizontalLayout_5->addWidget(SaveDataButton);


        verticalLayout_3->addLayout(horizontalLayout_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        horizontalSpacer_2 = new QSpacerItem(100, 30, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_2);

        FramesPerSecondText = new QLabel(layoutWidget);
        FramesPerSecondText->setObjectName(QString::fromUtf8("FramesPerSecondText"));
        QPalette palette4;
        palette4.setBrush(QPalette::Active, QPalette::Text, brush3);
        QBrush brush5(QColor(170, 0, 0, 128));
        brush5.setStyle(Qt::NoBrush);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette4.setBrush(QPalette::Active, QPalette::PlaceholderText, brush5);
#endif
        palette4.setBrush(QPalette::Inactive, QPalette::Text, brush3);
        QBrush brush6(QColor(170, 0, 0, 128));
        brush6.setStyle(Qt::NoBrush);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette4.setBrush(QPalette::Inactive, QPalette::PlaceholderText, brush6);
#endif
        palette4.setBrush(QPalette::Disabled, QPalette::Text, brush1);
        QBrush brush7(QColor(170, 0, 0, 128));
        brush7.setStyle(Qt::NoBrush);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        palette4.setBrush(QPalette::Disabled, QPalette::PlaceholderText, brush7);
#endif
        FramesPerSecondText->setPalette(palette4);
        QFont font3;
        font3.setFamily(QString::fromUtf8("Arial"));
        font3.setPointSize(12);
        font3.setBold(true);
        font3.setWeight(75);
        FramesPerSecondText->setFont(font3);
        FramesPerSecondText->setFrameShape(QFrame::Box);
        FramesPerSecondText->setFrameShadow(QFrame::Sunken);
        FramesPerSecondText->setLineWidth(2);
        FramesPerSecondText->setTextFormat(Qt::PlainText);
        FramesPerSecondText->setScaledContents(false);
        FramesPerSecondText->setAlignment(Qt::AlignCenter);

        horizontalLayout_6->addWidget(FramesPerSecondText);

        BeadType = new QCheckBox(layoutWidget);
        BeadType->setObjectName(QString::fromUtf8("BeadType"));

        horizontalLayout_6->addWidget(BeadType);

        expLabel = new QLabel(layoutWidget);
        expLabel->setObjectName(QString::fromUtf8("expLabel"));

        horizontalLayout_6->addWidget(expLabel);

        cam_exposure = new QSpinBox(layoutWidget);
        cam_exposure->setObjectName(QString::fromUtf8("cam_exposure"));
        cam_exposure->setLayoutDirection(Qt::LeftToRight);
        cam_exposure->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        cam_exposure->setButtonSymbols(QAbstractSpinBox::NoButtons);

        horizontalLayout_6->addWidget(cam_exposure);

        View_fast_slow = new QCheckBox(layoutWidget);
        View_fast_slow->setObjectName(QString::fromUtf8("View_fast_slow"));
        View_fast_slow->setLayoutDirection(Qt::RightToLeft);

        horizontalLayout_6->addWidget(View_fast_slow);

        horizontalSpacer = new QSpacerItem(100, 30, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer);


        verticalLayout_3->addLayout(horizontalLayout_6);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_3);

        execute_pulse = new QPushButton(layoutWidget);
        execute_pulse->setObjectName(QString::fromUtf8("execute_pulse"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(10);
        sizePolicy1.setVerticalStretch(10);
        sizePolicy1.setHeightForWidth(execute_pulse->sizePolicy().hasHeightForWidth());
        execute_pulse->setSizePolicy(sizePolicy1);
        QPalette palette5;
        QBrush brush8(QColor(85, 0, 255, 255));
        brush8.setStyle(Qt::SolidPattern);
        palette5.setBrush(QPalette::Active, QPalette::ButtonText, brush8);
        palette5.setBrush(QPalette::Inactive, QPalette::ButtonText, brush8);
        palette5.setBrush(QPalette::Disabled, QPalette::ButtonText, brush1);
        execute_pulse->setPalette(palette5);
        execute_pulse->setFont(font);

        horizontalLayout_7->addWidget(execute_pulse);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_4);


        verticalLayout_3->addLayout(horizontalLayout_7);

        MT_2_N52Class->setCentralWidget(centralWidget);

        retranslateUi(MT_2_N52Class);

        QMetaObject::connectSlotsByName(MT_2_N52Class);
    } // setupUi

    void retranslateUi(QMainWindow *MT_2_N52Class)
    {
        MT_2_N52Class->setWindowTitle(QCoreApplication::translate("MT_2_N52Class", "MT_2_N52", nullptr));
        actionshorcutROI->setText(QCoreApplication::translate("MT_2_N52Class", "shorcutROI", nullptr));
#if QT_CONFIG(shortcut)
        actionshorcutROI->setShortcut(QCoreApplication::translate("MT_2_N52Class", "R", nullptr));
#endif // QT_CONFIG(shortcut)
        version->setText(QString());
        label_2->setText(QCoreApplication::translate("MT_2_N52Class", "Magnet Pos.       Focal                                      Pulse Protocol", nullptr));
        MP_textBox->setText(QCoreApplication::translate("MT_2_N52Class", "MP (mm)", nullptr));
        label_3->setText(QCoreApplication::translate("MT_2_N52Class", "OP(nm)", nullptr));
        label->setText(QCoreApplication::translate("MT_2_N52Class", "Lock", nullptr));
        lockReference->setText(QString());
        ROIButton->setText(QCoreApplication::translate("MT_2_N52Class", "ROI", nullptr));
        MeasureButton->setText(QCoreApplication::translate("MT_2_N52Class", "Measure", nullptr));
        stackButton->setText(QCoreApplication::translate("MT_2_N52Class", "Stack", nullptr));
        SaveDataButton->setText(QCoreApplication::translate("MT_2_N52Class", "Save", nullptr));
        FramesPerSecondText->setText(QString());
        BeadType->setText(QCoreApplication::translate("MT_2_N52Class", "M270/M450", nullptr));
        expLabel->setText(QCoreApplication::translate("MT_2_N52Class", "Exposure", nullptr));
        View_fast_slow->setText(QCoreApplication::translate("MT_2_N52Class", "View Slow", nullptr));
        execute_pulse->setText(QCoreApplication::translate("MT_2_N52Class", "Run Pulse", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MT_2_N52Class: public Ui_MT_2_N52Class {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MT_2_N52_H
