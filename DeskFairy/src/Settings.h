#pragma once
#pragma execution_character_set("utf-8")

#include <QDialog>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QFileDialog>
#include <QKeyEvent>

#include <vector>
#include <string>

#include "Logger.h"

namespace Settings
{
	extern bool firstTimeStart;

	extern int size;
	extern bool sleep;
	extern bool fallWhenSleep;
	extern std::string screenshotPath;
	extern QKeySequence screenshotFastHotKey;
	extern QKeySequence exitHotKey;
	extern QKeySequence hideHotKey;
	extern QKeySequence clearItemHotKey;
	extern bool allowItemSpawn;
	extern int itemGravity;
	extern QKeySequence screenshotHotKey;
	extern bool hideWhenScreenshot;
	extern int mainFps;
	extern int itemFps;
	extern bool expandToStateBar;
	extern QKeySequence thLauncherHotkey;
	extern QKeySequence editScreenshotHotkey;
	extern QKeySequence openScreenshotHotkey;

	void Save();
	void Load();
};


class HotkeyWidget : public QWidget
{
	Q_OBJECT
public:
	HotkeyWidget(QWidget* parent = nullptr) : QWidget(parent)
	{
		_button = new QPushButton(this);
		QHBoxLayout* layout = new QHBoxLayout;
		layout->addWidget(_button);
		layout->setMargin(0);
		this->setLayout(layout);
		connect(_button, &QPushButton::clicked, this, &HotkeyWidget::onButtonClicked);
	}
	QKeySequence key() const
	{
		return _key;
	}
	void setKey(QKeySequence key)
	{
		_key = key;
		if (_key.isEmpty())
			_button->setText("<无>");
		else
			_button->setText(("<" + key.toString() + ">").toStdString().c_str());
	}
	void onButtonClicked()
	{
		if (!_isSetting)
		{
			_button->setText("<按键设置或点击置空...>");
			_isSetting = true;
		}
		else
		{
			_button->setText("<无>");
			_key = QKeySequence();
			_isSetting = false;
		}
	}
	void keyPressEvent(QKeyEvent* e)
	{
		if (_isSetting)
		{
			if (e->key() != Qt::Key::Key_Control &&
				e->key() != Qt::Key::Key_Meta &&
				e->key() != Qt::Key::Key_Shift &&
				e->key() != Qt::Key::Key_Alt)
			{
				setKey(QKeySequence(e->modifiers() + e->key()));
				_isSetting = false;
			}
		}
	}
private:
	bool _isSetting = false;
	QPushButton* _button = nullptr;
	QKeySequence _key;
};

class FilePathWidget : public QWidget
{
	Q_OBJECT
public:
	FilePathWidget(QWidget* parent = nullptr) : QWidget(parent)
	{
		_text = new QLineEdit(this);
		_button = new QPushButton(this);
		_button->setText("*");
		QHBoxLayout* layout = new QHBoxLayout;
		layout->addWidget(_button);
		layout->addWidget(_text);
		layout->setMargin(0);
		_button->setMaximumWidth(_button->height());
		this->setLayout(layout);
		connect(_button, &QPushButton::clicked, this, &FilePathWidget::onButtonClicked);
		this->setContentsMargins(0, 0, 0, 0);
	}
	QString text() const
	{
		return _text->text();
	}
	void setText(QString text)
	{
		_text->setText(text);
	}
	void onButtonClicked()
	{
		QString s = QFileDialog::getExistingDirectory();
		if (s.length())
			setText(s);
	}
private:
	QLineEdit* _text = nullptr;
	QPushButton* _button = nullptr;
};

class FileWidget : public QWidget
{
	Q_OBJECT
public:
	FileWidget(std::string filter = "", QWidget* parent = nullptr)
		: QWidget(parent), _filter(filter)
	{
		_text = new QLineEdit(this);
		_button = new QPushButton(this);
		_button->setText("*");
		QHBoxLayout* layout = new QHBoxLayout;
		layout->addWidget(_button);
		layout->addWidget(_text);
		layout->setMargin(0);
		_button->setMaximumWidth(_button->height());
		this->setLayout(layout);
		connect(_button, &QPushButton::clicked, this, &FileWidget::onButtonClicked);
		this->setContentsMargins(0, 0, 0, 0);
	}
	QString text() const
	{
		return _text->text();
	}
	void setText(QString text)
	{
		_text->setText(text);
	}
	void onButtonClicked()
	{
		QString s = QFileDialog::getOpenFileName(nullptr, QString(), QString(), _filter.c_str());
		if (s.length())
			setText(s);
	}
private:
	QLineEdit* _text = nullptr;
	QPushButton* _button = nullptr;
	std::string _filter;
};


class SettingItem
{
public:
	SettingItem(const std::string& text)
	{
		label = new QLabel;
		label->setText((text).c_str());
	}
	virtual void Apply() { }
	QWidget* widget = nullptr;
	QLabel* label = nullptr;
};

class SettingItemBool : public SettingItem
{
public:
	SettingItemBool(
		const std::string& label,
		bool* data
	) : SettingItem(label)
	{
		_data = data;

		_ui = new QCheckBox;
		_ui->setChecked(*_data);

		widget = _ui;
	}
	void Apply() override
	{
		*_data = _ui->isChecked();
	}
private:
	bool* _data = nullptr;
	QCheckBox* _ui = nullptr;
};

class SettingItemInt : public SettingItem
{
public:
	SettingItemInt(
		const std::string& label,
		int* data,
		int min,
		int max,
		int step
	) : SettingItem(label)
	{
		_data = data;

		_ui = new QSpinBox;
		_ui->setMaximum(max);
		_ui->setMinimum(min);
		_ui->setSingleStep(step);
		_ui->setValue(*_data);

		widget = _ui;
	}
	void Apply() override
	{
		*_data = _ui->value();
	}
private:
	int* _data = nullptr;
	QSpinBox* _ui = nullptr;
};

class SettingItemString : public SettingItem
{
public:
	SettingItemString(
		const std::string& label,
		std::string* data
	) : SettingItem(label)
	{
		_data = data;

		_ui = new QLineEdit;
		_ui->setText(_data->c_str());

		widget = _ui;
	}
	void Apply() override
	{
		*_data = _ui->text().toLocal8Bit();
	}
private:
	std::string* _data = nullptr;
	QLineEdit* _ui = nullptr;
};

class SettingItemPath : public SettingItem
{
public:
	SettingItemPath(
		const std::string& label,
		std::string* data
	) : SettingItem(label)
	{
		_data = data;
		_ui = new FilePathWidget();
		_ui->setText(data->c_str());
		widget = _ui;
	}
	void Apply() override
	{
		*_data = _ui->text().toLocal8Bit();
	}
private:
	std::string* _data = nullptr;
	FilePathWidget* _ui = nullptr;
};

class SettingItemHotKey : public SettingItem
{
public:
	SettingItemHotKey(
		const std::string& label,
		QKeySequence* data
	) : SettingItem(label)
	{
		_data = data;
		_ui = new  HotkeyWidget;
		_ui->setKey(*_data);
		widget = _ui;
	}
	void Apply() override
	{
		*_data = _ui->key();
	}
private:
	QKeySequence* _data = nullptr;
	HotkeyWidget* _ui = nullptr;
};


class SettingsMenu : public QDialog
{
	Q_OBJECT

public:
	SettingsMenu(QWidget* parent = nullptr);

	~SettingsMenu();

	void Apply();

	void Cancel();

private:
	std::vector<SettingItem*> _items;
};