#pragma once
#include <QtGui\QWidget>
#include <AniCardLibFileInfo.h>
#include <Preview.h>
class QListWidget;
class Editor : public QWidget
{
	Q_OBJECT;

	QListWidget* arCardsList , *modelsList , *texturesList;
	AniCardLibFileInfo* file;
	Preview preview;
private slots:
void initialize();
void addCard();
void addModel();
void linkModel();
void addTexture();
void linkTexture();
void save();
void load();
public:
	Editor();
};