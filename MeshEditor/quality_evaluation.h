#ifndef QUALITY_EVALUATION_H
#define QUALITY_EVALUATION_H

#include <QDialog>
#include "ui_quality_evaluation.h"

class quality_evaluation : public QDialog
{
	Q_OBJECT

public:
	quality_evaluation(QWidget *parent = 0);
	~quality_evaluation();

	Ui::quality_evaluation get_quality_evaluation_ui() {return ui;}
private:
	Ui::quality_evaluation ui;
};

#endif // QUALITY_EVALUATION_H
