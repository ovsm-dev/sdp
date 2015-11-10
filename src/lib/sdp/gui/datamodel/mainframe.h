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



#ifndef __SDP_QT4_MAINFRAME_H__
#define __SDP_QT4_MAINFRAME_H__

#include <QWidget>
#include <QHash>
#include <QScopedPointer>
#include <sdp/gui/datamodel/singleton.h>
#include <QVector>


namespace Ui {
class MainFrame;
}


QT_FORWARD_DECLARE_CLASS(QPropertyAnimation);


namespace SDP {
namespace Qt4 {


class ParameterManager;
class FancyPanel;
class FancyButton;
class FancyHeaderFrame;
class FancyLabel;
class SettingsPanel;
class RecentPanel;
class ActivityPanel;
class ImportPanel;
class DetectionPanel;
class TriggerPanel;
class DispatchPanel;
class HelpPanel;
class PanelWidget;
class Config;
class DatabaseManager;
class Environment;
class Cache;


/**
 * @class MainFrame
 * @brief This class implements the main frame of the application. Panels,
 *        buttons, everything concerning the GUI is managed at the upper
 *        level by this class.
 */
class MainFrame : public QWidget, public Singleton<MainFrame> {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum Panel {
			pSettings, pRecent, pActivity, pTrigger, pDispatch, pDetection, pHelp
		};
		struct Session {
				Session();
				QString file;
				bool registered;
		};
		typedef QHash<Panel, PanelWidget*> PanelList;
		typedef QHash<Panel, FancyButton*> ButtonList;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit MainFrame(QWidget* = NULL);
		~MainFrame();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setActivePanel(Panel);
		const Panel& activePanel() const {
			return __activePanel;
		}
		ParameterManager* parameterManager() {
			return __parameterMgr.data();
		}
		DatabaseManager* databaseManager() {
			return __dbMgr.data();
		}
		Session& activeSession() {
			return __activeSession;
		}
		bool readConfiguration();
		Config* config() {
			return __mainConfig;
		}
		bool clearForExit();

		//! Go thru database and recovers old sessions runs and results
		void loadInventory();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		//! Should be called whenever a new job has been created
		void newJobToManage();
		//! Should be called whenever a detection script reports something
		void newTriggerToInspect();

		void newStatusMessage(const QString&);
		void resetStatusMessage();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void panelButtonClicked();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void statusMessage(const QString&);
		void setTimeWindowAccessible(bool);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QScopedPointer<Ui::MainFrame> __ui;
		QScopedPointer<ParameterManager> __parameterMgr;
		QScopedPointer<DatabaseManager> __dbMgr;
		QScopedPointer<Environment> __env;
		QScopedPointer<Cache> __cache;
		FancyButton* __configButton;
		FancyButton* __recentButton;
		FancyButton* __activityButton;
		FancyButton* __triggerButton;
		FancyButton* __dispatchButton;
		FancyButton* __detectionButton;
		FancyButton* __helpButton;
		FancyHeaderFrame* __hdr;
		FancyPanel* __statusBar;
		Config* __mainConfig;
		FancyLabel* __statusLabel;

		//! Application's panels
		SettingsPanel* __config;
		RecentPanel* __recent;
		ActivityPanel* __activity;
		TriggerPanel* __trigger;
		ImportPanel* __import;
		DispatchPanel* __dispatch;
		DetectionPanel* __detection;
		HelpPanel* __help;

		QPropertyAnimation* __inAnim;
		QPropertyAnimation* __outAnim;
		QPropertyAnimation* __bgAnim;

		ButtonList __buttons;
		PanelList __panels;
		Panel __activePanel;
		Session __activeSession;
};


} // namespace Qt4
} // namespace SDP

#endif
