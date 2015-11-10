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



#ifndef __SDP_QT4_DATAMODEL_ARCHIVEOBJECTS_H__
#define __SDP_QT4_DATAMODEL_ARCHIVEOBJECTS_H__


#include <QStringList>
#include <QDateTime>


namespace SDP {
namespace Qt4 {


struct TimeWindow {
		QDateTime start;
		QDateTime end;
};

struct DetectionInformation {
		QString source;
		QString stations;
		QString channels;
		TimeWindow tw;
		QString sampleLength;
		QString triggerInfo;
		QString jobID;
};

struct DispatchInformation {
		QString tool;
		QString sdsDir;
		QString sdsPattern;
		QString msdDir;
		QString msdPattern;
		QString jobID;
};

class ArchivedOrigin;

class ArchivedStation {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit ArchivedStation(ArchivedOrigin*, const QString&, const QString&,
		                         const QString&, const QString&, const QString&);
		bool operator==(const ArchivedStation&);
		~ArchivedStation();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		ArchivedOrigin* origin();
		const QString& network() const;
		const QString& code() const;
		const QString& locationCode() const;
		const QString& channelCode() const;
		const QString& pixmap() const;

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		ArchivedOrigin* __parent;
		QString __network;
		QString __code;
		QString __locationCode;
		QString __channelCode;
		QString __pixmap;
};
typedef QList<ArchivedStation*> ArchivedStationList;

class ArchivedRun;

class ArchivedOrigin {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit ArchivedOrigin(ArchivedRun*, const QDateTime&, const QString&);
		bool operator==(const ArchivedOrigin&);
		~ArchivedOrigin();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		static ArchivedOrigin* load(ArchivedRun*, const QString&, const QString&);

		ArchivedRun* run();
		const QDateTime& time() const;
		const ArchivedStationList& stations() const;
		const QString& webpage() const;

		void appendStation(ArchivedStation*);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		ArchivedRun* __parent;
		QDateTime __time;
		ArchivedStationList __stations;
		QString __webpage;
};
typedef QList<ArchivedOrigin*> ArchivedOriginList;

class Job;

class ArchivedRun {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit ArchivedRun(const Job*);
		~ArchivedRun();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		static ArchivedRun* loadRun(const Job*);

		const Job* job();
		const ArchivedOriginList& origins() const;
		const QString& script() const;
		const TimeWindow& timeWindow() const;

		void appendOrigin(ArchivedOrigin*);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		const Job* __job;
		ArchivedOriginList __origins;
		QString __script;
		TimeWindow __tw;
};
typedef QList<ArchivedRun*> ArchivedRunList;

} // namespace Qt4
} // namespace SDP

#endif
