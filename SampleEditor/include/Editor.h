#pragma once
#include <QtGui\QWidget>
#include <AniCardLibCommonEditor.h>
#include <Preview.h>
class QListWidget;
class Editor : public QWidget
{
	Q_OBJECT;

	QListWidget* arCardsList , *modelsList , *texturesList;
	AniCardLibCommonEditor* editor;
	Preview preview;
private slots:
void initialize();
void selectCard( int selected );
void addCard();
void swapCards();
void addModel();
void linkModel();
void addTexture();
void linkTexture();
void save();
void load();

public:
	Editor();
	~Editor();
};