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

#include <sdp/gui/datamodel/logger.h>
#include "ui_logger.h"
#include <QtGui>


namespace SDP {
namespace Qt4 {


Logger::Logger(QWidget* parent, const QString& name, Qt::WFlags f) :
		QDialog(parent, f), __ui(new Ui::Logger) {

	__ui->setupUi(this);
	setWindowTitle(QString("%1 log's").arg(name));

	connect(__ui->pushButtonClear, SIGNAL(clicked()), this, SLOT(clearMessages()));
	connect(__ui->pushButtonOkay, SIGNAL(clicked()), this, SLOT(hide()));

	__ui->tableWidget->verticalHeader()->setVisible(false);
	__ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	__ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	__ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	__ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	__ui->tableWidget->resizeColumnsToContents();
	__ui->tableWidget->resizeRowsToContents();
	__ui->tableWidget->setSelectionMode(QTreeView::ExtendedSelection);
	__ui->tableWidget->setSelectionBehavior(QTreeView::SelectRows);
	__ui->tableWidget->setAlternatingRowColors(false);
	__ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

	( qApp->arguments().contains("--debug")) ? __stdMessages = true : __stdMessages = false;
}


Logger::~Logger() {}


void Logger::addMessage(MsgType mt, const QString& module,
                        const QString& message, const bool& stdout) {

	int idx = __ui->tableWidget->rowCount();
	__ui->tableWidget->insertRow(idx);

	QFont bf(font());
	bf.setBold(true);
	bf.setPointSize(font().pointSize() - 1);

	QFont f(font());
	f.setPointSize(font().pointSize() - 1);

	QTableWidgetItem* type = new QTableWidgetItem;
	type->setFont(bf);
	type->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	switch ( mt ) {
		case DEBUG:
			type->setText("DEBUG");
			type->setForeground(Qt::darkGreen);
			if ( stdout && __stdMessages )
			    qDebug() << qPrintable(message);
			break;
		case INFO:
			type->setText("INFO");
			type->setForeground(Qt::darkGray);
			if ( stdout && __stdMessages )
			    qDebug() << qPrintable(message);
			break;
		case WARNING:
			type->setText("WARNING");
			type->setForeground(Qt::darkYellow);
			if ( stdout && __stdMessages )
			    qWarning() << qPrintable(message);
			break;
		case CRITICAL:
			type->setText("CRITICAL");
			type->setForeground(Qt::red);
			if ( stdout && __stdMessages )
			    qCritical() << qPrintable(message);
			break;
	}

	QTableWidgetItem* ts = new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
	ts->setFont(f);

	QTableWidgetItem* mod = new QTableWidgetItem(module);
	mod->setFont(bf);
//	mod->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	QTableWidgetItem* msg = new QTableWidgetItem(message);
	msg->setFont(f);

	__ui->tableWidget->setItem(idx, 0, type);
	__ui->tableWidget->setItem(idx, 1, ts);
	__ui->tableWidget->setItem(idx, 2, mod);
	__ui->tableWidget->setItem(idx, 3, msg);

	__ui->tableWidget->resizeColumnsToContents();
	__ui->tableWidget->resizeRowsToContents();
}


void Logger::setAppName(const QString& name) {
	__appName = name;
	setWindowTitle(QString("%1 log's").arg(name));
}


void Logger::clearMessages() {
	__ui->tableWidget->clearSelection();
	__ui->tableWidget->setRowCount(0);
}


} // namespace Qt4
} // namespace SDP
