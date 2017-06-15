/********************************************************************************
** Form generated from reading UI file 'labelv2.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LABELV2_H
#define UI_LABELV2_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LabelV2Class
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *LabelV2Class)
    {
        if (LabelV2Class->objectName().isEmpty())
            LabelV2Class->setObjectName(QStringLiteral("LabelV2Class"));
        LabelV2Class->resize(600, 400);
        menuBar = new QMenuBar(LabelV2Class);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        LabelV2Class->setMenuBar(menuBar);
        mainToolBar = new QToolBar(LabelV2Class);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        LabelV2Class->addToolBar(mainToolBar);
        centralWidget = new QWidget(LabelV2Class);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        LabelV2Class->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(LabelV2Class);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        LabelV2Class->setStatusBar(statusBar);

        retranslateUi(LabelV2Class);

        QMetaObject::connectSlotsByName(LabelV2Class);
    } // setupUi

    void retranslateUi(QMainWindow *LabelV2Class)
    {
        LabelV2Class->setWindowTitle(QApplication::translate("LabelV2Class", "LabelV2", 0));
    } // retranslateUi

};

namespace Ui {
    class LabelV2Class: public Ui_LabelV2Class {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LABELV2_H
