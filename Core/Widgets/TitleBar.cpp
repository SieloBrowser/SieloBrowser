/***********************************************************************************
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

#include "TitleBar.hpp"

#include <QDesktopWidget>
#include <QtWidgets/QMessageBox>

#include "Application.hpp"

#include "Bookmarks/AddBookmarkDialog.hpp"
#include "Bookmarks/BookmarksToolBar.hpp"

#include "Widgets/Tab/TabWidget.hpp"

namespace Sn {
TitleBar::TitleBar(BookmarksModel* model, BrowserWindow* window, bool showBookmarks) :
		QWidget(window),
		m_window(window),
		m_showBookmarks(showBookmarks)
{
	m_bookmarksToolbar = new BookmarksToolBar(model, m_window);
	m_controlsToolbar = new QToolBar(m_window);

	m_bookmarksToolbar->setFloatable(false);
	m_bookmarksToolbar->installEventFilter(this);
	m_bookmarksToolbar->setContextMenuPolicy(Qt::CustomContextMenu);
	m_controlsToolbar->setFloatable(false);
	m_controlsToolbar->installEventFilter(this);
	m_controlsToolbar->setObjectName(QLatin1String("title-bar"));
	m_controlsToolbar->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(m_bookmarksToolbar, &BookmarksToolBar::orientationChanged, this, &TitleBar::build);
	connect(m_controlsToolbar, &QToolBar::orientationChanged, this, &TitleBar::build);

	m_window->addToolBar(m_bookmarksToolbar);
	m_window->addToolBar(m_controlsToolbar);

	build();
}

TitleBar::~TitleBar()
{
	// Empty
}

void TitleBar::setTitle(const QString& title)
{
	if (!m_title)
		return;

	m_title->setText(title);
}

void TitleBar::setShowBookmark(bool show)
{
	m_showBookmarks = show;
	build();
}

bool TitleBar::isWindowMaximized() const
{
	//return m_window->geometry() == Application::desktop()->availableGeometry(m_window);
	return m_isMaximized;
}

bool TitleBar::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::MouseButtonPress) {
		mousePressEvent(static_cast<QToolBar*>(obj), static_cast<QMouseEvent*>(event));
	}
	else if (event->type() == QEvent::MouseMove) {
		mouseMoveEvent(static_cast<QMouseEvent*>(event));
	}
	else if (event->type() == QEvent::MouseButtonDblClick) {
		mouseDoubleClickEvent(static_cast<QMouseEvent*>(event));
	}
	else if (event->type() == QEvent::ContextMenu) {
		contextMenuEvent(static_cast<QContextMenuEvent*>(event));
	}

	return QObject::eventFilter(obj, event);
}


void TitleBar::mousePressEvent(QToolBar* toolBar, QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton && ((event->x() >= 10 && toolBar->orientation() == Qt::Horizontal) ||
											  (event->y() >= 10 && toolBar->orientation() == Qt::Vertical))) {
		m_offset = event->globalPos() - m_window->frameGeometry().topLeft();
		m_canMove = true;
		event->accept();
	}
	else
		m_canMove = false;

	QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton && m_canMove) {
		if (isWindowMaximized()) {
			m_window->resize(m_geometry.size());
			m_toggleMaximize->setObjectName(QLatin1String("titlebar-button-maximize"));
			m_toggleMaximize->setIcon(Application::getAppIcon("tb-maximize", "titlebar"));
		}

		m_window->move(event->globalPos() - m_offset);
		event->accept();
	}

	QWidget::mouseMoveEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton) {
		toggleMaximize();
	}
}

void TitleBar::contextMenuEvent(QContextMenuEvent* event)
{
	QMenu menu{};
	QAction* hideShowAction{menu.addAction(tr("Show Bookmarks Bar"))};

	hideShowAction->setCheckable(true);
	hideShowAction->setChecked(m_showBookmarks);
	connect(hideShowAction, &QAction::toggled, this, [=]() {
		setShowBookmark(!m_showBookmarks);
		emit toggleBookmarksBar(m_showBookmarks);
	});

	QAction* addBookmark(menu.addAction(tr("Add Bookmark")));

	connect(addBookmark, &QAction::toggled, this, []() {
		TabWidget* tabWidget = Application::instance()->getWindow()->tabWidget();
		AddBookmarkDialog* dialog{
				new AddBookmarkDialog(tabWidget->weTab()->url().toString(), tabWidget->weTab()->title(), tabWidget,
									  Application::instance()->bookmarksManager())};
	});

	const QPoint position{event->globalPos()};
	QPoint point{position.x(), position.y() + 1};
	menu.exec(point);
}

void TitleBar::build()
{
#ifdef Q_OS_WIN
	if (m_showBookmarks)
		m_bookmarksToolbar->show();
	else
		m_bookmarksToolbar->hide();

	m_controlsToolbar->clear();

	m_title = new QLabel(m_window->windowTitle(), m_controlsToolbar);
	m_closeButton = new QToolButton(m_controlsToolbar);
	m_toggleMaximize = new QToolButton(m_controlsToolbar);
	m_minimize = new QToolButton(m_controlsToolbar);

	m_title->setObjectName(QLatin1String("titlebar-title"));

	m_closeButton->setObjectName(QLatin1String("titlebar-button-close"));
	m_closeButton->setIcon(Application::getAppIcon("tb-close", "titlebar"));

	m_toggleMaximize->setObjectName(
			QLatin1String(isWindowMaximized() ? "titlebar-button-reverse-maximize" : "titlebar-button-maximize"));
	m_toggleMaximize->setIcon(isWindowMaximized() ? Application::getAppIcon("tb-revert-maximize", "titlebar") :
							  Application::getAppIcon("tb-maximize", "titlebar"));

	m_minimize->setObjectName(QLatin1String("titlebar-button-minimize"));
	m_minimize->setIcon(Application::getAppIcon("tb-minimize", "titlebar"));

	connect(m_closeButton, &QToolButton::clicked, this, &TitleBar::closeWindow);
	connect(m_toggleMaximize, &QToolButton::clicked, this, &TitleBar::toggleMaximize);
	connect(m_minimize, &QToolButton::clicked, this, &TitleBar::minimize);

	QWidget* firstSpacer{new QWidget(m_controlsToolbar)};
	QWidget* secondSpacer{new QWidget(m_controlsToolbar)};

	if (m_controlsToolbar->orientation() == Qt::Horizontal) {
		firstSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
		secondSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	}
	else if (m_controlsToolbar->orientation() == Qt::Vertical) {
		firstSpacer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
		secondSpacer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	}

	m_controlsToolbar->addWidget(firstSpacer);
	m_controlsToolbar->addWidget(m_title);
	m_controlsToolbar->addWidget(secondSpacer);
	m_controlsToolbar->addWidget(m_minimize);
	m_controlsToolbar->addWidget(m_toggleMaximize);
	m_controlsToolbar->addWidget(m_closeButton);
#else
	if (m_showBookmarks)
		m_bookmarksToolbar->show();
	else
		m_bookmarksToolbar->hide();
#endif
}

void TitleBar::closeWindow()
{
	m_window->close();
}

void TitleBar::toggleMaximize()
{
	if (isWindowMaximized()) {
		if (m_window->isFullScreen())
			m_window->showNormal();

		m_window->setGeometry(m_geometry);
		m_isMaximized = false;

		m_toggleMaximize->setObjectName(QLatin1String("titlebar-button-maximize"));
		m_toggleMaximize->setIcon(Application::getAppIcon("tb-maximize", "titlebar"));
	}
	else {
		m_geometry = m_window->geometry();
		m_window->setGeometry(Application::desktop()->availableGeometry(m_window));
		m_isMaximized = true;

		m_window->showNormal();

		m_toggleMaximize->setObjectName(QLatin1String("titlebar-button-reverse-maximize"));
		m_toggleMaximize->setIcon(Application::getAppIcon("tb-revert-maximize", "titlebar"));
	}
}

void TitleBar::minimize()
{
	m_window->showMinimized();
}
}