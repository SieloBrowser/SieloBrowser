﻿/***********************************************************************************
** MIT License                                                                    **
**                                                                                **
** Copyright (c) 2018 Victor DENIS (victordenis01@gmail.com)                      **
**                                                                                **
** Permission is hereby granted, free of charge, to any person obtaining a copy   **
** of this software and associated documentation files (the "Software"), to deal  **
** in the Software without restriction, including without limitation the rights   **
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      **
** copies of the Software, and to permit persons to whom the Software is          **
** furnished to do so, subject to the following conditions:                       **
**                                                                                **
** The above copyright notice and this permission notice shall be included in all **
** copies or substantial portions of the Software.                                **
**                                                                                **
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     **
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       **
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    **
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         **
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  **
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  **
** SOFTWARE.                                                                      **
***********************************************************************************/

#pragma once
#ifndef SIELOBROWSER_NAVIGATIONCONTROLDIALOG_HPP
#define SIELOBROWSER_NAVIGATIONCONTROLDIALOG_HPP

#include <QDialog>

#include <QGridLayout>

#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include <QCloseEvent> 

namespace Sn
{
class NavigationControlDialog: public QDialog {
Q_OBJECT

public:
	NavigationControlDialog(QWidget* parent = nullptr);
	~NavigationControlDialog();

	void save();

protected:
	void closeEvent(QCloseEvent* event);

private slots:
	void buttonCheckChanged(bool checked);

private:
	void setupUI();

	QGridLayout* m_layout{nullptr};

	QDialogButtonBox* m_buttonBox{nullptr};

	QLabel* m_desc{nullptr};
	QLabel* m_floatingButtonImage{nullptr};
	QLabel* m_toolbarsImage{nullptr};
	QPushButton* m_floatingButton{nullptr};
	QPushButton* m_toolbars{nullptr};
};
}

#endif //SIELOBROWSER_NAVIGATIONCONTROLDIALOG_HPP
