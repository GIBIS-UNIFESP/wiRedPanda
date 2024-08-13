#include "settings.h"

QSettings *Settings::settings = nullptr;

QSettings *Settings::settingsInstance()
{
    if (!settings)
    {
        settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "GIBIS-UNIFESP", "wiRedPanda");
    }
    return settings;
}

QString Settings::fileName()
{
    return settingsInstance()->fileName();
}

QVariant Settings::value(const QString &key)
{
    return settingsInstance()->value(key);
}

bool Settings::contains(const QString &key)
{
    return settingsInstance()->contains(key);
}

void Settings::remove(const QString &key)
{
    settingsInstance()->remove(key);
}

void Settings::setValue(const QString &key, const QVariant &value)
{
    settingsInstance()->setValue(key, value);
}
