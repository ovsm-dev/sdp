/**************************************************************************
 *                                                                        *
 *  Copyright (C) 2015 OVSM/IPGP                                          *
 *                                                                        *
 *  This file is part of Seismic Data Playback 'SDP'.                     *
 *                                                                        *
 *  SDP is free software: you can redistribute it and/or modify           *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *                                                                        *
 *  SDP is distributed in the hope that it will be useful,                *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with SDP. If not, see <http://www.gnu.org/licenses/>.           *
 *                                                                        *
 **************************************************************************/


#ifndef __SDP_QT4_MAINWINDOW_H__
#define __SDP_QT4_MAINWINDOW_H__

#include <QMainWindow>
#include <QSettings>
#include <QScopedPointer>
#include <stdlib.h>


namespace Ui {
class SeismicDataPlayback;
}


QT_FORWARD_DECLARE_CLASS(QAction);
QT_FORWARD_DECLARE_CLASS(QMenu);
QT_FORWARD_DECLARE_CLASS(QSystemTrayIcon);


namespace SDP {
namespace Qt4 {

class MainFrame;

/**
 * @class SeismicDataPlayback
 * @brief This implements the main window of the application...
 */
class SeismicDataPlayback : public QMainWindow {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit SeismicDataPlayback(QWidget* = NULL);
		~SeismicDataPlayback();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void showEvent(QShowEvent*);
		void closeEvent(QCloseEvent*);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void about();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QScopedPointer<Ui::SeismicDataPlayback> __ui;
		QScopedPointer<MainFrame> __mainFrame;

		QAction* __minimizeAction;
		QAction* __maximizeAction;
		QAction* __restoreAction;
		QAction* __quitAction;

	    QMenu* __trayIconMenu;
	    QSystemTrayIcon* __trayIcon;

		QSettings __settings;
};

} // namespace Qt4
} // namespace SDP

#endif
