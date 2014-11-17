#pragma once
#include <Core\CommonWarningDisables.h>
#include <QtGui\QWidget>
#include <WebCamSource.h>
#include <WebCamHelpers.h>
class QComboBox;
class Setup : public QWidget
{
	Q_OBJECT;

	WebCamSource* source;
	std::vector<CameraItem> cameras;
	QComboBox* cameraSelection;
	QComboBox* resolutionSelection;
private slots:
void selectedCamera(int selection);
void run();
protected:

public:
	Setup();
	~Setup();
};