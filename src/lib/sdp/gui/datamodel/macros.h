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



#ifndef __SDP_QT4_MACROS_H__
#define __SDP_QT4_MACROS_H__


#include <QDebug>
#include <cassert>

//! Function pre-declaration
QString outputFunction(QString);

//! Some std output methods
#define SDPDEBUG(message) \
(\
    (qDebug() << outputFunction(Q_FUNC_INFO).toStdString().c_str() << ":" << QString(message).toStdString().c_str()), \
    (void)0 \
)

#define SDPWARN(message) \
(\
    (qWarning() << outputFunction(Q_FUNC_INFO).toStdString().c_str() << ":" << QString(message).toStdString().c_str()), \
    (void)0 \
)

#define SDPCRITICAL(message) \
(\
    (qCritical() << outputFunction(Q_FUNC_INFO).toStdString().c_str() << ":" << QString(message).toStdString().c_str()), \
    (void)0 \
)

#define SDPFATAL(message) \
(\
    (qFatal("%s : %s", outputFunction(Q_FUNC_INFO).toStdString().c_str(), QString(message).toStdString().c_str())), \
    (void)0 \
)

//! Condition checker
#define SDPASSERT(condition) {\
	if(!(condition)) {\
		qCritical() << "Assertion failed at: " << __FILE__ << ":" << __LINE__;\
		qCritical() << "Function: " << __FUNCTION__;\
		qCritical() << "Condition: " << #condition;\
		abort();\
	}\
}



#endif
