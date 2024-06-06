/********************************************************************************
** Form generated from reading UI file 'topologyoptwidget.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TOPOLOGYOPTWIDGET_H
#define UI_TOPOLOGYOPTWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QToolBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <hoopsview.h>

QT_BEGIN_NAMESPACE

class Ui_TopologyOptWidget
{
public:
    QHBoxLayout *horizontalLayout;
    HoopsView *hoopsview;
    QVBoxLayout *verticalLayout_3;
    QToolBox *toolBox;
    QWidget *page_7;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_7;
    QSpinBox *spinBox_Cube_Length;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_9;
    QDoubleSpinBox *doubleSpinBox_density;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *pushButton_Start_Generation;
    QSpacerItem *verticalSpacer_3;

    void setupUi(QWidget *TopologyOptWidget)
    {
        if (TopologyOptWidget->objectName().isEmpty())
            TopologyOptWidget->setObjectName(QString::fromUtf8("TopologyOptWidget"));
        TopologyOptWidget->setWindowModality(Qt::NonModal);
        TopologyOptWidget->resize(898, 606);
        horizontalLayout = new QHBoxLayout(TopologyOptWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        hoopsview = new HoopsView(TopologyOptWidget);
        hoopsview->setObjectName(QString::fromUtf8("hoopsview"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(hoopsview->sizePolicy().hasHeightForWidth());
        hoopsview->setSizePolicy(sizePolicy);
        verticalLayout_3 = new QVBoxLayout(hoopsview);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        toolBox = new QToolBox(hoopsview);
        toolBox->setObjectName(QString::fromUtf8("toolBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(toolBox->sizePolicy().hasHeightForWidth());
        toolBox->setSizePolicy(sizePolicy1);
        toolBox->setMaximumSize(QSize(120, 16777215));
        toolBox->setCursor(QCursor(Qt::ArrowCursor));
        toolBox->setAcceptDrops(true);
        page_7 = new QWidget();
        page_7->setObjectName(QString::fromUtf8("page_7"));
        page_7->setGeometry(QRect(0, 0, 120, 544));
        verticalLayout = new QVBoxLayout(page_7);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_7 = new QLabel(page_7);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_2->addWidget(label_7);

        spinBox_Cube_Length = new QSpinBox(page_7);
        spinBox_Cube_Length->setObjectName(QString::fromUtf8("spinBox_Cube_Length"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(spinBox_Cube_Length->sizePolicy().hasHeightForWidth());
        spinBox_Cube_Length->setSizePolicy(sizePolicy2);
        spinBox_Cube_Length->setLayoutDirection(Qt::LeftToRight);
        spinBox_Cube_Length->setValue(10);

        horizontalLayout_2->addWidget(spinBox_Cube_Length);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_9 = new QLabel(page_7);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        horizontalLayout_3->addWidget(label_9);

        doubleSpinBox_density = new QDoubleSpinBox(page_7);
        doubleSpinBox_density->setObjectName(QString::fromUtf8("doubleSpinBox_density"));
        sizePolicy2.setHeightForWidth(doubleSpinBox_density->sizePolicy().hasHeightForWidth());
        doubleSpinBox_density->setSizePolicy(sizePolicy2);
        doubleSpinBox_density->setLayoutDirection(Qt::LeftToRight);

        horizontalLayout_3->addWidget(doubleSpinBox_density);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        pushButton_Start_Generation = new QPushButton(page_7);
        pushButton_Start_Generation->setObjectName(QString::fromUtf8("pushButton_Start_Generation"));
        QSizePolicy sizePolicy3(QSizePolicy::Ignored, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(pushButton_Start_Generation->sizePolicy().hasHeightForWidth());
        pushButton_Start_Generation->setSizePolicy(sizePolicy3);
        pushButton_Start_Generation->setAcceptDrops(true);
        pushButton_Start_Generation->setAutoFillBackground(true);

        horizontalLayout_4->addWidget(pushButton_Start_Generation);


        verticalLayout->addLayout(horizontalLayout_4);

        verticalSpacer_3 = new QSpacerItem(20, 476, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_3);

        toolBox->addItem(page_7, QString::fromUtf8("\345\271\263\346\235\277\345\276\256\347\273\223\346\236\204"));

        verticalLayout_3->addWidget(toolBox);


        horizontalLayout->addWidget(hoopsview);


        retranslateUi(TopologyOptWidget);

        toolBox->setCurrentIndex(0);
        toolBox->layout()->setSpacing(6);


        QMetaObject::connectSlotsByName(TopologyOptWidget);
    } // setupUi

    void retranslateUi(QWidget *TopologyOptWidget)
    {
        TopologyOptWidget->setWindowTitle(QApplication::translate("TopologyOptWidget", "TopologyOptWidget", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("TopologyOptWidget", "\345\244\226\345\275\242\345\260\272\345\257\270", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("TopologyOptWidget", "\347\233\270\345\257\271\345\257\206\345\272\246", 0, QApplication::UnicodeUTF8));
        pushButton_Start_Generation->setText(QApplication::translate("TopologyOptWidget", "\345\274\200\345\247\213\347\224\237\346\210\220", 0, QApplication::UnicodeUTF8));
        toolBox->setItemText(toolBox->indexOf(page_7), QApplication::translate("TopologyOptWidget", "\345\271\263\346\235\277\345\276\256\347\273\223\346\236\204", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TopologyOptWidget: public Ui_TopologyOptWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TOPOLOGYOPTWIDGET_H
