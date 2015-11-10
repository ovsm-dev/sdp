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



#ifndef __SDP_QT4_ERRORHANDLER_H__
#define __SDP_QT4_ERRORHANDLER_H__


#include <QtGlobal>
#include <QDateTime>
#include <stdio.h>

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

/**
 * @brief Custom message handler
 * @param type Qt message type
 * @param msg the message
 */
void errorHandler(QtMsgType type, const char* msg) {

	const char* time = QDateTime::currentDateTime().toString("hh:mm:ss").toStdString().c_str();
	switch ( type ) {
		case QtDebugMsg:
			fprintf(stdout, RESET "[%s] [" GREEN "DEBUG" RESET "] %s\n", time, msg);
			break;
		case QtWarningMsg:
			fprintf(stderr, RESET "[%s] [" YELLOW "WARNING" RESET "] %s\n", time, msg);
			break;
		case QtCriticalMsg:
			fprintf(stderr, RESET "[%s] [" RED "CRITICAL" RESET "] %s\n", time, msg);
			break;
		case QtFatalMsg:
			fprintf(stderr, RESET "[%s] [" RED "FATAL" RESET "] %s\n", time, msg);
			abort();
	}
}

#endif
