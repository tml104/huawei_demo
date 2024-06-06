#ifndef MAINTESTWIDGET_H
#define MAINTESTWIDGET_H

#include <QObject>

class MainTestWidget : public QObject
{
	Q_OBJECT

public:
	MainTestWidget(QObject *parent);
	~MainTestWidget();

private:
	
};

#endif // MAINTESTWIDGET_H
