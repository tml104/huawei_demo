/********************************************************************************
** Form generated from reading UI file 'mousecontrolwidget.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MOUSECONTROLWIDGET_H
#define UI_MOUSECONTROLWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QStackedWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MouseControlWidget
{
public:
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout_3;
    QSpacerItem *verticalSpacer_2;
    QRadioButton *radioButton_Selection;
    QRadioButton *radioButton_Camera_Manipulate;
    QSpacerItem *verticalSpacer;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_4;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_2;
    QRadioButton *radioButton_Select_By_Click;
    QRadioButton *radioButton_Select_By_Rectangle;
    QRadioButton *radioButton_Select_By_Polygon;
    QFrame *line;
    QVBoxLayout *verticalLayout;
    QRadioButton *radioButton_Select_Vertices;
    QRadioButton *radioButton_Select_Edges;
    QRadioButton *radioButton_Select_Faces;
    QWidget *page_2;
    QGridLayout *gridLayout;
    QWidget *page_3;
    QVBoxLayout *verticalLayout_5;
    QButtonGroup *buttonGroup_2;
    QButtonGroup *buttonGroup_3;
    QButtonGroup *buttonGroup;

    void setupUi(QWidget *MouseControlWidget)
    {
        if (MouseControlWidget->objectName().isEmpty())
            MouseControlWidget->setObjectName(QString::fromUtf8("MouseControlWidget"));
        MouseControlWidget->resize(272, 130);
        horizontalLayout_2 = new QHBoxLayout(MouseControlWidget);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_2);

        radioButton_Selection = new QRadioButton(MouseControlWidget);
        buttonGroup_3 = new QButtonGroup(MouseControlWidget);
        buttonGroup_3->setObjectName(QString::fromUtf8("buttonGroup_3"));
        buttonGroup_3->addButton(radioButton_Selection);
        radioButton_Selection->setObjectName(QString::fromUtf8("radioButton_Selection"));
        radioButton_Selection->setChecked(false);

        verticalLayout_3->addWidget(radioButton_Selection);

        radioButton_Camera_Manipulate = new QRadioButton(MouseControlWidget);
        buttonGroup_3->addButton(radioButton_Camera_Manipulate);
        radioButton_Camera_Manipulate->setObjectName(QString::fromUtf8("radioButton_Camera_Manipulate"));
        radioButton_Camera_Manipulate->setChecked(true);

        verticalLayout_3->addWidget(radioButton_Camera_Manipulate);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer);


        horizontalLayout_2->addLayout(verticalLayout_3);

        groupBox = new QGroupBox(MouseControlWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy);
        verticalLayout_4 = new QVBoxLayout(groupBox);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        stackedWidget = new QStackedWidget(groupBox);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        horizontalLayout = new QHBoxLayout(page);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        radioButton_Select_By_Click = new QRadioButton(page);
        buttonGroup = new QButtonGroup(MouseControlWidget);
        buttonGroup->setObjectName(QString::fromUtf8("buttonGroup"));
        buttonGroup->addButton(radioButton_Select_By_Click);
        radioButton_Select_By_Click->setObjectName(QString::fromUtf8("radioButton_Select_By_Click"));
        radioButton_Select_By_Click->setChecked(true);

        verticalLayout_2->addWidget(radioButton_Select_By_Click);

        radioButton_Select_By_Rectangle = new QRadioButton(page);
        buttonGroup->addButton(radioButton_Select_By_Rectangle);
        radioButton_Select_By_Rectangle->setObjectName(QString::fromUtf8("radioButton_Select_By_Rectangle"));

        verticalLayout_2->addWidget(radioButton_Select_By_Rectangle);

        radioButton_Select_By_Polygon = new QRadioButton(page);
        buttonGroup->addButton(radioButton_Select_By_Polygon);
        radioButton_Select_By_Polygon->setObjectName(QString::fromUtf8("radioButton_Select_By_Polygon"));

        verticalLayout_2->addWidget(radioButton_Select_By_Polygon);


        horizontalLayout->addLayout(verticalLayout_2);

        line = new QFrame(page);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(line);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        radioButton_Select_Vertices = new QRadioButton(page);
        buttonGroup_2 = new QButtonGroup(MouseControlWidget);
        buttonGroup_2->setObjectName(QString::fromUtf8("buttonGroup_2"));
        buttonGroup_2->addButton(radioButton_Select_Vertices);
        radioButton_Select_Vertices->setObjectName(QString::fromUtf8("radioButton_Select_Vertices"));

        verticalLayout->addWidget(radioButton_Select_Vertices);

        radioButton_Select_Edges = new QRadioButton(page);
        buttonGroup_2->addButton(radioButton_Select_Edges);
        radioButton_Select_Edges->setObjectName(QString::fromUtf8("radioButton_Select_Edges"));
        radioButton_Select_Edges->setChecked(true);

        verticalLayout->addWidget(radioButton_Select_Edges);

        radioButton_Select_Faces = new QRadioButton(page);
        buttonGroup_2->addButton(radioButton_Select_Faces);
        radioButton_Select_Faces->setObjectName(QString::fromUtf8("radioButton_Select_Faces"));

        verticalLayout->addWidget(radioButton_Select_Faces);


        horizontalLayout->addLayout(verticalLayout);

        stackedWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        gridLayout = new QGridLayout(page_2);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        stackedWidget->addWidget(page_2);
        page_3 = new QWidget();
        page_3->setObjectName(QString::fromUtf8("page_3"));
        verticalLayout_5 = new QVBoxLayout(page_3);
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        stackedWidget->addWidget(page_3);

        verticalLayout_4->addWidget(stackedWidget);


        horizontalLayout_2->addWidget(groupBox);


        retranslateUi(MouseControlWidget);

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MouseControlWidget);
    } // setupUi

    void retranslateUi(QWidget *MouseControlWidget)
    {
        MouseControlWidget->setWindowTitle(QApplication::translate("MouseControlWidget", "MouseControlWidget", 0, QApplication::UnicodeUTF8));
        radioButton_Selection->setText(QApplication::translate("MouseControlWidget", "\351\274\240\346\240\207\351\200\211\346\213\251", 0, QApplication::UnicodeUTF8));
        radioButton_Camera_Manipulate->setText(QApplication::translate("MouseControlWidget", "\350\247\206\350\247\222\346\216\247\345\210\266", 0, QApplication::UnicodeUTF8));
        radioButton_Camera_Manipulate->setShortcut(QApplication::translate("MouseControlWidget", "Alt+V", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("MouseControlWidget", "\350\256\276\347\275\256", 0, QApplication::UnicodeUTF8));
        radioButton_Select_By_Click->setText(QApplication::translate("MouseControlWidget", "\351\274\240\346\240\207\347\202\271\351\200\211", 0, QApplication::UnicodeUTF8));
        radioButton_Select_By_Rectangle->setText(QApplication::translate("MouseControlWidget", "\351\225\277\346\226\271\345\275\242\346\241\206\351\200\211", 0, QApplication::UnicodeUTF8));
        radioButton_Select_By_Polygon->setText(QApplication::translate("MouseControlWidget", "\345\244\232\350\276\271\345\275\242\351\200\211", 0, QApplication::UnicodeUTF8));
        radioButton_Select_Vertices->setText(QApplication::translate("MouseControlWidget", "\347\202\271", 0, QApplication::UnicodeUTF8));
        radioButton_Select_Edges->setText(QApplication::translate("MouseControlWidget", "\350\276\271", 0, QApplication::UnicodeUTF8));
        radioButton_Select_Faces->setText(QApplication::translate("MouseControlWidget", "\351\235\242", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MouseControlWidget: public Ui_MouseControlWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MOUSECONTROLWIDGET_H
