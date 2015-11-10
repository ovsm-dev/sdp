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


#include <sdp/gui/datamodel/utils.h>
#include <QDir>
#include <QTextStream>
#include <QCoreApplication>
#include <QDebug>
#include <pwd.h>
#include <unistd.h>

#ifdef Q_OS_WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#endif

#define fKB 1024

namespace SDP {
namespace Qt4 {
namespace Utils {

bool mkdir(const QString& path) {
	return QDir().mkpath(path);
}

bool removeDir(const QString& dirName) {

	bool result = true;
	QDir dir(dirName);

	if ( dir.exists(dirName) ) {
		Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot
						| QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files,
						QDir::DirsFirst)){
		if (info.isDir()) {
			result = removeDir(info.absoluteFilePath());
		}
		else {
			result = QFile::remove(info.absoluteFilePath());
		}

		if (!result) {
			return result;
		}
	}
	result = dir.rmdir(dirName);
}
	return result;
}

bool dirExists(const QString& path) {
	return QDir(path).exists();
}

bool fileExists(const QString& filepath) {
	return QFile(filepath).exists();
}

bool writeFile(QFile& file, const QString& content) {

	QTextStream out(&file);
	try {
		out << content;
	} catch ( ... ) {
		return false;
	}

	return true;
}

bool writeScript(const QString& filepath, const QString& content) {

	QFile file(filepath);
	if ( !file.open(QIODevice::WriteOnly | QIODevice::Text) ) {
		qWarning() << "Failed to open file " << filepath;
		return false;
	}

	return writeFile(file, content);
}

QStringList getFileList(const QString& path, const QString& filter,
                        const QString& pattern) {

	QDir dir(path);
	dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
	dir.setSorting(QDir::Size | QDir::Reversed);

	QStringList files;

	QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); ++i) {
		QFileInfo fileInfo = list.at(i);

		if ( filter != "*" )
		    if ( fileInfo.fileName().right(filter.size()) != filter )
		        continue;

		if ( !pattern.isEmpty() )
		    if ( !fileInfo.fileName().contains(pattern) )
		        continue;

		files << path + QDir::separator() + fileInfo.fileName();
	}

	return files;
}

QStringList getFileContentList(const QString& file,
                               const bool& ignoreDuplicate) {

	QFile f(file);
	QStringList content;
	if ( f.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		QTextStream in(&f);
		while ( !in.atEnd() ) {
			const QString line = in.readLine();
			if ( line.isEmpty() ) continue;
			if ( ignoreDuplicate ) {
				if ( !content.contains(line) )
				    content << line;
			}
			else
				content << line;
		}
	}

	return content;
}

QString getFileContentString(const QString& file, const QString& separator,
                             const bool& ignoreDuplicate) {

	QFile f(file);
	QString content;
	if ( f.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		QTextStream in(&f);
		while ( !in.atEnd() ) {
			const QString line = in.readLine();
			if ( line.isEmpty() ) continue;
			if ( ignoreDuplicate ) {
				if ( !content.contains(line) )
				    content += line + separator;
			}
			else
				content += line + separator;
		}
	}

	return content;
}

void responsiveDelay(int msec) {
	QTime dieTime = QTime::currentTime().addMSecs(msec);
	while ( QTime::currentTime() < dieTime )
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

QString elapsedTime(const QDateTime& start, const QDateTime& end) {

	int span = start.secsTo(end);
	int days = span / (3600 * 24);
	int hours = span / 3600;
	int mins = span / 60;

	QString str;
	if ( days != 0 ) str += QString("%1 day(s) ").arg(QString::number(days));
	if ( hours != 0 ) str += QString("%1 hour(s) ").arg(QString::number(hours));
	if ( mins != 0 ) str += QString("%1 min(s) ").arg(QString::number(mins));
	if ( days == 0 && hours == 0 && mins == 0 )
	    str += QString("%1 sec(s) ").arg(QString::number(span));

	return str;
}

quint64 dirSize(const QString& str) {

	quint64 sizex = 0;
	QFileInfo str_info(str);

	if ( !str_info.isDir() )
	    return sizex;

	QDir dir(str);
	QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs
	    | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	for (int i = 0; i < list.size(); ++i) {
		QFileInfo fileInfo = list.at(i);
		if ( fileInfo.isDir() )
			sizex += dirSize(fileInfo.absoluteFilePath());
		else
			sizex += fileInfo.size();
	}

	return sizex;
}

quint64 userAvailableFreeSpace() {

	struct statvfs stat;
	struct passwd* pw = getpwuid(getuid());
	if ( pw && 0 == statvfs(pw->pw_dir, &stat) ) {
		quint64 freeBytes = static_cast<quint64>(stat.f_bavail)
		    * static_cast<quint64>(stat.f_frsize);
		return freeBytes;
	}
	return 0ULL;
}

bool getFreeTotalSpace(const QString& sDirPath, double& fTotal, double& fFree) {

#ifdef Q_OS_WIN32

	QString sCurDir = QDir::current().absPath();
	QDir::setCurrent( sDirPath );

	ULARGE_INTEGER free,total;
	bool bRes = ::GetDiskFreeSpaceExA( 0 , &free , &total , NULL );
	if ( !bRes ) return false;

	QDir::setCurrent( sCurDir );

	fFree = static_cast<double>( static_cast<__int64>(free.QuadPart) ) / fKB;
	fTotal = static_cast<double>( static_cast<__int64>(total.QuadPart) ) / fKB;

#else

	struct stat stst;
	struct statvfs stfs;

	if ( ::stat(sDirPath.toLocal8Bit(), &stst) == -1 ) return false;
	if ( ::statvfs(sDirPath.toLocal8Bit(), &stfs) == -1 ) return false;

	fFree = stfs.f_bavail * (stst.st_blksize / fKB);
	fTotal = stfs.f_blocks * (stst.st_blksize / fKB);

#endif

	return true;
}

/*
 //! Get the disk available free space by using the configuration file
 //! path as reference disk.
 struct statvfs buf;
 if ( !statvfs(pm->parameter("PROJECT_CONFIG_FILE").toString().toStdString().c_str(),
 &buf) ) {

 unsigned long blksize, blocks, freeblks;

 blksize = buf.f_bsize;
 blocks = buf.f_blocks;
 freeblks = buf.f_bfree;

 quint64 disk_size = static_cast<quint64>(blocks) * static_cast<quint64>(blksize);
 quint64 free = freeblks * blksize;
 quint64 used = disk_size - free;

 qDebug() << "Disk size: " << disk_size / (1024 * 1024);
 qDebug() << "Disk used: " << used / (1024 * 1024);
 qDebug() << "Disk free: " << free / (1024 * 1024);
 }
 else {
 printf("Couldn't get file system statistics\n");
 }
 */

quint64 percentageOfSomething(const quint64& size, const quint64& iterator) {

	// Check zero division first
	if ( size == 0 ) return 0;

	return (100 * iterator) / size;
}

} // namespace Utils
} // namespace Qt4
} // namespace SDP

