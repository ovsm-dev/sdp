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

#ifndef __SDP_QT4_DATAMODEL_UTILS_H__
#define __SDP_QT4_DATAMODEL_UTILS_H__


#include <QFile>
#include <QString>
#include <QDateTime>
#include <QStringList>
#include <QVariant>


namespace SDP {
namespace Qt4 {
namespace Utils {

bool mkdir(const QString& path);

bool removeDir(const QString& path);

bool dirExists(const QString& path);

bool fileExists(const QString& filepath);

bool writeFile(QFile& file, const QString& content);

bool writeScript(const QString& filepath, const QString& content);

QStringList getFileList(const QString& path, const QString& filter = "*",
                        const QString& pattern = QString());

QStringList getFileContentList(const QString& file,
                               const bool& ignoreDuplicate = true);

QString getFileContentString(const QString& file, const QString& separator,
                             const bool& ignoreDuplicate = true);

/**
 * @brief Wait function that doesn't impact Qt event looper
 * @param msec The time to wait
 */
void responsiveDelay(int msec);

QString elapsedTime(const QDateTime& start, const QDateTime& end);

template<class T> class VariantPtr {
	public:
		static T* asPtr(QVariant v) {
			return reinterpret_cast<T*>(v.value<void *>());
		}
		static QVariant asQVariant(T* ptr) {
			return qVariantFromValue((void *) ptr);
		}
};

quint64 dirSize(const QString&);

/**
 * @brief Diskfree function
 * @return byte value of available free space
 */
quint64 userAvailableFreeSpace();
bool getFreeTotalSpace(const QString& sDirPath, double& fTotal, double& fFree);

quint64 percentageOfSomething(const quint64& size, const quint64& iterator);

} // namespace Utils
} // namespace Qt4
} // namespace SDP

#endif
