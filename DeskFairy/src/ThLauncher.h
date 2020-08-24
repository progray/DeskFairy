#pragma once
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QDialog>
#include <QIcon>
#include <QFileIconProvider>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>

#include <string>
#include <vector>
#include <fstream>

#include "Settings.h"

class ShortcutWidget : public QWidget
{
	Q_OBJECT

public:
	friend class ShortcutEditor;

	ShortcutWidget(int x, int y, QWidget* parent = nullptr);

	~ShortcutWidget();

	void Select();

	void Unselect();

	void Edit();

	void Delete();

	void Start();

	void Update();

	void mousePressEvent(QMouseEvent* e);

	void Load(std::ifstream& in);

	void Save(std::ofstream& out) const;

	void ChangePos(int x, int y);

signals:

	void SelectedSignal(int x, int y);

	void StartedSignal();

	void EditSignal(ShortcutWidget* s);

private:
	std::string _name;
	std::string _exePath;
	std::string _iconPath;
	QLabel* _icon = nullptr;
	QLabel* _text = nullptr;
	QGroupBox* _box = nullptr;
	int _x, _y;
	bool selected = false;
};


class ShortcutEditor : public QDialog
{
	Q_OBJECT

public:

	ShortcutEditor(QWidget* parent = nullptr);

	~ShortcutEditor();

	void Edit(ShortcutWidget* s);

	void Accept();

	void Reject();

private:

	FileWidget* _exePath		= nullptr;
	FileWidget* _iconPath		= nullptr;
	QLineEdit* _name			= nullptr;
	ShortcutWidget* _shortcut	= nullptr;
};


class ThLauncher : public QDialog
{
	Q_OBJECT

public:

	ThLauncher(QWidget* parent = nullptr);

	~ThLauncher();

	void closeEvent(QCloseEvent* e) override;

signals:
	void LaunchedSignal();

private:

	ShortcutWidget* _shortcuts[5][10] = { nullptr };
	ShortcutEditor* _editor = nullptr;

	QGridLayout* _grid = nullptr;

	QPushButton* _btnNext	= nullptr;
	QPushButton* _btnLast	= nullptr;
	QPushButton* _btnDelete = nullptr;
	QPushButton* _btnEdit	= nullptr;

	int _selectX = -1;
	int _selectY = -1;

	void _OnBtnNext();

	void _OnBtnLast();

	void _OnBtnDelete();

	void _OnBtnEdit();

	void _OnSelected(int x, int y);
	
	void _Load();

	void _Save() const;

	void _Launch();

	void _Swap(int x1, int y1, int x2, int y2);
};
