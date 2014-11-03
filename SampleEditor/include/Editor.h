#pragma once
#include <QtGui\QWidget>
class QListView;
class Editor : public QWidget
{
	Q_OBJECT;

	QListView* arCardsList, *modelsList, *texturesList;
private slots:
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