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



#ifndef __SDP_QT4_DATAMODEL_TRIGGER_H__
#define __SDP_QT4_DATAMODEL_TRIGGER_H__


#include <QObject>
#include <QString>
#include <QList>
#include <QDateTime>
#include <QDataStream>


namespace SDP {
namespace Qt4 {

enum TriggerStatus {
	WaitingForRevision, Accepted, Rejected, Committed, TRIGGER_STATUS_COUNT
};

struct TriggerStatusValue {
		TriggerStatus status;
		const char* string;
};

const TriggerStatusValue g_TriggerStatusValues[TRIGGER_STATUS_COUNT] = {
    { WaitingForRevision, "Waiting For Revision" }, { Accepted, "Accepted" },
    { Rejected, "Rejected" }, { Committed, "Committed" }
};

const TriggerStatusValue& operator*(TriggerStatus);

/**
 * @class Trigger
 * @brief This class implements a trigger resulting from a detection script.
 *        Like Jobs, triggers are stand alone entities in spite of being
 *        handled by panels.
 */
class Trigger : public QObject {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		struct Station {
				QString networkCode;
				QString code;
				QString channelCode;
				QString locationCode;
				QString snapshotFile;
				friend QDataStream& operator<<(QDataStream&, const Station&);
				friend QDataStream& operator>>(QDataStream&, Station&);
		};
		typedef QList<Station> StationList;

	protected:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Trigger(const QString&, QObject* = NULL);

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		static Trigger* create(const QString&);
		bool operator==(Trigger&);
		~Trigger();

	public:
		// ------------------------------------------------------------------
		//  Public stream interface
		// ------------------------------------------------------------------
		void fromDataStream(QDataStream&);
		void toDataStream(QDataStream&);

	public:
		// ------------------------------------------------------------------
		//  Public general interface
		// ------------------------------------------------------------------
		const QString id();
		void setStatus(const TriggerStatus& s);
		const TriggerStatus& status() const;
		const QDateTime& creationTime() const;
		void setOriginTime(const QDateTime& dt);
		const QDateTime& originTime() const;
		const int& runExitCode() const;
		void setTableWidget(void* tw);
		void* tableWidget();
		void setStations(const StationList& l);
		const StationList& stations() const;
		void setResumePage(const QString& p);
		const QString& resumePage() const;
		void setInformation(const QString& p);
		const QString& information() const;
		const QString& jobID() const;

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		void* __tableWidget;
		QDateTime __creationTime;
		QDateTime __originTime;
		StationList __stations;
		QString __resumePage;
		QString __info;
		QString __jobID;
		TriggerStatus __status;
		int __retCode;
};

} // namespace Qt4
} // namespace SDP

#endif
