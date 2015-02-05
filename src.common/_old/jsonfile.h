#ifndef JSONFILE_H
#define JSONFILE_H

#include <QByteArray>
#include <QVariant>
#include <QString>
#include <QList>
//==================================================================================

typedef bool (*JsnLoadItem) (const QVariantMap &map, void *userData);
typedef bool (*JsnSaveItem) (QVariantMap &result, void *userData);
typedef bool (*JsnSaveList) (QVariantList &result, void *userData);

struct JsnItem
{
	JsnItem(const QString &k, JsnLoadItem li, JsnSaveItem si, JsnSaveList sl)
		: key(k), loadItem(li), saveItem(si), saveList(sl) {}

	QString key;
	JsnLoadItem loadItem;
	JsnSaveItem saveItem;
	JsnSaveList saveList;
};
//==================================================================================

class JsonFile
{
public:
	JsonFile(const QString &fileName, void *userData, bool stopOnError = false);
	~JsonFile();

	void addKey(const QString key, JsnLoadItem loadItem,
				JsnSaveItem saveItem, JsnSaveList saveList);

	bool loadFile();
	bool saveFile();

private:
	bool locate(JsnLoadItem &load, const QString &key);

	bool parseItem(const QString &key, const QVariantMap &map);
	bool parseList(const QString &key, const QVariantList &list);
	bool parse(const QByteArray &data);

	bool buildItem(QVariantMap &result, const JsnItem &item);
	bool buildList(QVariantMap &result, const JsnItem &item);
	bool build(QByteArray &data);

private:
	QList<JsnItem> m_items;
	QString        m_fileName;
	void          *m_userData;
	bool           m_stopOnError;
};

#endif
