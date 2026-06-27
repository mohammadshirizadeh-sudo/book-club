// genre.h
#ifndef GENRE_H
#define GENRE_H

#include <QString>
#include <QVector>
#include <QMap>

enum class Genre {
    Fiction,
    NonFiction,
    Science,
    History,
    Biography,
    Fantasy,
    Mystery,
    Romance,
    Horror,
    Comedy,
    Poetry,
    Philosophy,
    Psychology,
    Technology,
    Art,
    Travel,
    Cooking,
    Health,
    Business,
    Education,
    Other
};

class GenreHelper {
public:
    //for convert Genre to QString
    static QString toString(Genre genre) {
        switch(genre) {
        case Genre::Fiction:     return "Fiction";
        case Genre::NonFiction:  return "NonFiction";
        case Genre::Science:     return "Science";
        case Genre::History:     return "History";
        case Genre::Biography:   return "Biography";
        case Genre::Fantasy:     return "Fantasy";
        case Genre::Mystery:     return "Mystery";
        case Genre::Romance:     return "Romance";
        case Genre::Horror:      return "Horror";
        case Genre::Comedy:      return "Comedy";
        case Genre::Poetry:      return "Poetry";
        case Genre::Philosophy:  return "Philosophy";
        case Genre::Psychology:  return "Psychology";
        case Genre::Technology:  return "Technology";
        case Genre::Art:         return "Art";
        case Genre::Travel:      return "Travel";
        case Genre::Cooking:     return "Cooking";
        case Genre::Health:      return "Health";
        case Genre::Business:    return "Business";
        case Genre::Education:   return "Education";
        case Genre::Other:       return "Other";
        default:                 return "Unknown";
        }
    }

    // for convert QString to Genre
    static Genre fromString(const QString& str) {
        if (str == "Fiction")     return Genre::Fiction;
        if (str == "NonFiction")  return Genre::NonFiction;
        if (str == "Science")     return Genre::Science;
        if (str == "History")     return Genre::History;
        if (str == "Biography")   return Genre::Biography;
        if (str == "Fantasy")     return Genre::Fantasy;
        if (str == "Mystery")     return Genre::Mystery;
        if (str == "Romance")     return Genre::Romance;
        if (str == "Horror")      return Genre::Horror;
        if (str == "Comedy")      return Genre::Comedy;
        if (str == "Poetry")      return Genre::Poetry;
        if (str == "Philosophy")  return Genre::Philosophy;
        if (str == "Psychology")  return Genre::Psychology;
        if (str == "Technology")  return Genre::Technology;
        if (str == "Art")         return Genre::Art;
        if (str == "Travel")      return Genre::Travel;
        if (str == "Cooking")     return Genre::Cooking;
        if (str == "Health")      return Genre::Health;
        if (str == "Business")    return Genre::Business;
        if (str == "Education")   return Genre::Education;
        if (str == "Other")       return Genre::Other;
        return Genre::Other;
    }

    //
    static QVector<QString> getAllGenres() {
        static const QVector<QString> all = {
            "Fiction", "NonFiction", "Science", "History",
            "Biography", "Fantasy", "Mystery", "Romance",
            "Horror", "Comedy", "Poetry", "Philosophy",
            "Psychology", "Technology", "Art", "Travel",
            "Cooking", "Health", "Business", "Education", "Other"
        };
        return all;
    }


    static bool isValidGenre(const QString& genre) {
        return getAllGenres().contains(genre);
    }


    static QString toPersian(Genre genre) {
        switch(genre) {
        case Genre::Fiction:     return "داستانی";
        case Genre::NonFiction:  return "غیرداستانی";
        case Genre::Science:     return "علمی";
        case Genre::History:     return "تاریخی";
        case Genre::Biography:   return "بیوگرافی";
        case Genre::Fantasy:     return "فانتزی";
        case Genre::Mystery:     return "معمایی";
        case Genre::Romance:     return "عاشقانه";
        case Genre::Horror:      return "ترسناک";
        case Genre::Comedy:      return "کمدی";
        case Genre::Poetry:      return "شعر";
        case Genre::Philosophy:  return "فلسفه";
        case Genre::Psychology:  return "روان‌شناسی";
        case Genre::Technology:  return "تکنولوژی";
        case Genre::Art:         return "هنر";
        case Genre::Travel:      return "سفرنامه";
        case Genre::Cooking:     return "آشپزی";
        case Genre::Health:      return "سلامتی";
        case Genre::Business:    return "کسب‌وکار";
        case Genre::Education:   return "آموزشی";
        case Genre::Other:       return "سایر";
        default:                 return "نامشخص";
        }
    }

    static QString toPersian(const QString& englishGenre) {
        Genre genre = fromString(englishGenre);
        return toPersian(genre);
    }

    static QVector<QString> getAllGenresPersian() {
        QVector<QString> persianList;
        for (const QString& english : getAllGenres()) {
            persianList.append(toPersian(english));
        }
        return persianList;
    }
};

#endif // GENRE_H