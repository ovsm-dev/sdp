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



#ifndef __SDP_QT4_LOGGER_H__
#define __SDP_QT4_LOGGER_H__


#include <QWidget>
#include <QDialog>
#include <QScopedPointer>
#include <sdp/gui/datamodel/singleton.h>


namespace Ui {
class Logger;
}

namespace SDP {
namespace Qt4 {


/**
 * @class Logger
 * @brief This class implements a logger dialog. The user should instantiate
 *        it before any other SDP objects because they lean on it.
 */
class Logger : public QDialog, public Singleton<Logger> {

	Q_OBJECT

	public:
		enum MsgType {
			DEBUG, INFO, WARNING, CRITICAL
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Logger(QWidget* parent = NULL,
		                const QString& name = QString("Unknown"),
		                Qt::WFlags f = 0);
		~Logger();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		/**
		 * @brief Adds a new message to the log.
		 * @param mt the message's type @see LogMessage enum
		 * @param module the name of the module sending the message
		 * @param message the message itself
		 */
		void addMessage(MsgType mt, const QString& module,
		                const QString& message,
		                const bool& stdout = true);
		void setAppName(const QString&);
		const QString& appName() const {
			return __appName;
		}

	private Q_SLOTS:
		// ------------------------------------------------------------------
		//  Private Qt interface
		// ------------------------------------------------------------------
		void clearMessages();

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QScopedPointer<Ui::Logger> __ui;
		QString __appName;
		bool __stdMessages;
};


} // namespace Qt4
} // namespace SDP

#endif
