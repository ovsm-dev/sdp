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



#ifndef __SDP_QT4_PANELS_H__
#define __SDP_QT4_PANELS_H__


#include <QWidget>
#include <QString>
#include <QDialog>
#include <QDateTime>
#include <QQueue>
#include <QPair>
#include <QMap>
#include <QItemSelection>
#include <QScopedPointer>
#include <QBasicTimer>
#include <QTableWidgetItem>
#include <QTreeWidgetItem>
#include <sdp/gui/datamodel/singleton.h>


namespace Ui {

class Enqueue;
class CommitDialog;
class SettingsPanel;
class RecentPanel;
class ActivityPanel;
class ImportPanel;
class TriggerPanel;
class DetectionPanel;
class DispatchPanel;

}


QT_FORWARD_DECLARE_CLASS(QTimer);
QT_FORWARD_DECLARE_CLASS(QWebView);
QT_FORWARD_DECLARE_CLASS(QAction);

class QRoundProgressBar;

namespace SDP {
namespace Qt4 {

class FancyButton;
class Job;
class DetectionJob;
class DispatchJob;
class Trigger;

class DataSourceSubPanel;
class InventorySubPanel;
class TimeWindowSubPanel;
class TriggerSubPanel;
class StreamSubPanel;
class SubPanelWidget;
typedef QList<SubPanelWidget*> SubPanelList;

typedef QPair<int, QAction*> PairedAction;
typedef QList<PairedAction> HeaderActions;

class TableItem;
struct TriggerItem {
		TableItem* item;
		Trigger* trigger;
};
typedef QList<TriggerItem> TriggerItems;

struct JobItem {
		TableItem* item;
		Job* job;
};
typedef QList<JobItem> JobItems;

struct RecentItem {
		Job* job;
		QTreeWidgetItem* item;
};
typedef QList<RecentItem> RecentItems;

enum TriggerMessage {
	tmAccepted, tmRejected, tmCommitted, tmAwaiting, tmDeleted
};

/**
 * @class PanelWidget
 * @brief Macro for any widget that's suppose to be a panel inside
 *        the MainFrame
 **/
class PanelWidget : public QWidget {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit PanelWidget(QWidget* parent = NULL, Qt::WFlags f = 0) :
				QWidget(parent, f) {}
		virtual ~PanelWidget() {}

	public:
		// ------------------------------------------------------------------
		//  Public virtual interface
		// ------------------------------------------------------------------
		virtual bool saveParameters()=0;

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setHeader(const QString& s) {
			__hdr = s;
		}
		const QString& header() const {
			return __hdr;
		}
		void setDescription(const QString& s) {
			__desc = s;
		}
		const QString& description() const {
			return __desc;
		}

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QString __hdr;
		QString __desc;

};
typedef QList<PanelWidget*> PanelList;


class SettingsPanel : public PanelWidget {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit SettingsPanel(QWidget* = NULL);
		~SettingsPanel();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void init();
		void timerEvent(QTimerEvent*);

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		bool saveParameters();
		bool loadConfiguration();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void saveSettings();
		void tableSettingsEdited(QTableWidgetItem*);
		void procTerminated(int);
		void buttonToggled(int);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QScopedPointer<Ui::SettingsPanel> __ui;
		QRoundProgressBar* __diskSpace;
		QRoundProgressBar* __runDirSize;
		QBasicTimer __timer;

		struct EntityVariable {
				enum evType {
					evSTRING, evPATH, evBIN
				};
				EntityVariable(QString n, evType t, QString v, QString c,
				               QString z) :
						name(n), type(t), value(v), config(c), tooltip(z),
						checked(false) {}
				QString name;
				evType type;
				QString value;
				QString config;
				QString tooltip;
				bool checked;
		};
		typedef QList<EntityVariable> EVList;
		EVList __vars;

		struct ProcessCheck {
				ProcessCheck(QString n, QString v, QString c) :
						name(n), variable(v), command(c) {}
				QString name;
				QString variable;
				QString command;
		};
		typedef QList<ProcessCheck> PCList;
		PCList __procs;
};

/**
 * @brief A custom implementation of QTreeWidget supporting some key events
 *        making users interactions more easy breezy.
 */
class InteractiveTree : public QTreeWidget {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit InteractiveTree(QWidget* parent = NULL);
		~InteractiveTree();

	protected:
		// ------------------------------------------------------------------
		//  Protected Qt interface
		// ------------------------------------------------------------------
		void keyPressEvent(QKeyEvent*);

	Q_SIGNALS:
		void deleteSelected();
};


/**
 * @brief A custom implementation of QTableWidget supporting some key events
 *        making users interactions more easy breezy.
 */
class InteractiveTable : public QTableWidget {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit InteractiveTable(QWidget* parent = NULL);
		~InteractiveTable();

	protected:
		// ------------------------------------------------------------------
		//  Protected Qt interace
		// ------------------------------------------------------------------
		void keyPressEvent(QKeyEvent*);

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void deleteSelected();
};


class ArchivedRun;
class ArchivedOrigin;
typedef QList<ArchivedRun*> RunList;
class RecentPanel : public PanelWidget, public Singleton<RecentPanel> {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit RecentPanel(QWidget* = NULL);
		~RecentPanel();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		bool saveParameters();
		void loadJob(Job*);
		void addTriggerRow(Job*, Trigger*);
		void loadJobFromRunDir(Job*);
		void removeJob(Job*);

		/**
		 * @brief Removes jobs from panel without deleting objects from cache
		 * @param jobs the job list
		 */
		void removeJobs(QList<QString>);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void reprocessJobs();
		void removeJobs();

		void triggerStatusModified(int, QList<QString>);
		void addDetectionTriggers(QString, QList<QString>);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void headerMenu(const QPoint&);
		void showHideHeaderItems();
		void objectSelectionChanged();
		void objectSelected(QTreeWidgetItem*, const int&);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		void initInteractiveTree();
		RecentItems treeSelection();
		RecentItems jobs();
		void displayJobOutput(Job*);
		void displayTriggerInformation(Trigger*);
		void displayOriginOutput(ArchivedOrigin*);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QScopedPointer<Ui::RecentPanel> __ui;
		FancyButton* __reprocessButton;
		FancyButton* __removeButton;
		Job* __jobSelected;
		InteractiveTree* __tree;
		RunList __runs;
		HeaderActions __actions;
		struct Family {
				Family(QTreeWidgetItem* o1 = NULL, QTreeWidgetItem* o2 = NULL,
				       QTreeWidgetItem* o3 = NULL, QTreeWidgetItem* o4 = NULL,
				       QTreeWidgetItem* o5 = NULL, QTreeWidgetItem* o6 = NULL) :
						object(o1), parentItem(o2), acceptedItem(o3), rejectedItem(o4),
						committedItem(o5), awaitingItem(o6) {}
				QTreeWidgetItem* object;
				QTreeWidgetItem* parentItem;
				QTreeWidgetItem* acceptedItem;
				QTreeWidgetItem* rejectedItem;
				QTreeWidgetItem* committedItem;
				QTreeWidgetItem* awaitingItem;
		};
		QMap<QString, Family> __parents;
};


class ActivityPanel : public PanelWidget, public Singleton<ActivityPanel> {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum QueueStatus {
			Idle, RunningAll, RunningSelected
		};
		typedef QQueue<Job*> JobQueue;

		enum JobQueueStatus {
			jqsScheduled, jqsRunning, jqsSkipped_DiskError,
			jqsSkipped_RunDirError, jqsCustom, jqsNone
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit ActivityPanel(QWidget* = NULL);
		~ActivityPanel();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void addJob(Job*);
		void removeJob(Job*);
		const QueueStatus& status() const {
			return __status;
		}
		JobQueue jobs() {
			return __queue;
		}
		bool saveParameters();
		quint32 remainingJobs();
		bool autoArchiveJobs();

		/**
		 * @brief Removes jobs from panel without deleting objects from cache
		 * @param jobs the job list
		 */
		void removeJobs(QList<QString>);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		void initInteractiveTable();
		JobItems selectedJobs();
		JobItems allJobs();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void commitJobsIntoDB();
		//! Moves terminated jobs into the RecentPanel
		void archiveJobs();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void headerMenu(const QPoint&);
		void showHideHeaderItems();

		//! Starts the queue by launching the timer
		void startQueue();
		//! Stops the queue by stopping the timer
		void stopQueue();
		//! Kicks jobs out of the queue
		void removeJobs();
		//! Resets jobs to their initial state
		void resetJobs();

		//! Called by the timer when pending jobs have to be launched
		void startJobs();

		//! Bunch of slots in response of signals fired by jobs
		void jobStarted();
		void jobTerminated();
		void incomingStdMsg();

		void jobSelectionChanged(const QItemSelection&, const QItemSelection&);
		void showJobOutput(const int&, const int&);

		void editJob();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void detectionJobTerminated(DetectionJob*);
		void jobReseted(int, QList<QString>);
		void editDetectionJob(DetectionJob*);
		void editDispatchJob(DispatchJob*);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		Job* selectedJob();
		void displayJobOutput(Job*);
		void updateJobStatus(Job*, JobQueueStatus = jqsNone, const QString& = QString());

		//! Once a detection job is finished, checks if there are some triggers
		//! resulting from it.
		void checkDetectionOutput(DetectionJob*);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QScopedPointer<Ui::ActivityPanel> __ui;
		InteractiveTable* __table;
		FancyButton* __runButton;
		FancyButton* __stopButton;
		FancyButton* __removeButton;
		FancyButton* __resetButton;
		FancyButton* __editButton;
		FancyButton* __archiveButton;
		Job* __jobSelected;
		QTimer* __timer;
		qint32 __remainingSlots;
		JobQueue __queue;
		JobQueue __selectedQueue;
		QueueStatus __status;
		HeaderActions __actions;
};


class CommitDialog : public QDialog {

	Q_OBJECT

	public:
		enum ReturnCode {
			cdSUCCESS, cdFAILED, cdUNKNOWN
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit CommitDialog(const QString&, QWidget* = NULL);
		~CommitDialog();

	public:
		ReturnCode commitReturnCode();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void processTerminated(int);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QScopedPointer<Ui::CommitDialog> __ui;
		ReturnCode __retCode;
};


class TriggerPanel : public PanelWidget, public Singleton<TriggerPanel> {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit TriggerPanel(QWidget* = NULL);
		~TriggerPanel();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		bool saveParameters();
		void loadTrigger(Trigger*);
		void removeTriggers(const QString& jobID);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		void initInteractiveTable();
		void displayTriggerOutput(Trigger*);
		const QString generateSC3ML(Trigger*);
		void updateButtons();
		TriggerItems tableSelection();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void commitTriggersIntoDB();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void headerMenu(const QPoint&);
		void showHideHeaderItems();
		void createTrigger(DetectionJob*);
		void removeTrigger(Trigger*);
		void rejectTriggers();
		void acceptTriggers();
		void removeTriggers();
		void commitTriggers();
		void resetTriggers();
		void autocleanTriggers();

		void triggerSelectionChanged(const QItemSelection&, const QItemSelection&);
		void showTriggerOutput(const int&, const int&);

	Q_SIGNALS:
		void triggerStatusModified(int, QList<QString>);
		void newDetectionTriggers(QString, QList<QString>);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		typedef QList<Trigger*> TriggerList;
		QScopedPointer<Ui::TriggerPanel> __ui;
		InteractiveTable* __table;
		FancyButton* __acceptButton;
		FancyButton* __rejectButton;
		FancyButton* __removeButton;
		FancyButton* __commitButton;
		FancyButton* __resetButton;
		FancyButton* __autocleanButton;
		TriggerList __triggers;
		HeaderActions __actions;

		struct Commitable {
				bool operator==(Commitable& c) {
					return this->objectFile == c.objectFile;
				}
				Trigger* object;
				QString objectFile;
		};
		QList<Commitable> __commitables;
};

class EnqueueDialog : public QDialog {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit EnqueueDialog(const QString&, QWidget* = NULL);
		~EnqueueDialog();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		const QString& information() const {
			return __information;
		}
		const QString& comment() const {
			return __comment;
		}
		const bool& accepted() const {
			return __accepted;
		}

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void validated();
		void cancelled();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QScopedPointer<Ui::Enqueue> __ui;
		QString __information;
		QString __comment;
		bool __accepted;
};

class ScriptEditor;
class DetectionPanel : public PanelWidget {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit DetectionPanel(QWidget* = NULL);
		~DetectionPanel();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		bool saveParameters();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void load(DetectionJob*);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void generateScript();
		void enqueueJob();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QScopedPointer<Ui::DetectionPanel> __ui;
		DataSourceSubPanel* __dataSrc;
		InventorySubPanel* __inventory;
		TimeWindowSubPanel* __tw;
		TriggerSubPanel* __trig;
		StreamSubPanel* __stream;
		ScriptEditor* __scriptEditor;
		SubPanelList __widgets;
};


class DispatchPanel : public PanelWidget {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit DispatchPanel(QWidget* = NULL);
		~DispatchPanel();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		bool saveParameters();
		bool loadConfiguration();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void load(DispatchJob*);
		void generateScript();
		void enqueueJob();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void showHideMsModOptions();
		void showHideCustomOptions();
		void selectDataDir();
		void selectSDSDir();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QScopedPointer<Ui::DispatchPanel> __ui;
		ScriptEditor* __scriptEditor;
};


class HelpPanel : public PanelWidget {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit HelpPanel(QWidget* = NULL);
		~HelpPanel();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		bool saveParameters();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QWebView* __view;
};


} // namespace Qt4
} // namespace SDP


#endif
