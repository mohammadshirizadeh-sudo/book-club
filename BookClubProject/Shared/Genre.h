// genre.h
#ifndef GENRE_H
#define GENRE_H

#include <QString>
#include <QVector>
#include <QMap>

enum class Genre {



    Adventure,
    Art,
    Biography,
    Comedy,
    Comics,
    Cooking,
    Documentation,
    Drama,
    Education,
    Fantasy,
    Fiction,
    Health,
    History,
    Horror,
    Language_Learning,
    LGBTQ,
    Manga,
    Music,
    Mystery_and_Crime,
    Personal_Development,
    Philosophy,
    Poetry,
    Politics_and_Society,
    Psychology,
    Reference,
    Romance,
    Science,
    Science_Fiction,
    Technology,
    Thriller,
    other

};

class GenreHelper {
public:
    //for convert Genre to QString
    static QString toString(Genre genre)
    {
        switch (genre)
        {
        case Genre::Adventure:               return "Adventure";
        case Genre::Art:                     return "Art";
        case Genre::Biography:               return "Biography";
        case Genre::Comedy:                  return "Comedy";
        case Genre::Comics:                  return "Comics";
        case Genre::Cooking:                 return "Cooking";
        case Genre::Documentation:           return "Documentation";
        case Genre::Drama:                   return "Drama";
        case Genre::Education:               return "Education";
        case Genre::Fantasy:                 return "Fantasy";
        case Genre::Fiction:                 return "Fiction";
        case Genre::Health:                  return "Health";
        case Genre::History:                 return "History";
        case Genre::Horror:                  return "Horror";
        case Genre::Language_Learning:       return "Language_Learning";
        case Genre::LGBTQ:                   return "LGBTQ";
        case Genre::Manga:                   return "Manga";
        case Genre::Music:                   return "Music";
        case Genre::Mystery_and_Crime:       return "Mystery_and_Crime";
        case Genre::Personal_Development:    return "Personal_Development";
        case Genre::Philosophy:              return "Philosophy";
        case Genre::Poetry:                  return "Poetry";
        case Genre::Politics_and_Society:    return "Politics_and_Society";
        case Genre::Psychology:              return "Psychology";
        case Genre::Reference:                return "Refrence";
        case Genre::Romance:                 return "Romance";
        case Genre::Science:                 return "Science";
        case Genre::Science_Fiction:         return "Science_Fiction";
        case Genre::Technology:              return "Technology";
        case Genre::Thriller:                return "Thriller";
        case Genre::other:                   return "other";
        default:                             return "Unknown";
        }
    }

    // for convert QString to Genre
    static Genre fromString(const QString& str)
    {
        if (str == "Adventure")              return Genre::Adventure;
        if (str == "Art")                    return Genre::Art;
        if (str == "Biography")              return Genre::Biography;
        if (str == "Comedy")                 return Genre::Comedy;
        if (str == "Comics")                 return Genre::Comics;
        if (str == "Cooking")                return Genre::Cooking;
        if (str == "Documentation")          return Genre::Documentation;
        if (str == "Drama")                  return Genre::Drama;
        if (str == "Education")              return Genre::Education;
        if (str == "Fantasy")                return Genre::Fantasy;
        if (str == "Fiction")                return Genre::Fiction;
        if (str == "Health")                 return Genre::Health;
        if (str == "History")                return Genre::History;
        if (str == "Horror")                 return Genre::Horror;
        if (str == "Language_Learning")      return Genre::Language_Learning;
        if (str == "LGBTQ")                  return Genre::LGBTQ;
        if (str == "Manga")                  return Genre::Manga;
        if (str == "Music")                  return Genre::Music;
        if (str == "Mystery_and_Crime")      return Genre::Mystery_and_Crime;
        if (str == "Personal_Development")   return Genre::Personal_Development;
        if (str == "Philosophy")             return Genre::Philosophy;
        if (str == "Poetry")                 return Genre::Poetry;
        if (str == "Politics_and_Society")   return Genre::Politics_and_Society;
        if (str == "Psychology")             return Genre::Psychology;
        if (str == "Refrence")               return Genre::Reference;
        if (str == "Romance")                return Genre::Romance;
        if (str == "Science")                return Genre::Science;
        if (str == "Science_Fiction")        return Genre::Science_Fiction;
        if (str == "Technology")             return Genre::Technology;
        if (str == "Thriller")               return Genre::Thriller;
        if (str == "other")                  return Genre::other;

        return Genre::other;
    }

    //
    static QVector<QString> getAllGenres()
    {
        static const QVector<QString> all = {
            "Adventure",
            "Art",
            "Biography",
            "Comedy",
            "Comics",
            "Cooking",
            "Documentation",
            "Drama",
            "Education",
            "Fantasy",
            "Fiction",
            "Health",
            "History",
            "Horror",
            "Language_Learning",
            "LGBTQ",
            "Manga",
            "Music",
            "Mystery_and_Crime",
            "Personal_Development",
            "Philosophy",
            "Poetry",
            "Politics_and_Society",
            "Psychology",
            "Refrence",
            "Romance",
            "Science",
            "Science_Fiction",
            "Technology",
            "Thriller",
            "other"
        };
        return all;
    }


    static bool isValidGenre(const QString& genre) {
        return getAllGenres().contains(genre);
    }


    static QString toPersian(Genre genre)
    {
        switch (genre)
        {
        case Genre::Adventure:               return "ماجراجویی";
        case Genre::Art:                     return "هنر";
        case Genre::Biography:               return "زندگی‌نامه";
        case Genre::Comedy:                  return "کمدی";
        case Genre::Comics:                  return "کمیک";
        case Genre::Cooking:                 return "آشپزی";
        case Genre::Documentation:           return "مستند";
        case Genre::Drama:                   return "درام";
        case Genre::Education:               return "آموزشی";
        case Genre::Fantasy:                 return "فانتزی";
        case Genre::Fiction:                 return "داستانی";
        case Genre::Health:                  return "سلامت";
        case Genre::History:                 return "تاریخی";
        case Genre::Horror:                  return "ترسناک";
        case Genre::Language_Learning:       return "یادگیری زبان";
        case Genre::LGBTQ:                   return "موضوعات LGBTQ+";
        case Genre::Manga:                   return "مانگا";
        case Genre::Music:                   return "موسیقی";
        case Genre::Mystery_and_Crime:       return "معمایی و جنایی";
        case Genre::Personal_Development:    return "توسعه فردی";
        case Genre::Philosophy:              return "فلسفه";
        case Genre::Poetry:                  return "شعر";
        case Genre::Politics_and_Society:    return "سیاست و جامعه";
        case Genre::Psychology:              return "روان‌شناسی";
        case Genre::Reference:                return "مرجع";
        case Genre::Romance:                 return "عاشقانه";
        case Genre::Science:                 return "علمی";
        case Genre::Science_Fiction:         return "علمی‌تخیلی";
        case Genre::Technology:              return "فناوری";
        case Genre::Thriller:                return "هیجان‌انگیز";
        case Genre::other:                   return "سایر";
        default:                             return "نامشخص";
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
    static QVector<Genre> stringListToGenres(const QStringList& genres) {
        QVector<Genre> result;
        for (const QString& genreStr : genres) {
            Genre genre = GenreHelper::fromString(genreStr);
            if (genre != Genre::other || genreStr == "Other") {
                result.append(genre);
            }
        }
        return result;
    }
};

#endif // GENRE_H