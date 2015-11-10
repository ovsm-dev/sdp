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



#include <sdp/gui/datamodel/bashhighlighter.h>


namespace SDP {
namespace Qt4 {


BashHighlighter::BashHighlighter(QTextDocument* parent) :
		QSyntaxHighlighter(parent) {

	__keywords = QStringList() << "else" << "for" << "do" << "function" << "in"
	    << "select" << "until" << "while" << "elif" << "if" << "then" << "fi"
	    << "done" << "set";

	__builtins = QStringList() << ":" << "source" << "alias" << "bg"
	    << "bind" << "break" << "builtin" << "cd" << "caller" << "command"
	    << "compgen" << "complete" << "continue" << "dirs" << "disown"
	    << "echo" << "enable" << "eval" << "exec" << "exit" << "fc" << "fg"
	    << "getopts" << "hash" << "help" << "history" << "jobs" << "kill"
	    << "let" << "logout" << "popd" << "printf" << "pushd" << "pwd"
	    << "return" << "set" << "shift" << "shopt" << "suspend" << "test"
	    << "time" << "times" << "trap" << "type" << "ulimit" << "umask"
	    << "unalias" << "wait";

	__builtinsVar = QStringList() << "export" << "unset" << "declare"
	    << "typeset" << "local" << "read" << "readonly";

	__unixCommands = QStringList() << "arch" << "awk" << "bash" << "bunzip2"
	    << "bzcat" << "bscmp" << "bzdiff" << "bzegrep" << "bzfgrep" << "bzgrep"
	    << "bzip2" << "bzip2recover" << "bzless" << "bzmore" << "cat"
	    << "chattr" << "chgrp" << "chmod" << "chown" << "chvt" << "cp" << "date"
	    << "dd" << "deallocvt" << "df" << "dir" << "dircolors" << "dmesg"
	    << "dnsdomainname" << "domainname" << "du" << "dumpkeys" << "echo"
	    << "ed" << "egrep" << "false" << "fgconsole" << "fgrep" << "fuser"
	    << "gawk" << "getkeycodes" << "gocr" << "grep" << "groff" << "groups"
	    << "gunzip" << "gzexe" << "gzip" << "hostname" << "igawk" << "install"
	    << "kbd_mode" << "kbdrate" << "killall" << "last" << "lastb" << "link"
	    << "ln" << "loadkeys" << "loadunimap" << "login" << "ls" << "lsattr"
	    << "lsmod" << "lsmod.old" << "lzcat" << "lzcmp" << "lzdiff"
	    << "lzegrep" << "lzfgrep" << "lzgrep" << "lzless" << "lzcat" << "lzma"
	    << "lzmainfo" << "lzmore" << "mapscrn" << "mesg" << "mkdir" << "mkfifo"
	    << "mknod" << "mktemp" << "more" << "mount" << "mv" << "nano"
	    << "netstat" << "nisdomainname" << "nroff" << "openvt" << "pgawk"
	    << "pidof" << "ping" << "ps" << "pstree" << "pwd" << "rbash"
	    << "readlink" << "red" << "resizecons" << "rm" << "rmdir"
	    << "run-parts" << "sash" << "sed" << "setfont" << "setkeycodes"
	    << "setleds" << "setmetamode" << "setserial" << "sh" << "showkey"
	    << "shred" << "sleep" << "ssed" << "stat" << "stty" << "su"
	    << "sync" << "tar" << "tempfile" << "touch" << "troff" << "true"
	    << "umount" << "uname" << "unicode_start" << "unicode_stop"
	    << "unlink" << "unlzma" << "unxz" << "utmpdump" << "uuidgen"
	    << "vdir" << "wall" << "wc" << "xz" << "xzcat" << "ypdomainname"
	    << "zcat" << "zcmp" << "zdiff" << "zegrep" << "zfgrep" << "zforce"
	    << "zgrep" << "zless" << "zmore" << "znew" << "zsh";

	__operators = QStringList() << "=" <<
	    // Comparison
	    "==" << "!=" << "<" << "<=" << ">" << ">=" <<
	    // Arithmetic
	    "\\+" << "-" << "\\*" << "/" << "//" << "%" << "\\*\\*" <<
	    // In-place
	    "\\+=" << "-=" << "\\*=" << "/=" << "%=" <<
	    // Bitwise
	    "\\^" << "\\|" << "&" << "~" << ">>" << "<<";

	__braces = QStringList() << "{" << "}" << "\\(" << "\\)" << "\\[" << "]";

	__basicStyles.insert("keyword", getTextCharFormat(Qt::darkRed, "bold"));
	__basicStyles.insert("operator", getTextCharFormat(Qt::red));
	__basicStyles.insert("builtins", getTextCharFormat(Qt::darkBlue, "bold"));
	__basicStyles.insert("builtinsVar", getTextCharFormat(Qt::blue, "bold"));
	__basicStyles.insert("unixCommands", getTextCharFormat(Qt::darkRed, "bold"));
	__basicStyles.insert("brace", getTextCharFormat(Qt::darkRed, "bold"));
	__basicStyles.insert("string", getTextCharFormat(Qt::magenta));
	__basicStyles.insert("string2", getTextCharFormat(Qt::darkMagenta));
	__basicStyles.insert("comment", getTextCharFormat(Qt::blue));
	__basicStyles.insert("numbers", getTextCharFormat(Qt::blue));
	__basicStyles.insert("variable", getTextCharFormat(Qt::black, "bold"));

	triSingleQuote.setPattern("'''");
	triDoubleQuote.setPattern("\"\"\"");

	initializeRules();
}

void BashHighlighter::initializeRules() {

	// Double-quoted string, possibly containing escape sequences
	// FF: originally in python : r'"[^"\\]*(\\.[^"\\]*)*"'
	__rules.append(BashHighlightingRule("\"[^\"\\\\]*(\\\\.[^\"\\\\]*)*\"", 0, __basicStyles.value("string")));

//	__rules.append(BashHighlightingRule("\\{(.*)\\}", 0, __basicStyles.value("variable")));
//	__rules.append(BashHighlightingRule("\\{[A-Za-z](.*)\\}", 0, __basicStyles.value("variable")));

	for (QStringList::iterator it = __keywords.begin();
	        it != __keywords.end(); ++it) {
		__rules.append(BashHighlightingRule(QString("\\b%1\\b").arg((*it)), 0,
		    __basicStyles.value("keyword")));
	}

	for (QStringList::iterator it = __builtins.begin();
	        it != __builtins.end(); ++it) {
		__rules.append(BashHighlightingRule(QString("\\b%1\\b").arg((*it)), 0,
		    __basicStyles.value("builtins")));
	}

	for (QStringList::iterator it = __builtinsVar.begin();
	        it != __builtinsVar.end(); ++it) {
		__rules.append(BashHighlightingRule(QString("\\b%1\\b").arg((*it)), 0,
		    __basicStyles.value("builtinsVar")));
	}

	for (QStringList::iterator it = __unixCommands.begin();
	        it != __unixCommands.end(); ++it) {
		__rules.append(BashHighlightingRule(QString("\\b%1\\b").arg((*it)), 0,
		    __basicStyles.value("unixCommands")));
	}

	for (QStringList::iterator it = __operators.begin();
	        it != __operators.end(); ++it) {
		__rules.append(BashHighlightingRule(QString("%1").arg((*it)), 0,
		    __basicStyles.value("operator")));
	}

	for (QStringList::iterator it = __braces.begin();
	        it != __braces.end(); ++it) {
		__rules.append(BashHighlightingRule(QString("%1").arg((*it)), 0,
		    __basicStyles.value("brace")));
	}

	// From '#' until a newline
	// FF: originally: r'#[^\\n]*'
	__rules.append(BashHighlightingRule("#[^\\n]*", 0, __basicStyles.value("comment")));

	// Numeric literals
	__rules.append(BashHighlightingRule("\\b[+-]?[0-9]+[lL]?\\b", 0, __basicStyles.value("numbers"))); // r'\b[+-]?[0-9]+[lL]?\b'
	__rules.append(BashHighlightingRule("\\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\\b", 0, __basicStyles.value("numbers"))); // r'\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\b'
	__rules.append(BashHighlightingRule("\\b[+-]?[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\\b", 0, __basicStyles.value("numbers"))); // r'\b[+-]?[0-9]+(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\b'
}

void BashHighlighter::highlightBlock(const QString& text) {

	for (RuleList::iterator it = __rules.begin();
	        it != __rules.end(); ++it) {
		int idx = (*it).pattern.indexIn(text, 0);
		while ( idx >= 0 ) {
			// Get index of Nth match
			idx = (*it).pattern.pos((*it).nth);
			int length = (*it).pattern.cap((*it).nth).length();
			setFormat(idx, length, (*it).format);
			idx = (*it).pattern.indexIn(text, idx + length);
		}
	}

	setCurrentBlockState(0);

	// Do multi-line strings
	bool isInMultilne = matchMultiline(text, triSingleQuote, 1, __basicStyles.value("string2"));
	if ( !isInMultilne )
	    isInMultilne = matchMultiline(text, triDoubleQuote, 2, __basicStyles.value("string2"));
}

bool BashHighlighter::matchMultiline(const QString& text,
                                     const QRegExp& delimiter,
                                     const int inState,
                                     const QTextCharFormat& style) {

	int start = -1;
	int add = -1;
	int end = -1;
	int length = 0;

	// If inside triple-single quotes, start at 0
	if ( previousBlockState() == inState ) {
		start = 0;
		add = 0;
	}
	// Otherwise, look for the delimiter on this line
	else {
		start = delimiter.indexIn(text);
		// Move past this match
		add = delimiter.matchedLength();
	}

	// As long as there's a delimiter match on this line...
	while ( start >= 0 ) {
		// Look for the ending delimiter
		end = delimiter.indexIn(text, start + add);
		// Ending delimiter on this line?
		if ( end >= add ) {
			length = end - start + add + delimiter.matchedLength();
			setCurrentBlockState(0);
		}
		// No; multi-line string
		else {
			setCurrentBlockState(inState);
			length = text.length() - start + add;
		}
		// Apply formatting and look for next
		setFormat(start, length, style);
		start = delimiter.indexIn(text, start + length);
	}

	// Return True if still inside a multi-line string, False otherwise
	if ( currentBlockState() == inState )
		return true;
	else
		return false;
}


const QTextCharFormat
BashHighlighter::getTextCharFormat(const QColor& color,
                                   const QString& style) {

	QTextCharFormat charFormat;
	charFormat.setForeground(color);

	if ( style.contains("bold", Qt::CaseInsensitive) )
	    charFormat.setFontWeight(QFont::Bold);

	if ( style.contains("italic", Qt::CaseInsensitive) )
	    charFormat.setFontItalic(true);

	return charFormat;
}

} // namespace Qt4
} // namespace SDP
