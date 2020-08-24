#include "ThLauncher.h"
#include "Logger.h"
#include "Defines.h"
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QLayout>
#include <QMouseEvent>
#include <QMessageBox>
#include <QGroupBox>
#include <QProcess>
#include <codecvt>



static bool IsFile(const std::string& path)
{
	QFileInfo info(path.c_str());
	return info.isFile();
}

static std::string Utf8ToAnsi(const std::string &s)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
	std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>> 
		conv2(new std::codecvt_byname < wchar_t, char, mbstate_t>(".936"));
	return conv2.to_bytes(conv1.from_bytes(s));
}

static void CreateStartBat(const std::string& path)
{

	int i = path.size() - 1;
	while (i >= 0 && path[i] != L'/' && path[i] != L'\\')
		i--;
	std::string dir = path.substr(0, std::max(0, i));
	std::string exe = path.substr(i + 1, path.size() - i - 1);

	Logger::MakeDir(Def::thLauncherSavePath);
	std::ofstream out("./thLauncher.bat");
	out << Utf8ToAnsi("cd \"" + dir + "\"") << std::endl;
	out << Utf8ToAnsi(exe) << std::endl;
	out << Utf8ToAnsi("exit") << std::endl;
	out.close();
}



ShortcutWidget::ShortcutWidget(int x, int y, QWidget* parent)
	: QWidget(parent)
	, _x(x)
	, _y(y)
{
	_icon = new QLabel;
	_text = new QLabel;
	QVBoxLayout* layout = new QVBoxLayout;
	_icon->setMinimumSize(QSize(75, 60));
	_icon->setMaximumSize(QSize(75, 60));
	_icon->setAlignment(Qt::AlignCenter);
	_text->setMinimumSize(QSize(75, 16));
	_text->setMaximumSize(QSize(75, 32));
	_text->setAlignment(Qt::AlignCenter);
	layout->setMargin(0);
	layout->addWidget(_icon);
	layout->addWidget(_text);
	_box = new QGroupBox(this);
	_box->setStyleSheet("QGroupBox{border:1px solid rgb(200, 200, 200);}");
	_box->setLayout(layout);
	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->setMargin(0);
	mainLayout->addWidget(_box);
	this->setLayout(mainLayout);
}

ShortcutWidget::~ShortcutWidget()
{

}

void ShortcutWidget::Start()
{
	if (IsFile(_exePath))
	{
		CreateStartBat(_exePath);

		system("start thLauncher.bat");
		
		emit StartedSignal();
	}
	else if (_exePath.empty())
	{
		Edit();
	}
	else
	{
		QMessageBox::StandardButton result =  
		QMessageBox::warning(nullptr,
							"Touhou Launcher",
							"该路径已失效\n是否删除？",
							QMessageBox::Yes | QMessageBox::No,
							QMessageBox::Yes);
		switch (result)
		{
		case QMessageBox::Yes:
			Delete();
			break;
		case QMessageBox::No: default:
			break;
		}		
	}
}

void ShortcutWidget::Update()
{
	QFileInfo info(_iconPath.c_str());
	if (_iconPath.size() && info.isFile())
	{
		QFileIconProvider icon;
		_icon->setPixmap(icon.icon(info).pixmap(32, 32));
	}
	else
	{
		_icon->setPixmap(QPixmap());
	}

	_text->setText(_name.c_str());
	_text->adjustSize();
}

void ShortcutWidget::Select()
{
	if (!selected)
	{
		selected = true;
		_box->setStyleSheet("QGroupBox{border:2px solid rgb(0, 0, 230);}");
		emit SelectedSignal(_x, _y);
	}
	else
	{
		Start();
	}
}

void ShortcutWidget::Unselect()
{
	selected = false;
	_box->setStyleSheet("QGroupBox{border:1px solid rgb(200, 200, 200);}");
}

void ShortcutWidget::Edit()
{
	emit EditSignal(this);
}

void ShortcutWidget::Delete()
{
	_exePath.clear();
	_iconPath.clear();
	_name.clear();

	_icon->setPixmap(QPixmap());
	_text->setText("");
}

void ShortcutWidget::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::MouseButton::LeftButton)
	{
		Select();
	}
	if (e->button() == Qt::MouseButton::RightButton)
	{
		Edit();
	}
}

void ShortcutWidget::Load(std::ifstream& in)
{
	std::getline(in, _exePath);
	if (_exePath == "None" || !IsFile(_exePath))
	{
		return Delete();
	}
	std::getline(in, _iconPath);
	if (!IsFile(_iconPath))
	{
		return Delete();
	}	
	std::getline(in, _name);
	
	Update();
}

void ShortcutWidget::Save(std::ofstream& out) const
{
	if (_exePath.empty())
	{
		out << "None" << std::endl;
		return;
	}

	out << _exePath << std::endl;
	out << _iconPath << std::endl;
	out << _name << std::endl;
}

void ShortcutWidget::ChangePos(int x, int y)
{
	_x = x, _y = y;
}



ShortcutEditor::ShortcutEditor(QWidget* parent)
	: QDialog(parent)
{
	this->setWindowTitle("ThLauncher->Edit");
	this->setAttribute(Qt::WA_DeleteOnClose, false);

	_exePath = new FileWidget("可执行文件(*.exe)");
	_iconPath = new FileWidget;
	_name = new QLineEdit;

	QLabel* label1 = new QLabel("EXE文件路径：");
	label1->setMinimumWidth(200);
	_exePath->setMinimumWidth(300);
	QHBoxLayout* lay1 = new QHBoxLayout;
	lay1->addWidget(label1);
	lay1->addWidget(_exePath);

	QLabel* label2 = new QLabel("图标文件路径：");
	label2->setMinimumWidth(200);
	_iconPath->setMinimumWidth(300);
	QHBoxLayout* lay2 = new QHBoxLayout;
	lay2->addWidget(label2);
	lay2->addWidget(_iconPath);

	QLabel* label3 = new QLabel("描述：");
	label3->setMinimumWidth(200);
	_name->setMinimumWidth(300);
	QHBoxLayout* lay3 = new QHBoxLayout;
	lay3->addWidget(label3);
	lay3->addWidget(_name);


	QPushButton* btnReject = new QPushButton("取消");
	connect(btnReject, &QPushButton::clicked, this, &ShortcutEditor::Reject);
	QPushButton* btnAccept = new QPushButton("确认");
	connect(btnAccept, &QPushButton::clicked, this, &ShortcutEditor::Accept);
	QHBoxLayout* lay4 = new QHBoxLayout;
	lay4->addWidget(btnReject);
	lay4->addWidget(btnAccept);
	
	QLabel* title = new QLabel("编辑");
	title->setAlignment(Qt::AlignCenter);
	
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(title);
	lay->addLayout(lay1);
	lay->addLayout(lay2);
	lay->addLayout(lay3);
	lay->addLayout(lay4);
	this->setLayout(lay);
}

ShortcutEditor::~ShortcutEditor()
{

}

void ShortcutEditor::Edit(ShortcutWidget* s)
{
	_exePath->setText(s->_exePath.c_str());
	_iconPath->setText(s->_iconPath.c_str());
	_name->setText(s->_name.c_str());

	_shortcut = s;

	this->exec();
}

void ShortcutEditor::Accept()
{
	_shortcut->_exePath = _exePath->text().toStdString();
	_shortcut->_iconPath = _iconPath->text().toStdString();
	_shortcut->_name = _name->text().toStdString();
	_shortcut->Update();
	_shortcut = nullptr;

	this->close();
}

void ShortcutEditor::Reject()
{
	_shortcut = nullptr;
	this->close();
}


ThLauncher::ThLauncher(QWidget* parent)
	: QDialog(parent)
{
	this->setWindowTitle("DeskFairy->Touhou Launcher");
	_editor = new ShortcutEditor(this);

	QScrollArea* scrollArea = new QScrollArea;
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QWidget* shortcutsWidget = new QWidget;
	_grid = new QGridLayout;

	for (int x = 0; x < 5; x++)
		for (int y = 0; y < 10; y++)
		{
			_shortcuts[x][y] = new ShortcutWidget(x, y);
			connect(_shortcuts[x][y], &ShortcutWidget::SelectedSignal, this, &ThLauncher::_OnSelected);
			connect(_shortcuts[x][y], &ShortcutWidget::StartedSignal, this, &ThLauncher::_Launch);
			connect(_shortcuts[x][y], &ShortcutWidget::EditSignal, _editor, &ShortcutEditor::Edit);
			_grid->addWidget(_shortcuts[x][y], y, x);
		}	
	shortcutsWidget->setLayout(_grid);
	scrollArea->setWidget(shortcutsWidget);
	scrollArea->setMinimumSize(QSize(450, 370));
	scrollArea->setAlignment(Qt::AlignCenter);

	QGroupBox* btnBox = new QGroupBox;
	QHBoxLayout* btnLayout = new QHBoxLayout;
	
	btnLayout->setAlignment(Qt::AlignRight);

	_btnEdit = new QPushButton("*");
	_btnEdit->setMaximumSize(QSize(35, 35));
	connect(_btnEdit, &QPushButton::clicked, this, &ThLauncher::_OnBtnEdit);
	btnLayout->addWidget(_btnEdit);

	_btnDelete = new QPushButton("-");
	_btnDelete->setMaximumSize(QSize(35, 35));
	connect(_btnDelete, &QPushButton::clicked, this, &ThLauncher::_OnBtnDelete);
	btnLayout->addWidget(_btnDelete);

	_btnLast = new QPushButton("<");
	_btnLast->setMaximumSize(QSize(35, 35));
	connect(_btnLast, &QPushButton::clicked, this, &ThLauncher::_OnBtnLast);
	btnLayout->addWidget(_btnLast);

	_btnNext = new QPushButton(">");
	_btnNext->setMaximumSize(QSize(35, 35));
	connect(_btnNext, &QPushButton::clicked, this, &ThLauncher::_OnBtnNext);
	btnLayout->addWidget(_btnNext);

	btnBox->setLayout(btnLayout);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(btnBox);
	scrollArea->adjustSize();
	mainLayout->addWidget(scrollArea);
	this->setLayout(mainLayout);

	_Load();

	this->setAttribute(Qt::WA_DeleteOnClose, false);
	this->show();
}

ThLauncher::~ThLauncher()
{
	_Save();
}

void ThLauncher::closeEvent(QCloseEvent* e)
{
	emit LaunchedSignal();
}

void ThLauncher::_OnBtnNext()
{
	if (_selectX == -1)
		return;
	if (_selectX == 4)
	{
		if (_selectY != 9)
		{			
			_Swap(_selectX, _selectY, 0, _selectY + 1);		
			_selectX = 0;
			_selectY++;
		}
	}
	else
	{	
		_Swap(_selectX, _selectY, _selectX + 1, _selectY);
		_selectX++;
	}
}

void ThLauncher::_OnBtnLast()
{
	if (_selectX == -1)
		return;
	if (_selectX == 0)
	{
		if (_selectY != 0)
		{
			_Swap(_selectX, _selectY, 4, _selectY - 1);
			_selectX = 4;
			_selectY--;
		}
	}
	else
	{
		_Swap(_selectX, _selectY, _selectX - 1, _selectY);
		_selectX--;
	}
}

void ThLauncher::_OnBtnDelete()
{
	if (_selectX == -1)
		return;
	_shortcuts[_selectX][_selectY]->Delete();
}

void ThLauncher::_OnBtnEdit()
{
	if (_selectX == -1)
		return;
	_shortcuts[_selectX][_selectY]->Edit();
}

void ThLauncher::_OnSelected(int x, int y)
{
	if (_selectX != -1)
	{
		_shortcuts[_selectX][_selectY]->Unselect();
	}
	_selectX = x;
	_selectY = y;
}

void ThLauncher::_Load()
{
	std::ifstream in(std::string(Def::thLauncherSavePath) + "/thLauncher.txt");
	if (!in)
	{
		return;
	}

	for(int y = 0; y < 10 ;y++)
		for (int x = 0; x < 5; x++)
		{
			_shortcuts[x][y]->Load(in);
		}

	in.close();
}

void ThLauncher::_Save() const
{
	Logger::MakeDir(Def::thLauncherSavePath);
	std::ofstream out(std::string(Def::thLauncherSavePath) + "/thLauncher.txt");


	for (int y = 0; y < 10; y++)
		for (int x = 0; x < 5; x++)
		{
			_shortcuts[x][y]->Save(out);
		}

	out.close();
}

void ThLauncher::_Launch()
{
	this->close();
}

void ThLauncher::_Swap(int x1, int y1, int x2, int y2)
{
	_grid->removeWidget(_shortcuts[x1][y1]);
	_grid->removeWidget(_shortcuts[x2][y2]);
	_grid->addWidget(_shortcuts[x1][y1], y2, x2);
	_grid->addWidget(_shortcuts[x2][y2], y1, x1);

	_shortcuts[x1][y1]->ChangePos(x2, y2);
	_shortcuts[x2][y2]->ChangePos(x1, y1);

	auto ptr = _shortcuts[x1][y1];
	_shortcuts[x1][y1] = _shortcuts[x2][y2];
	_shortcuts[x2][y2] = ptr;
}