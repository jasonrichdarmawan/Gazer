#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>


class Utilities
{
public:
    // returns the directory in which we will save the video files.
    static QString GetDataPath();
    // generates a new name for the video that will be saved.
    static QString NewSavedVideoName();
    // accepts a name and a postfix (the extension name)
    // returns the absolute path of the video file with the given name.
    static QString GetSavedVideoPath(QString name, QString postfix);

public:
    Utilities();
};

#endif // UTILITIES_H
