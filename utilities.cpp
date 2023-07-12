#include "utilities.h"

#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

Utilities::Utilities()
{
}

QString Utilities::GetDataPath()
{
    QStringList moviesLocation = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation);
    QString userMoviePath = moviesLocation[0];
    QDir movieDir(userMoviePath);
    movieDir.mkpath("Gazer");
    return movieDir.absoluteFilePath("Gazer");
}

QString Utilities::NewSavedVideoName()
{
    QDateTime time = QDateTime::currentDateTime();
    return time.toString("yyyy-MM-dd+HH:mm:ss");
}

QString Utilities::GetSavedVideoPath(QString name, QString postfix)
{
    return QString("%1/%2.%3").arg(Utilities::GetDataPath(), name, postfix);
}
