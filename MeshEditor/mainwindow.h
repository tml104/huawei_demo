#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <map>
#include "ui_mainwindow.h"
#include "topologyoptwidget.h"
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MainWindow();
private:
	void setup_actions ();
private slots:
	void on_tab_changed (int index);
	void on_tab_close_requested (int index);
	void on_hex_topology_opt();
	void on_quad_surfaces();
private:
	Ui::MainWindowClass ui;
	std::set<QWidget*> widgets_in_tabwidget;
};

#endif // MAINWINDOW_H
