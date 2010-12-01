/*
 * Copyright(c) 2009 by Zurcher Jérémy
 *
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */



#include <hydrogen/helpers/xml.h>
#include <QtCore/QLocale>
#include <QtCore/QFile>
#include <QtXmlPatterns/QXmlSchema>
#include <QtXmlPatterns/QXmlSchemaValidator>

namespace H2Core
{

const char* XMLNode::__class_name ="XMLNode";

XMLNode::XMLNode() : Object(__class_name) { }
XMLNode::XMLNode( QDomNode node ) : Object(__class_name), QDomNode(node) { }

QString XMLNode::read_child_node( const QString& node, bool inexistent_ok, bool empty_ok ) {
    if( isNull() ) {
        DEBUGLOG( QString("try to read %1 XML node from an empty parent %2.").arg(node).arg(nodeName()) );
        return 0;
    }
    QDomElement el = firstChildElement( node );
    if( el.isNull() ) {
        if(!inexistent_ok) DEBUGLOG( QString("XML node %1->%2 should exists.").arg(nodeName()).arg(node) );
        return 0;
    }
    if( el.text().isEmpty() ) {
        if(!empty_ok) DEBUGLOG( QString("XML node %1->%2 should not be empty.").arg(nodeName()).arg(node) );
        return 0;
    }
    return el.text();
}

QString XMLNode::read_string( const QString& node, const QString& default_value, bool inexistent_ok, bool empty_ok ) {
    QString ret = read_child_node( node, inexistent_ok, empty_ok );
    if( ret.isNull() ) {
        DEBUGLOG( QString("Using default value %1 for %2").arg(default_value).arg(node) );
        return default_value;
    }
    return ret;
}

float XMLNode::read_float( const QString& node, float default_value, bool inexistent_ok, bool empty_ok ) {
    QString ret = read_child_node( node, inexistent_ok, empty_ok );
    if( ret.isNull() ) {
        DEBUGLOG( QString("Using default value %1 for %2").arg(default_value).arg(node) );
        return default_value;
    }
    QLocale c_locale = QLocale::c();
    return c_locale.toFloat( ret );
}

int XMLNode::read_int( const QString& node, int default_value, bool inexistent_ok, bool empty_ok ) {
    QString ret = read_child_node( node, inexistent_ok, empty_ok );
    if( ret.isNull() ) {
        DEBUGLOG( QString("Using default value %1 for %2").arg(default_value).arg(node) );
        return default_value;
    }
    QLocale c_locale = QLocale::c();
    return c_locale.toInt( ret );
}

bool XMLNode::read_bool( const QString& node, bool default_value, bool inexistent_ok, bool empty_ok ) {
    QString ret = read_child_node( node, inexistent_ok, empty_ok );
    if( ret.isNull() ) {
        DEBUGLOG( QString("Using default value %1 for %2").arg(default_value).arg(node) );
        return default_value;
    }
    if(ret=="true") { return true; } else { return false; }
}

void XMLNode::write_child_node( const QString& node, const QString& text ) {
    QDomDocument doc;
    QDomElement el = doc.createElement( node );
    el.appendChild( doc.createTextNode( text ) );
    appendChild( el );
}
void XMLNode::write_string( const QString& node, const QString& value ) { write_child_node( node, value ); }
void XMLNode::write_float( const QString& node, const float value ) { write_child_node( node, QString::number( value ) ); }
void XMLNode::write_int( const QString& node, const int value ) { write_child_node( node, QString::number( value ) ); }
void XMLNode::write_bool( const QString& name, const bool value ) { write_child_node( name, QString( ( value ? "true" : "false" ) ) ); }

const char* XMLDoc::__class_name ="XMLDoc";

XMLDoc::XMLDoc( ) : Object(__class_name) { }

bool XMLDoc::read( const QString& filepath, const QString& schemapath ) {
    QXmlSchema schema;
    if( schemapath!=0 ) {
        QFile file(schemapath);
        if ( !file.open(QIODevice::ReadOnly) ) {
            ERRORLOG( QString("Unable to open xml schema %1 for reading").arg(schemapath) );
        } else {
            schema.load(&file, QUrl::fromLocalFile(file.fileName()));
            file.close();
        }
    }
    QFile file( filepath );
    if ( !file.open(QIODevice::ReadOnly) ) {
        ERRORLOG( QString("Unable to open %1 for reading").arg(filepath) );
        return false;
    }
    if ( schemapath!=0 ) {
        if ( schema.isValid() ) {
            QXmlSchemaValidator validator(schema);
            if ( !validator.validate(&file, QUrl::fromLocalFile(file.fileName())) ) {
                ERRORLOG( QString("XML document %1 is not valid (%2)").arg(filepath).arg(schemapath) );
                return false;
            }
        } else {
            WARNINGLOG( QString("%2 XML schema is not valid").arg(schemapath) );
        }
    }
    if( !setContent( &file ) ) {
        ERRORLOG( QString("Unable to read XML document %1").arg(filepath) );
        return false;
    }
    file.close();
    return true;
}

bool XMLDoc::write( const QString& filepath ) {
    QFile file( filepath );
    if ( !file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) ) {
        ERRORLOG( QString("Unable to open %1 for writting").arg(filepath) );
        return false;
    }
    QTextStream out(&file);
    out << toString().toUtf8();
    file.close();
    return true;
};

};

/* vim: set softtabstop=4 expandtab: */

