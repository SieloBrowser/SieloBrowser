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
#include <QScreen>

#include "Bookmarks/BookmarksUtils.hpp"
#include "Bookmarks/BookmarksToolbar.hpp"

#include "Utils/Settings.hpp"

#include "Widgets/NavigationBar.hpp"
#include "Widgets/Tab/TabWidget.hpp"

#include "Application.hpp"
#include "BrowserWindow.hpp"

namespace Sn {
TitleBar::TitleBar(BrowserWindow* window, bool showBookmarks) :
		QWidget(window),
		m_window(window),
		m_showBookmarks(showBookmarks)
{
	setupUI();

	connect(m_bookmarksToolbar, &BookmarksToolbar::orientationChanged, this, &TitleBar::build);

	if (Application::instance()->hideBookmarksHistoryActions())
		m_navigationToolBar->hideBookmarksHistory();
	else
		m_navigationToolBar->showBookmarksHistory();

	if (m_showBookmarks && m_show)
		m_bookmarksToolbar->show();
	else
		m_bookmarksToolbar->hide();

	m_window->setCaption(m_moveControlWidget);
}

void TitleBar::setShowBookmark(bool show)
{
	m_showBookmarks = show;
	if (m_showBookmarks && m_show)
		m_bookmarksToolbar->show();
	else
		m_bookmarksToolbar->hide();
}

void TitleBar::hide()
{
	setView(false);
}

void TitleBar::show()
{
	setView(true);
}

bool TitleBar::isView()
{
	return m_show;
}

void TitleBar::setView(bool view)
{
	m_show = view;
	if (m_showBookmarks && m_show)
		m_bookmarksToolbar->show();
	else
		m_bookmarksToolbar->hide();
}

bool TitleBar::isWindowMaximized() const
{
	//return m_window->geometry() == Application::desktop()->availableGeometry(m_window);
	return m_isMaximized;
}

bool TitleBar::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::ContextMenu) {
		contextMenuEvent(obj, static_cast<QContextMenuEvent*>(event));
	}
	return QObject::eventFilter(obj, event);
}

void TitleBar::contextMenuEvent(QObject* obj, QContextMenuEvent* event)
{
	QMenu menu{};
	QAction* hideShowAction{menu.addAction(tr("Show Bookmarks Bar"))};

	hideShowAction->setCheckable(true);
	hideShowAction->setChecked(m_showBookmarks);
	connect(hideShowAction, &QAction::toggled, this, [=]() {
		setShowBookmark(!m_showBookmarks);
		emit toggleBookmarksBar(m_showBookmarks);
	});

	if (obj != nullptr) {
		QToolBar* toolbar = qobject_cast<QToolBar*>(obj);
		
		if (toolbar->objectName() == "bookmarks-toolbar") {
			menu.addSeparator();
			m_bookmarksToolbar->createContextMenu(menu, event->pos());
		}
	}

	const QPoint position{ event->globalPos() };
	QPoint point{ position.x(), position.y() + 1 };
	menu.exec(point);

	m_bookmarksToolbar->contextMenuCreated();
}

void TitleBar::build()
{
	Application::instance()->saveSession();
}

void TitleBar::closeWindow()
{
	m_window->close();
}

void TitleBar::toggleMaximize(bool forceMaximize)
{
	if (isWindowMaximized() && !forceMaximize) {
		if (m_window->isFullScreen())
			m_window->showNormal();

		m_window->setGeometry(m_geometry);
		m_isMaximized = false;

#ifdef Q_OS_WIN
		m_toggleMaximize->setObjectName(QLatin1String("titlebar-button-maximize"));
		m_toggleMaximize->setIcon(Application::getAppIcon("tb-maximize", "titlebar"));
#endif // Q_OS_WIN
	}
	else {
		if (!m_isOnSide)
			m_geometry = m_window->geometry();

		m_window->setGeometry(Application::desktop()->availableGeometry(m_window));
		m_isMaximized = true;

		m_window->showNormal();

#ifdef Q_OS_WIN
		m_toggleMaximize->setObjectName(QLatin1String("titlebar-button-reverse-maximize"));
		m_toggleMaximize->setIcon(Application::getAppIcon("tb-revert-maximize", "titlebar"));
#endif // Q_OS_WIN
	}
}

void TitleBar::minimize()
{
	m_window->showMinimized();
}

void TitleBar::setupUI()
{
	m_layout = new QHBoxLayout(this);
	m_layout->setSpacing(0);
	m_layout->setContentsMargins(0, 0, 0, 0);

	m_bookmarksToolbar = new BookmarksToolbar(m_window, m_window);
	m_bookmarksToolbar->setFloatable(false);
	m_bookmarksToolbar->installEventFilter(this);
	m_bookmarksToolbar->setContextMenuPolicy(Qt::CustomContextMenu);

	m_addressBars = new QStackedWidget(this);
	m_navigationToolBar = new NavigationToolBar(m_window, m_addressBars);
	m_navigationToolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	m_moveControlWidget = new QWidget(this);
	m_moveControlWidget->setObjectName("titlebar-movecontrol");
	m_moveControlWidget->setFixedWidth(64);

	m_layout->addWidget(m_bookmarksToolbar);
	m_layout->addWidget(m_navigationToolBar);
	m_layout->addWidget(m_moveControlWidget);

#ifdef Q_OS_WIN
	m_closeButton = new QToolButton(this);
	m_toggleMaximize = new QToolButton(this);
	m_minimize = new QToolButton(this);

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

	m_layout->addWidget(m_minimize);
	m_layout->addWidget(m_toggleMaximize);
	m_layout->addWidget(m_closeButton);
#endif
}
}
