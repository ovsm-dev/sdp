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


#include "../api.h"
#include <sdp/gui/datamodel/job.h>
#include <sdp/gui/datamodel/logger.h>
#include <sdp/gui/datamodel/macros.h>
#include <sdp/gui/datamodel/trigger.h>
#include <sdp/gui/datamodel/parametermanager.h>
#include <sdp/gui/datamodel/databasemanager.h>
#include <sdp/gui/datamodel/utils.h>
#include <sdp/gui/datamodel/cache.h>
#include <sdp/gui/datamodel/macros.h>

#include <QProcess>
#include <QDir>


namespace SDP {
namespace Qt4 {

const JobTypeValue& operator*(JobType jt) {
	SDPASSERT(jt >= 0 && jt < JOB_TYPE_COUNT);
	SDPASSERT(g_JobTypeValues[jt].type == jt);
	return g_JobTypeValues[jt];
}


const JobStatusValue& operator*(JobStatus js) {
	SDPASSERT(js >= 0 && js < JOB_STATUS_COUNT);
	SDPASSERT(g_JobStatusValues[js].status == js);
	return g_JobStatusValues[js];
}


Job::Job(QObject* parent) :
		QObject(parent), __process(NULL), __tableWidget(NULL), __type(Unknown),
		__status(Pending), __creationTime(QDateTime::currentDateTime()),
		__retCode(-2) {}


Job::Job(const JobType& t, QObject* parent) :
		QObject(parent), __process(NULL), __tableWidget(NULL), __type(t),
		__status(Pending), __creationTime(QDateTime::currentDateTime()),
		__retCode(-2) {}


bool Job::operator==(Job& job) {
	return this->id() == job.id();
}


Job::~Job() {
	if ( __process ) {
		__process->kill();
		delete __process;
	}
}


void Job::fromDataStream(QDataStream& stream) {

	QVariant v;
	stream >> __creationTime >> __runStart >> __runEnd >> __scriptData
	    >> __info >> __comment >> __tooltip >> __runDir >> __customRunDir
	    >> __profile >> v;

	QStringList stdl = v.value<QString>().split("<:br>");
	__stdOutput.clear();
	for (int i = 0; i < stdl.count(); ++i) {
		if ( stdl.at(i).contains("<:black>") )
			__stdOutput << OutputEntry(Info, stdl.at(i).mid(8, stdl.at(i).size()));
		else
			__stdOutput << OutputEntry(Error, stdl.at(i).mid(6, stdl.at(i).size()));
	}

	quint32 retCode = 0;
	stream >> retCode;
	__retCode = static_cast<int>(retCode);

	quint32 tmp;
	stream >> tmp;
	__type = static_cast<JobType>(tmp);

	tmp = 0;
	stream >> tmp;
	__status = static_cast<JobStatus>(tmp);
}


void Job::toDataStream(QDataStream& stream) {

	QString stdOutput;
	for (int i = 0; i < __stdOutput.count(); ++i) {
		stdOutput.append(
		    (__stdOutput.at(i).first == Info) ? "<:black>" : "<:red>");
		stdOutput.append(__stdOutput.at(i).second);
		stdOutput.append("<:br>");
	}
	QVariant v = QVariant::fromValue(stdOutput);

	stream << __creationTime << __runStart << __runEnd << __scriptData
	    << __info << __comment << __tooltip << __runDir << __customRunDir
	    << __profile << v << static_cast<quint32>(__retCode)
	    << static_cast<quint32>(__type)
	    << static_cast<quint32>(__status);
}


void Job::reset(bool deleteAll) {

	SDPASSERT(Logger::instancePtr());

	__status = Pending;
	__retCode = -2;
	if ( deleteAll )
	    if ( Utils::removeDir(__runDir) )
	        Logger::instancePtr()->addMessage(Logger::INFO, __func__,
	            "Removed job " + id() + " data");
}


const QString Job::id() {
	return QString("%1-%2").arg((__type == Dispatch) ? "dispatch" : "detection")
	    .arg(__creationTime.toString("yyyyMMddHHmmss"));
}


void Job::setJobType(const JobType& t) {
	__type = t;
}


void Job::setInformation(const QString& s) {
	__info = s;
}


void Job::setComment(const QString& s) {
	__comment = s;
}


void Job::setTooltip(const QString& s) {
	__tooltip = s;
}


const JobType& Job::type() const {
	return __type;
}


const JobStatus& Job::status() const {
	return __status;
}


const QDateTime& Job::creationTime() const {
	return __creationTime;
}


const QDateTime& Job::runStartTime() const {
	return __runStart;
}


const QDateTime& Job::runEndTime() const {
	return __runEnd;
}


const QString& Job::information() const {
	return __info;
}


const QString& Job::comment() const {
	return __comment;
}


const QString& Job::tooltip() const {
	return __tooltip;
}


const QString& Job::runDir() const {
	return __runDir;
}


void Job::setCustomRunDir(const QString& s) {
	__customRunDir = s;
}


const QString& Job::customRunDir() const {
	return __customRunDir;
}


const Job::StandardOutput& Job::stdOutput() const {
	return __stdOutput;
}


void Job::setProfile(const Profile& p) {
	__profile = p;
}


const Job::Profile& Job::profile() const {
	return __profile;
}


void Job::setScriptData(const QVariant& d) {
	__scriptData = d;
}


const QVariant& Job::scriptData() const {
	return __scriptData;
}


const int& Job::runExitCode() const {
	return __retCode;
}


void Job::setTableWidget(void* tw) {
	__tableWidget = tw;
}


void* Job::tableWidget() {
	return __tableWidget;
}


void Job::run() {

	SDPASSERT(Logger::instancePtr());
	Logger* log = Logger::instancePtr();

	if ( __type == Unknown ) {
		log->addMessage(Logger::WARNING, __func__, "Can't execute unqualified job " + id());
		return;
	}

	if ( __process ) {
		delete __process;
		__process = NULL;
		log->addMessage(Logger::WARNING, __func__, "Deleted existing instance of job " + id());
	}

	SDPASSERT(ParameterManager::instancePtr());
	ParameterManager* pm = ParameterManager::instancePtr();

	//! Create the run dir and all the stuff going inside it
	__runDir = pm->parameter("RUN_DIR").toString() + QDir::separator() + id();
	/*
	 if ( __type == Dispatch ) {
	 __runDir = pm->parameter("RUN_DIR").toString() + QDir::separator() + id();
	 }
	 else
	 */
	if ( __type == Detection && !__customRunDir.isEmpty() ) {
		__runDir = pm->parameter("RUN_DIR").toString() + QDir::separator() + __customRunDir;
	}
	/*
	 else {
	 log->addMessage(Logger::CRITICAL, __func__, "Job " + id() + " isn't a proper Job. Rejecting it.");
	 emit errorMessage("Job " + id() + " isn't a proper Job. Rejecting it.");
	 return;
	 }
	 */

	if ( !Utils::mkdir(__runDir) ) {
		log->addMessage(Logger::WARNING, __func__, "Failed to create path " + __runDir + "for job " + id());
		emit errorMessage("Failed to create path " + __runDir + "for job " + id());
		return;
	}

	QString scriptFile = __runDir + QDir::separator() + ((__type == Dispatch) ?
	    "dispatch.sh" : "detect.py");
	QString data = __scriptData.toString();
	if ( !Utils::writeScript(scriptFile, data.replace("@JOB_RUN_DIR@", __runDir + QDir::separator())) ) {
		log->addMessage(Logger::WARNING, __func__, "Failed to generate script file " + scriptFile + " of job " + id());
		emit errorMessage("Failed to generate script file " + scriptFile + " of job " + id());
		return;
	}

	if ( !Utils::fileExists(scriptFile) ) {
		__status = Stopped;
		log->addMessage(Logger::WARNING, __func__, "Job's " + id() + " script file not found");
		emit errorMessage("Job's " + id() + " script file not found");
		return;
	}

	__process = new QProcess(this);
	__process->setReadChannel(QProcess::StandardOutput);
	__process->setWorkingDirectory(__runDir);
	connect(__process, SIGNAL(started()), this, SLOT(starting()));
	connect(__process, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcStdOut()));
	connect(__process, SIGNAL(readyReadStandardError()), this, SLOT(readProcStdErr()));
	connect(__process, SIGNAL(finished(int)), this, SLOT(procTerminated(int)));

	if ( __type == Detection )
		//! Add '-u' argument to project lively the output (unbuffered)
		//! TODO: QProcess tends to read the output in a weird way, even
		//! with Unbuffered set, it kind of mixes up lines... Find hack for that!
		//! @note Passing arguments like this may not be the way Qt manual
		//! recommends, but wth it works the same... if not better...
		__process->start(QString("%1 -u %2")
		    .arg(pm->parameter("PYTHON_BIN").toString())
		    .arg(scriptFile), QProcess::Unbuffered | QProcess::ReadOnly);
	else
		__process->start(QString("%1 %2")
		    .arg(pm->parameter("BASH_BIN").toString())
		    .arg(scriptFile), QProcess::Unbuffered | QProcess::ReadOnly);

	if ( !__process->waitForStarted() ) {
		log->addMessage(Logger::CRITICAL, __func__, "Failed to start job's "
		    + id() + " script " + scriptFile);
		delete __process;
		__process = NULL;
		return;
	}

	__runStart = QDateTime::currentDateTime();
}

void Job::stop() {

	SDPASSERT(__process);
	__status = Stopped;
	__process->kill();
	emit stopped();
}


void Job::starting() {
	__stdOutput.clear();
	__status = Running;
	emit started();
}


void Job::readProcStdOut() {

	QByteArray stdout = __process->readAllStandardOutput();
	QList<QByteArray> output = stdout.split('\n');
	for (QList<QByteArray>::iterator it = output.begin();
	        it != output.end(); ++it) {
		if ( it->isEmpty() ) continue;
		__stdOutput << OutputEntry(Info, (*it).constData());
	}

	emit newStdMsg();
}


void Job::readProcStdErr() {

	QByteArray stderr = __process->readAllStandardError();
	QList<QByteArray> output = stderr.split('\n');
	for (QList<QByteArray>::iterator it = output.begin();
	        it != output.end(); ++it) {
		if ( it->isEmpty() ) continue;
		__stdOutput << OutputEntry(Error, (*it).constData());
	}

	emit newStdMsg();
}


void Job::procTerminated(int retCode) {

	//! @info Deleting the process here will result in segmentation fault...

	__retCode = retCode;
	disconnect(__process);

	if ( __status != Stopped )
	    __status = Terminated;

	__runEnd = QDateTime::currentDateTime();

	emit terminated();
}


void Job::updateReading() {
	SDPASSERT(__process);
	__process->waitForReadyRead();
}


DispatchJob::DispatchJob(QObject* parent) :
		Job(Dispatch, parent) {}


DispatchJob::~DispatchJob() {}


DispatchJob* DispatchJob::create() {

	SDPASSERT(Cache::instancePtr());

	DispatchJob* job = new DispatchJob;

	if ( Cache::instancePtr()->addObject(job) )
		return job;
	else {
		delete job;
		job = NULL;
	}

	return job;
}


void DispatchJob::fromDataStream(QDataStream& stream) {
	Job::fromDataStream(stream);
	stream >> __tool >> __sdsDir >> __sdsPattern >> __msdDir >> __msdPattern;
}


void DispatchJob::toDataStream(QDataStream& stream) {
	Job::toDataStream(stream);
	stream << __tool << __sdsDir << __sdsPattern << __msdDir << __msdPattern;
}

void DispatchJob::setTool(const QString& v) {
	__tool = v;
}


void DispatchJob::setSdsDir(const QString& v) {
	__sdsDir = v;
}

void DispatchJob::setSdsPattern(const QString& v) {
	__sdsPattern = v;
}


void DispatchJob::setMsdDir(const QString& v) {
	__msdDir = v;
}


void DispatchJob::setMsdPattern(const QString& v) {
	__msdPattern = v;
}


const QString& DispatchJob::tool() const {
	return __tool;
}


const QString& DispatchJob::sdsDir() const {
	return __sdsDir;
}


const QString& DispatchJob::sdsPattern() const {
	return __sdsPattern;
}


const QString& DispatchJob::msdDir() const {
	return __msdDir;
}


const QString& DispatchJob::msdPattern() const {
	return __msdPattern;
}


QDataStream& operator<<(QDataStream& stream, const DetectionJob::DetectionStation& s) {
	stream << s.networkCode << s.code << s.locationCode << s.channelCode;
	return stream;
}


QDataStream& operator>>(QDataStream& stream, DetectionJob::DetectionStation& s) {
	stream >> s.networkCode >> s.code >> s.locationCode >> s.channelCode;
	return stream;
}


DetectionJob::DetectionJob(QObject* parent) :
		Job(Detection, parent) {

	__params.insert(peDATASOURCE, ParameterList());
	__params.insert(peSTREAM, ParameterList());
	__params.insert(peTIMEWINDOW, ParameterList());
	__params.insert(peTRIGGER, ParameterList());
}


DetectionJob::~DetectionJob() {}


DetectionJob* DetectionJob::create() {

	SDPASSERT(Cache::instancePtr());

	DetectionJob* job = new DetectionJob;

	if ( Cache::instancePtr()->addObject(job) )
		return job;
	else {
		delete job;
		job = NULL;
	}

	return job;
}


void DetectionJob::fromDataStream(QDataStream& stream) {

	Job::fromDataStream(stream);
	stream >> __stations;

	//! Next byte indicates the number of iterations to perform in order
	//! to get each parameter list.
	quint32 count = 0;
	stream >> count;

	__params.clear();
	for (quint32 it = 0; it < count; ++it) {
		quint32 tmp = 0;
		ParameterList l;
		stream >> tmp >> l;
		__params.insert(static_cast<ParameterEntity>(tmp), l);
	}
}


void DetectionJob::toDataStream(QDataStream& stream) {

	Job::toDataStream(stream);
	stream << __stations;
	stream << static_cast<quint32>(__params.size());
	for (Parameters::const_iterator it = __params.constBegin();
	        it != __params.constEnd(); ++it) {
		stream << static_cast<quint32>(it.key()) << it.value();
	}
}


void DetectionJob::loadTriggers() {

	SDPASSERT(DatabaseManager::instancePtr());
	__triggers = DatabaseManager::instancePtr()->triggers(id());
}


void DetectionJob::setStations(const DetectionStationList& l) {
	__stations = l;
}


void DetectionJob::setParameterList(const ParameterEntity& pe,
                                    const ParameterList& l) {
	__params[pe] = l;
}


const DetectionJob::DetectionStationList& DetectionJob::stations() const {
	return __stations;
}


const DetectionJob::TriggerList& DetectionJob::triggers() const {
	return __triggers;
}


DetectionJob::ParameterList DetectionJob::parameters(const ParameterEntity& pe) {
	if ( __params.contains(pe) )
	    return __params.value(pe);
	return ParameterList();
}


} // namespace Qt4
} // namespace SDP
