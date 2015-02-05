#include <QFile>
#include <QDebug>

#include "cJSON.h"
#include "jsonfile.h"
//==================================================================================

JsonFile::JsonFile(const QString &fileName, void *userData)
	: m_fileName(fileName)
	, m_userData(userData)
{
}

JsonFile::~JsonFile()
{
}

//==================================================================================
// load
//==================================================================================

static bool isKey(const char *key, cJSON *node)
{
	if (!key || !node->string) return false;

	size_t len = strlen(key);
	if (strlen(node->string) != len) return false;

	return memcmp(node->string, key, len) == 0;
}

void JsonFile::loadItem(cJSON *obj, void *parent)
{
	cJSON *textNode = cJSON_GetObjectItem(obj, "text");
	cJSON *iconNode = cJSON_GetObjectItem(obj, "icon");
	cJSON *execNode = cJSON_GetObjectItem(obj, "exec");

	QString title, icon, exec;
	if (textNode) title = QString::fromUtf8(textNode->valuestring);
	if (iconNode) icon  = QString::fromUtf8(iconNode->valuestring);
	if (execNode) exec  = QString::fromUtf8(execNode->valuestring);

	m_addItem(parent, title, icon, exec, m_userData);
}

void JsonFile::loadMenu(cJSON *obj, void *parent)
{
	cJSON *textNode = cJSON_GetObjectItem(obj, "text");
	cJSON *iconNode = cJSON_GetObjectItem(obj, "icon");

	QString title, icon;
	if (textNode) title = QString::fromUtf8(textNode->valuestring);
	if (iconNode) icon  = QString::fromUtf8(iconNode->valuestring);

	void *menu = m_addMenu(parent, title, icon, m_userData);
	cJSON *child = obj->child;

	if      (iconNode) child = iconNode->next;
	else if (textNode) child = textNode->next;

	parse(child, menu);
}

void JsonFile::parse(cJSON *obj, void *parent)
{
	while (obj) {
		switch (obj->type) {
			case cJSON_False :
			case cJSON_True  :
			case cJSON_NULL  :
			case cJSON_Number:
			case cJSON_String: if (isKey("sep", obj)) m_addSep(parent, m_userData);  break;
			case cJSON_Array : break;
			case cJSON_Object: if      (isKey("menu", obj)) loadMenu(obj, parent);
							   else if (isKey("item", obj)) loadItem(obj, parent);
							   break;
		}
		obj = obj->next;
	}
}
//==================================================================================

bool JsonFile::loadFile(JsnAddMenu addMenu, JsnAddItem addItem, JsnAddSep addSep)
{
	m_addMenu = addMenu;
	m_addItem = addItem;
	m_addSep  = addSep;

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

	cJSON *root = cJSON_Parse(data.constData());
	if (!root) {
		qDebug() << "JsonFile::loadFile(): cJSON_Parse() Failed";
		return false;
	}
//	char *out = cJSON_Print(root);
//	qDebug("loadFile:\n%s\n", out);

	if (root->type == cJSON_Object) parse(root->child, 0);
	cJSON_Delete(root);

	return true;
}

//==================================================================================
// save
//==================================================================================

void JsonFile::saveSep(cJSON *obj)
{
	cJSON_AddStringToObject(obj, "sep", "");
}

void JsonFile::saveItem(cJSON *obj, const char *title, const char *icon, const char *exec)
{
	cJSON *item = cJSON_CreateObject();
	cJSON_AddStringToObject(item, "text", title);
	cJSON_AddStringToObject(item, "icon", icon);
	cJSON_AddStringToObject(item, "exec", exec);

	cJSON_AddItemToObject(obj, "item", item);
}

cJSON *JsonFile::saveMenu(cJSON *obj, const char *title, const char *icon)
{
	cJSON *menu = cJSON_CreateObject();
	cJSON_AddStringToObject(menu, "text", title);
	cJSON_AddStringToObject(menu, "icon", icon);

	cJSON_AddItemToObject(obj, "menu", menu);

	return menu;
}
//==================================================================================

bool JsonFile::saveFile(JsnSavAll saveAll)
{
	QFile file(m_fileName);

	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "JsonFile::saveFile(): ERROR to open file=" << m_fileName;
		return false;
	}

	cJSON *root = cJSON_CreateObject();
	saveAll(root, m_userData);

	char *out = cJSON_Print(root);
//	qDebug("saveFile:\n%s\n", out);
	if (out) {
		QByteArray data = QByteArray::fromRawData(out, strlen(out));
		file.write(data);
	}

	free(out);
	cJSON_Delete(root);
	file.close();

	return true;
}
//==================================================================================
