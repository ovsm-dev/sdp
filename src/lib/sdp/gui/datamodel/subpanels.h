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



#ifndef __SDP_QT4_DATAMODEL_SUBPANELS_H__
#define __SDP_QT4_DATAMODEL_SUBPANELS_H__


#include <QWidget>
#include <QString>
#include <QDialog>
#include <QDateTime>
#include <QScopedPointer>
#include <sdp/gui/datamodel/singleton.h>


namespace Ui {

class Entity;
class DataSource;
class Inventory;
class TimeWindow;
class Trigger;
class Stream;

}


QT_FORWARD_DECLARE_CLASS(QTreeWidgetItem);

namespace SDP {
namespace Qt4 {

/**
 * @class SubPanelWidget
 * @brief Virtual interface for any widget within a PanelWidget
 **/
class SubPanelWidget {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		SubPanelWidget() {}
		virtual ~SubPanelWidget() {}

	public:
		// ------------------------------------------------------------------
		//  Public virtual interface
		// ------------------------------------------------------------------
		virtual bool saveParameters()=0;
		virtual bool loadParameters()=0;
//		virtual bool loadConfiguration();
};
typedef QList<SubPanelWidget*> SubPanelList;


class DataSourceSubPanel : public QWidget, public SubPanelWidget,
                           public Singleton<DataSourceSubPanel> {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit DataSourceSubPanel(QWidget* = NULL);
		~DataSourceSubPanel();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		bool saveParameters();
		bool loadParameters();
		bool loadConfiguration();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		bool sourceIsOkay();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void loadPreset(int);
		void selectDataFile();
		void readFileInventory(bool);
		void readSEEDInventory();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void setTimeWindowAccessible(bool);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		struct Preset {
				Preset() :
						name("Unnamed"), idx(-1), arclinkPort(18001),
						arclinkTimeout(20) {}
				QString name;
				QString sourceType;
				QString sourceAddress;
				QString arclinkUser;
				QString arclinkPassword;
				QString arclinkInstitution;
				int idx;
				int arclinkPort;
				int arclinkTimeout;
		};
		typedef QList<Preset> PresetList;

		QScopedPointer<Ui::DataSource> __ui;
		PresetList __presets;
};


class StreamSubPanel : public QWidget, public SubPanelWidget,
                       public Singleton<StreamSubPanel> {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit StreamSubPanel(QWidget* = NULL);
		~StreamSubPanel();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		bool saveParameters();
		bool loadParameters();
		bool loadConfiguration();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		bool streamIsOkay();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void loadPreset(int);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		struct Preset {
				Preset() {
					name = "Unnamed";
					type = "Unknown";
					idx = -1;
				}
				QString name;
				QString type;
				QString customStream;
				int idx;
		};
		typedef QList<Preset> PresetList;

		QScopedPointer<Ui::Stream> __ui;
		PresetList __presets;
};


class ModalDialog : public QDialog {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum Type {
			Network, Station
		};
		enum DialogMode {
			Creation, Edition
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit ModalDialog(const Type& t, QWidget* parent = NULL);
		explicit ModalDialog(void*, QWidget* parent = NULL);
		~ModalDialog();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void checkForm();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void newNetwork(const QString& code);
		void newStation(const QString& networkCode, const QString& stationCode,
		                const QString& locationCode, const QString& channelCode);
		void editStation(const QString& networkCode, const QString& oldStationCode,
		                 const QString& newStationCode, const QString& newLocationCode,
		                 const QString& newChannelCode);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QScopedPointer<Ui::Entity> __ui;
		void* __object;
		Type __t;
		DialogMode __mode;
		QString __name;
};


class InventorySubPanel : public QWidget, public Singleton<InventorySubPanel>,
                          public SubPanelWidget {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		struct Entity {
				explicit Entity(const QString& name, Entity* parent = NULL);
				~Entity();
				Entity* parent;
				QTreeWidgetItem* item;
				QString name;
				QString locationCode;
				QString channelCode;
		};
		typedef QList<Entity*> EntityList;
		typedef QPair<QString, QString> PairedString;
		typedef QList<PairedString> PairedStringList;

		//ZU,CAPH,19.7355,-72.1883,17.0,2013-08-2T00:00:00,2013-10-10T23:59:00,HHZ, ,HHN, ,HHE, ,
		struct RespStation {
				RespStation() {
					network = "not set";
					code = "not set";
					latitude = 0.0;
					longitude = 0.0;
					elevation = 0.0;
				}
				bool operator==(const RespStation& r) {
					return this->network == r.network && this->code == r.code;
				}
				QString network;
				QString code;
				double latitude;
				double longitude;
				double elevation;
				QDateTime start;
				QDateTime end;
				PairedStringList locchan;
		};
		typedef QList<RespStation> RespStationsList;

		enum ASCIIDatalessFormat {
			RdseedStationSummary,
			Custom
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit InventorySubPanel(QWidget* = NULL);
		~InventorySubPanel();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		bool saveParameters();
		bool loadParameters();
		//		bool loadConfiguration();
		Entity* getNetwork(const QString&);
		Entity* getStation(const QString&, const QString&);
		void removeNetwork(const QString&);
		void removeStation(const QString&, const QString&);
		Entity* selectedEntity() const {
			return __selectedEntity;
		}
		const EntityList& entities() const {
			return __entities;
		}
		QString pythonStations(const QString& netVar, const QString& staVar);

		int networkCount();
		int stationCount();

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		void loadASCIIDataless(const QString&, ASCIIDatalessFormat = Custom);
		void loadBinaryDataless(const QString&);

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		bool inventoryIsOkay();
		void addNetwork(const QString&);
		void addStation(const QString&, const QString&, const QString&, const QString&);
		void editStation(const QString& networkCode, const QString& oldStationCode,
		                 const QString& newStationCode, const QString& newLocationCode,
		                 const QString& newChannelCode);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void showContextMenu(const QPoint&);
		void loadInventoryFromFile();
		void addNetwork();
		void addStation();
		void removeEntity();
		void editEntity();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QScopedPointer<Ui::Inventory> __ui;
		Entity* __selectedEntity;
		QString __currentEntity;
		EntityList __entities;
		RespStationsList __stations;
};


class TimeWindowSubPanel : public QWidget, public SubPanelWidget,
                           public Singleton<TimeWindowSubPanel> {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit TimeWindowSubPanel(QWidget* = NULL);
		~TimeWindowSubPanel();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		bool saveParameters();
		bool loadParameters();
		bool loadConfiguration();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		bool timeWindowIsOkay();
		bool sampleLengthIsOkay();
		void setTimeWindowSelectable(bool);

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void loadPreset(int);
		void setCustomBoxesVisible();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		struct Preset {
				Preset() :
						name("Unnamed"), sampleLengthUnit("Unknown"),
						sampleLength(-1), idx(-1) {}
				QString name;
				QString sampleLengthUnit;
				QDateTime start;
				QDateTime end;
				int sampleLength;
				int idx;
		};
		typedef QList<Preset> PresetList;
		QScopedPointer<Ui::TimeWindow> __ui;
		PresetList __presets;
};


class TriggerSubPanel : public QWidget, public SubPanelWidget,
                        public Singleton<TriggerSubPanel> {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit TriggerSubPanel(QWidget* = NULL);
		~TriggerSubPanel();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		bool saveParameters();
		bool loadParameters();
		bool loadConfiguration();

	private Q_SLOTS:
		void loadPreset(int);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		struct Preset {
				Preset() {
					name = "Unnamed";
					trigType = "classic";
					lta = 8.0;
					sta = 3.0;
					treshOn = 1.5;
					treshOff = 0.5;
					sum = 3;
					idx = -1;
					ratio = 0.0;
					quiet = 0.0;
				}
				QString name;
				QString trigType;
				double lta;
				double sta;
				double treshOn;
				double treshOff;
				int sum;
				int idx;
				double ratio;
				double quiet;
		};
		typedef QList<Preset> PresetList;

		QScopedPointer<Ui::Trigger> __ui;
		PresetList __presets;
};

} // namespace Qt4
} // namespace SDP


#endif
