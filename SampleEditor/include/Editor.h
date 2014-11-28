#pragma once
#include <QtGui\QWidget>
#include <AniCardLibCommonEditor.h>
#include <Preview.h>
class QListWidget;
class QComboBox;
class QSpinBox;
class Editor : public QWidget
{
	Q_OBJECT;
	QWidget* arCardsWidget , *pmdEditorWidget;
	QListWidget* arCardsList , *modelsList , *texturesList;
	AniCardLibCommonEditor* editor;
	QComboBox* frameList;
	QSpinBox* nextFrame , *startFrame , *endFrame;
	Preview preview;
	bool changing;
private slots:
void initialize();
void selectCard( int selected );
void selectModel( int selected );
void addCard();
void swapCards();
void addModel();
void linkModel();
void addTexture();
void linkTexture();
void save();
void load();

void frameIndexChanged( int index );
void addFrameRange();
void changeParamsNext();
void changeParamsStart();
void changeParamsEnd();

void changeCardMode( bool isTrue );
void changeModelMode( bool isTrue );
public:
	Editor();
	~Editor();
};