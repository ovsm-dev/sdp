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



#ifndef __SDP_QT4_DATAMODEL_JOB_H__
#define __SDP_QT4_DATAMODEL_JOB_H__


#include <QObject>
#include <QPair>
#include <QList>
#include <QString>
#include <QHash>
#include <QVariant>
#include <QDateTime>
#include <QDataStream>
#include <QMetaType>


QT_FORWARD_DECLARE_CLASS(QProcess);


namespace SDP {
namespace Qt4 {

enum JobType {
	Dispatch, Detection, Unknown, JOB_TYPE_COUNT
};

struct JobTypeValue {
		JobType type;
		const char* string;
};

const JobTypeValue g_JobTypeValues[JOB_TYPE_COUNT] = {
    { Dispatch, "Dispatch" }, { Detection, "Detection" }, { Unknown, "Unknown" }
};

const JobTypeValue& operator*(JobType);

enum JobStatus {
	Pending, Running, Terminated, Stopped, JOB_STATUS_COUNT
};

struct JobStatusValue {
		JobStatus status;
		const char* string;
};

const JobStatusValue g_JobStatusValues[JOB_STATUS_COUNT] = {
    { Pending, "Pending" }, { Running, "Running" }, { Terminated, "Terminated" },
    { Stopped, "Stopped" }
};

const JobStatusValue& operator*(JobStatus);

/**
 * @class Job
 * @brief This class implements an application task. Jobs are handled mainly
 *        by the ActivityPanel and less frequently by the TriggerPanel.
 * @info  The lifetime of a job is virtually based upon its status. If a job
 *        status differs from Terminated, said job is considered as alive and
 *        can be instantiated, otherwise said job is 'obsolete'.
 */
class Job : public QObject {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum OutputType {
			Info, Error
		};
		typedef QPair<OutputType, QString> OutputEntry;
		typedef QList<OutputEntry> StandardOutput;
		typedef QHash<QString, QVariant> Profile;

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Job(QObject* = NULL);
		explicit Job(const JobType&, QObject* = NULL);
		virtual ~Job();
		bool operator==(Job&);

	public:
		// ------------------------------------------------------------------
		//  Public stream interface
		// ------------------------------------------------------------------
		virtual void fromDataStream(QDataStream&);
		virtual void toDataStream(QDataStream&);

	public:
		// ------------------------------------------------------------------
		//  Public general interface
		// ------------------------------------------------------------------
		void reset(bool deleteAll = false);
		const QString id();
		void setJobType(const JobType& t);
		void setInformation(const QString& s);
		void setComment(const QString& s);
		void setTooltip(const QString& s);
		const JobType& type() const;
		const JobStatus& status() const;
		const QDateTime& creationTime() const;
		const QDateTime& runStartTime() const;
		const QDateTime& runEndTime() const;
		const QString& information() const;
		const QString& comment() const;
		const QString& tooltip() const;
		const QString& runDir() const;
		void setCustomRunDir(const QString& s);
		const QString& customRunDir() const;
		const StandardOutput& stdOutput() const;
		void setProfile(const Profile& p);
		const Profile& profile() const;
		void setScriptData(const QVariant& d);
		const QVariant& scriptData() const;
		const int& runExitCode() const;
		void setTableWidget(void* tw);

		void* tableWidget();

	public Q_SLOTS:
		// ------------------------------------------------------------------
		//  Public Qt interface
		// ------------------------------------------------------------------
		void run();
		void stop();

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void starting();
		void readProcStdOut();
		void readProcStdErr();
		void procTerminated(int);
		void updateReading();

	Q_SIGNALS:
		// ------------------------------------------------------------------
		//  Qt signals
		// ------------------------------------------------------------------
		void started();
		void stopped();
		void terminated();
		void newStdMsg();
		void errorMessage(QString);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QProcess* __process;
		void* __tableWidget;
		JobType __type;
		JobStatus __status;
		QDateTime __creationTime;
		QDateTime __runStart;
		QDateTime __runEnd;
		QVariant __scriptData;
		QString __info;
		QString __comment;
		QString __tooltip;
		QString __runDir;
		QString __customRunDir;
		StandardOutput __stdOutput;
		Profile __profile;
		int __retCode;
};

class DispatchJob : public Job {

	Q_OBJECT

	protected:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit DispatchJob(QObject* = NULL);
		~DispatchJob();

	public:
		// ------------------------------------------------------------------
		//  Creator
		// ------------------------------------------------------------------
		static DispatchJob* create();

	public:
		// ------------------------------------------------------------------
		//  Public stream interface
		// ------------------------------------------------------------------
		void fromDataStream(QDataStream&);
		void toDataStream(QDataStream&);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void setTool(const QString&);
		void setSdsDir(const QString&);
		void setSdsPattern(const QString&);
		void setMsdDir(const QString&);
		void setMsdPattern(const QString&);

		const QString& tool() const;
		const QString& sdsDir() const;
		const QString& sdsPattern() const;
		const QString& msdDir() const;
		const QString& msdPattern() const;

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QString __tool;
		QString __sdsDir;
		QString __sdsPattern;
		QString __msdDir;
		QString __msdPattern;
};

class Trigger;
class DetectionJob : public Job {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		enum ParameterEntity {
			peDATASOURCE, peSTREAM, peTIMEWINDOW, peTRIGGER
		};
		struct DetectionStation {
				QString networkCode;
				QString code;
				QString channelCode;
				QString locationCode;
				friend QDataStream& operator<<(QDataStream&, const DetectionStation&);
				friend QDataStream& operator>>(QDataStream&, DetectionStation&);
		};
		typedef QList<DetectionStation> DetectionStationList;
		typedef QList<Trigger*> TriggerList;
		typedef QHash<QString, QVariant> ParameterList;
		typedef QHash<ParameterEntity, ParameterList> Parameters;

	protected:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit DetectionJob(QObject* = NULL);
		~DetectionJob();

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		static DetectionJob* create();

	public:
		// ------------------------------------------------------------------
		//  Public stream interface
		// ------------------------------------------------------------------
		void fromDataStream(QDataStream&);
		void toDataStream(QDataStream&);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		void loadTriggers();

		void setStations(const DetectionStationList&);
		void setParameterList(const ParameterEntity&, const ParameterList&);

		const DetectionStationList& stations() const;
		const TriggerList& triggers() const;
		ParameterList parameters(const ParameterEntity&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		DetectionStationList __stations;
		TriggerList __triggers;
		Parameters __params;
};


} // namespace Qt4
} // namespace SDP

Q_DECLARE_METATYPE(SDP::Qt4::Job::OutputEntry);
Q_DECLARE_METATYPE(SDP::Qt4::Job::StandardOutput);

#endif
