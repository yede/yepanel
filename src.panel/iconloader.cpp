#include "iconloader.h"

#include <stdlib.h>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtGui/QIcon>
//==================================================================================

struct IconDirectory
{
	QString m_path;
	int m_size;
	bool m_scalable;
};

void IconTheme::init(const QString &themeName)
{
	m_themeName = themeName;

	foreach (const QString &searchPath, IconLoader::instance()->iconSearchPaths())
	{
		QFile file(searchPath + "/" + m_themeName + "/index.theme");

		if (!file.exists())
			continue;

		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			continue;

		QTextStream in(&file);
		QString context;
		while (!in.atEnd())
		{
			QString line = in.readLine();
			if (line.isEmpty())
				continue;
			if (line[0] == '#')
				continue;
			if (line[0] == '[')
			{
				if (line.size() < 3)
					continue;
				context = line.mid(1, line.size() - 2);
				if (context != "Icon Theme")
				{
					IconDirectory dir;
					dir.m_path = context;
					dir.m_size = 0;
					dir.m_scalable = false;
					m_iconDirs.append(dir);
				}
			}

			QStringList list = line.split('=');
			if (list.size() < 2)
				continue;
			QString key = list[0];
			QString value = list[1];

			if (key == "Inherits")
				m_inheritedThemes = value.split(',');

			if (key == "Size")
			{
				m_iconDirs.last().m_size = value.toInt();
			}

			if (key == "Type")
			{
				if (value.compare("Scalable", Qt::CaseInsensitive) == 0)
					m_iconDirs.last().m_scalable = true;
			}
		}
	}
}

bool IconTheme::loadIconFromDirectory(QImage &result, const IconDirectory &iconDir,
									  const QString &fileName)
{
	foreach (const QString &searchPath, IconLoader::instance()->iconSearchPaths())
	{
		QString iconFileName = searchPath + "/" + m_themeName + "/" +
							   iconDir.m_path + "/" + fileName;
		if (!QFile::exists(iconFileName))
			continue;
		result.load(iconFileName);
		if (!result.isNull())
			return true;
	}
	return false;
}

bool IconTheme::loadIcon(QImage &result, const QString &iconName, int size)
{
	QString fileName = iconName;
	if (!fileName.endsWith(".png"))
		fileName.append(".png");

	foreach (const IconDirectory &iconDir, m_iconDirs)
	{
		if (!iconDir.m_scalable && iconDir.m_size == size)
		{
			if (loadIconFromDirectory(result, iconDir, fileName))
				return true;
		}
	}

	foreach (const IconDirectory &iconDir, m_iconDirs)
	{
		if (!iconDir.m_scalable && iconDir.m_size > size)
		{
			if (loadIconFromDirectory(result, iconDir, fileName))
				return true;
		}
	}

	foreach (const IconDirectory &iconDir, m_iconDirs)
	{
		if (!iconDir.m_scalable)
		{
			if (loadIconFromDirectory(result, iconDir, fileName))
				return true;
		}
	}

	return false;
}

//==================================================================================
// IconLoader
//==================================================================================

IconLoader::IconLoader()
{
	m_instance = this;

	m_iconSearchPaths.append(QString(getenv("HOME")) + "/.icons");

	QString xdgDataDirs;
	char *xdgDataDirsEnv = getenv("XDG_DATA_DIRS");
	if (xdgDataDirsEnv != NULL)
		xdgDataDirs = xdgDataDirsEnv;
	else
		xdgDataDirs = "/usr/local/share/:/usr/share/";

	QStringList dirs = xdgDataDirs.split(':');
	foreach (const QString &dir, dirs)
	{
		m_iconSearchPaths.append(dir + "icons");
	}
}

IconLoader::~IconLoader()
{
	m_instance = NULL;
}

void IconLoader::scaleIcon(QImage &image, int size)
{
	if (!image.isNull() && (image.width() != size || image.height() != size))
		image = image.scaled(size, size,
							 Qt::KeepAspectRatio,
							 Qt::SmoothTransformation);
}

QImage IconLoader::defaultIcon(int size)
{
	QImage result;
	result.load(":/22/default");
	scaleIcon(result, size);

	return result;
}

QImage IconLoader::loadIcon(const QString &iconName, int size)
{
	if (iconName.isEmpty())
		return defaultIcon(size);

	QImage result;
	bool ok = m_instance->loadIconFromTheme(result, QIcon::themeName(),
											iconName, size);
	if (!ok)
		result.load("/usr/share/pixmaps/" + iconName);

	if (result.isNull())
		result.load(iconName);

	if (result.isNull())
		result.load(":/22/default");

	scaleIcon(result, size);

	return result;
}

QImage IconLoader::loadFromFile(const QString &iconFile, int size)
{
	if (iconFile.isEmpty())
		return defaultIcon(size);

	QImage result;
	result.load(iconFile);

	if (result.isNull())
		result.load(":/22/default");

	scaleIcon(result, size);

	return result;
}

bool IconLoader::loadIconFromTheme(QImage &result, const QString &themeName,
								   const QString &iconName, int size)
{
	if (!m_iconThemes.contains(themeName))
	{
		m_iconThemesMutex.lock();
		m_iconThemes[themeName].init(themeName);
		m_iconThemesMutex.unlock();
	}

	bool ok = m_iconThemes[themeName].loadIcon(result, iconName, size);

	if (!ok && themeName != "hicolor")
	{
		QStringList inheritedThemes = m_iconThemes[themeName].inheritedThemes();
		if (inheritedThemes.empty())
			inheritedThemes.append("hicolor");
		foreach (const QString &inheritedTheme, inheritedThemes)
		{
			ok = loadIconFromTheme(result, inheritedTheme, iconName, size);
			if (ok)
				break;
		}
	}

	return ok;
}
