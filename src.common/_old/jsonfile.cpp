#include <QFile>
#include <QDebug>

#include <qjson/parser.h>
#include <qjson/serializer.h>
#include <qjson/qobjecthelper.h>

#include "jsonfile.h"
//==================================================================================

JsonFile::JsonFile(const QString &fileName, void *userData, bool stopOnError)
	: m_fileName(fileName)
	, m_userData(userData)
	, m_stopOnError(stopOnError)
{
}

JsonFile::~JsonFile()
{
}

void JsonFile::addKey(const QString key, JsnLoadItem loadItem,
					  JsnSaveItem saveItem, JsnSaveList saveList)
{
	m_items.append(JsnItem(key, loadItem, saveItem, saveList));
}

bool JsonFile::locate(JsnLoadItem &load, const QString &key)
{
	foreach (JsnItem item, m_items) {
		if (item.key == key) {
			load = item.loadItem;
			return true;
		}
	}
	return false;
}

//==================================================================================
// load
//==================================================================================

bool JsonFile::parseItem(const QString &key, const QVariantMap &map)
{
	JsnLoadItem load;
	if (!locate(load, key)) return false;

	return load(map, m_userData);
}

bool JsonFile::parseList(const QString &key, const QVariantList &list)
{
	JsnLoadItem load;
	if (!locate(load, key)) return false;

	foreach (QVariant var, list)
	{
		if (var.type() != QVariant::Map) {
			qDebug() << "JsonFile::parseList(): var.type() != QVariant::Map" << var;
			if (m_stopOnError) return false;
			continue;
		}

		bool ok = load(var.toMap(), m_userData);
		if (!ok) {
			qDebug() << "JsonFile::parseList(): ERROR while loading item" << var;
			if (m_stopOnError) return false;
		}
	}

	return true;
}
//==================================================================================

bool JsonFile::parse(const QByteArray &data)
{
	QJson::Parser parser;
	bool ok;
	QVariant result = parser.parse(data, &ok);

	if (!ok || result.type() != QVariant::Map) {
		qDebug() << "JsonFile::parse(): Failed";
		return false;
	}

	QVariantMap map = result.toMap();
	QVariantMap::const_iterator i = map.constBegin();

	while (i != map.constEnd())
	{
		QString key = i.key();
		QVariant value = i.value();
		QVariant::Type type = value.type();

		if      (type == QVariant::Map)  ok = parseItem(key, value.toMap());
		else if (type == QVariant::List) ok = parseList(key, value.toList());
		else                             ok = false;

		if (!ok) {
			qDebug() << "JsonFile::parse(): ERROR key=" << i.key();
			if (m_stopOnError) return false;
		}
		++i;
	}

	return true;
}
//==================================================================================

bool JsonFile::loadFile()
{
	QFile file(m_fileName);

	if (!file.exists()) {
		qDebug() << "JsonFile::loadFile(): NOT exists file=" << m_fileName;
		return false;
	}

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "JsonFile::loadFile(): ERROR to open file=" << m_fileName;
		return false;
	}

	QByteArray data = file.readAll();
	file.close();

	return parse(data);
}

//==================================================================================
// save
//==================================================================================

bool JsonFile::buildItem(QVariantMap &result, const JsnItem &item)
{
	QVariantMap map;
	bool ok = item.saveItem(map, m_userData);
	if (ok) result.insert(item.key, map);
	return ok;
}

bool JsonFile::buildList(QVariantMap &result, const JsnItem &item)
{
	QVariantList list;
	bool ok = item.saveList(list, m_userData);
	if (ok) result.insert(item.key, list);
	return ok;
}

bool JsonFile::build(QByteArray &data)
{
	QVariantMap map;
	bool ok;

	foreach (JsnItem item, m_items)
	{
		if      (item.saveList) ok = buildList(map, item);
		else if (item.saveItem) ok = buildItem(map, item);
		else                    ok = false;

		if (!ok) {
			qDebug() << "JsonFile::build(): ERROR while building item" << item.key;
			if (m_stopOnError) return false;
		}
	}

	QJson::Serializer serializer;
	data = serializer.serialize(map);

	return true;
}
//==================================================================================

bool JsonFile::saveFile()
{
	QFile file(m_fileName);
#if 0
	if (file.exists()) {
		qDebug() << "JsonFile::saveFile(): File will be overwrited:" << m_fileName;
	}
#endif
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "JsonFile::saveFile(): ERROR to open file=" << m_fileName;
		return false;
	}

	QByteArray data;
	bool ok = build(data);
	if (ok) {
		file.write(data);
	}
	file.close();

	return ok;
}
//==================================================================================
