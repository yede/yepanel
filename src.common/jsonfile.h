#ifndef JSONFILE_H
#define JSONFILE_H

#include <QByteArray>
#include <QVariant>
#include <QString>
#include <QList>
//==================================================================================

struct cJSON;


typedef void  (*JsnSavAll)  (cJSON *root, void *userData);

typedef void  (*JsnAddSep)  (void *parent, void *userData);
typedef void  (*JsnAddItem) (void *parent, const QString &title, const QString &icon, const QString &exec, void *userData);
typedef void *(*JsnAddMenu) (void *parent, const QString &title, const QString &icon, void *userData);
//==================================================================================

class JsonFile
{
public:
	JsonFile(const QString &fileName, void *userData);
	~JsonFile();

	bool loadFile(JsnAddMenu addMenu, JsnAddItem addItem, JsnAddSep addSep);
	bool saveFile(JsnSavAll saveAll);

	void   saveSep (cJSON *obj);
	void   saveItem(cJSON *obj, const char *title, const char *icon, const char *exec);
	cJSON *saveMenu(cJSON *obj, const char *title, const char *icon);

private:
	void loadItem(cJSON *obj, void *parent);
	void loadMenu(cJSON *obj, void *parent);
	void parse(cJSON *obj, void *parent);

private:
	QString    m_fileName;

	JsnAddMenu m_addMenu;
	JsnAddItem m_addItem;
	JsnAddSep  m_addSep;
	void      *m_userData;
};

#endif
