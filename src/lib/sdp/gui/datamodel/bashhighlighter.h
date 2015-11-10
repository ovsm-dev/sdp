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


#ifndef __SDP_QT4_BASHHIGHLIGHTER_H__
#define __SDP_QT4_BASHHIGHLIGHTER_H__


#include <QSyntaxHighlighter>


namespace SDP {
namespace Qt4 {


/**
 * @class BashHighlightingRule
 * @brief Container to describe a highlighting rule.
 */
class BashHighlightingRule {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit BashHighlightingRule(const QString& patternStr, int n,
		                              const QTextCharFormat& matchingFormat) {

			originalRuleStr = patternStr;
			pattern = QRegExp(patternStr);
			nth = n;
			format = matchingFormat;
		}

	public:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		QString originalRuleStr;
		QRegExp pattern;
		int nth;
		QTextCharFormat format;
};


class BashHighlighter : public QSyntaxHighlighter {

	Q_OBJECT

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		BashHighlighter(QTextDocument* = NULL);

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		void highlightBlock(const QString&);

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		void initializeRules();

		//! Highlights multi-line strings, returns true if after processing
		//! we are still within the multi-line section.
		bool matchMultiline(const QString& text, const QRegExp& delimiter,
		                    const int inState, const QTextCharFormat& style);

		const QTextCharFormat getTextCharFormat(const QColor&,
		                                        const QString& style = QString());

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		typedef QList<BashHighlightingRule> RuleList;
		QStringList __keywords;
		QStringList __builtins;
		QStringList __builtinsVar;
		QStringList __unixCommands;
		QStringList __operators;
		QStringList __braces;
		QHash<QString, QTextCharFormat> __basicStyles;
		RuleList __rules;
		QRegExp triSingleQuote;
		QRegExp triDoubleQuote;
};

} // namespace Qt4
} // namespace SDP

#endif
