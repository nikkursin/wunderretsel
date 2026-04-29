#ifndef APPSTATEMANAGER_H
#define APPSTATEMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include <QVariantList>
#include <QVariantMap>

#include "Models/EntriesData.h"
#include "Models/UserData.h"
#include "StorageManager.h"
#include "PuzzleManager.h"

class AppStateManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Screen currentScreen READ currentScreen NOTIFY currentScreenChanged)
    Q_PROPERTY(QString languageLevel READ languageLevel WRITE setLanguageLevel NOTIFY languageLevelChanged)
    Q_PROPERTY(QString characterType READ characterType WRITE setCharacterType NOTIFY characterTypeChanged)
    Q_PROPERTY(QVariantMap currentPuzzle
                   READ currentPuzzle
                       NOTIFY currentPuzzleChanged)
    Q_PROPERTY(QString themeTintLight READ themeTintLight NOTIFY characterTypeChanged)
    Q_PROPERTY(QString themeTintMid READ themeTintMid NOTIFY characterTypeChanged)
    Q_PROPERTY(QString themeTintDeep READ themeTintDeep NOTIFY characterTypeChanged)
    Q_PROPERTY(QString themeAccentStart READ themeAccentStart NOTIFY characterTypeChanged)
    Q_PROPERTY(QString themeAccentEnd READ themeAccentEnd NOTIFY characterTypeChanged)
    Q_PROPERTY(QString themeAccentSoftStart READ themeAccentSoftStart NOTIFY characterTypeChanged)
    Q_PROPERTY(QString themeAccentSoftEnd READ themeAccentSoftEnd NOTIFY characterTypeChanged)
    Q_PROPERTY(QString themeTextPrimary READ themeTextPrimary NOTIFY characterTypeChanged)
    Q_PROPERTY(QString themeTextSecondary READ themeTextSecondary NOTIFY characterTypeChanged)
    Q_PROPERTY(QString themeTextStrong READ themeTextStrong NOTIFY characterTypeChanged)
    Q_PROPERTY(QString themeTileBase READ themeTileBase NOTIFY characterTypeChanged)
    Q_PROPERTY(QString themeTileVeil READ themeTileVeil NOTIFY characterTypeChanged)
    Q_PROPERTY(QString themeTileBorder READ themeTileBorder NOTIFY characterTypeChanged)
    Q_PROPERTY(QString themeControlBorder READ themeControlBorder NOTIFY characterTypeChanged)
    Q_PROPERTY(QString themeControlText READ themeControlText NOTIFY characterTypeChanged)

    // Gallery feed for QML. The list is recomputed whenever the
    // user's character preference changes or a puzzle unlock happens,
    // so the view never has to track unlock state itself.
    Q_PROPERTY(QVariantList galleryImages
                   READ galleryImages
                       NOTIFY galleryImagesChanged)
    Q_PROPERTY(int unlockedImagesCount
                   READ unlockedImagesCount
                       NOTIFY galleryImagesChanged)
    Q_PROPERTY(int totalImagesCount
                   READ totalImagesCount
                       NOTIFY galleryImagesChanged)

  public:
    enum Screen {
        Home,
        Play,
        Gallery,
        Settings,
        Onboarding
    };
    Q_ENUM(Screen)

    explicit AppStateManager(QSharedPointer<StorageManager> storageManager, QObject *parent = nullptr);

    bool init();

    Screen currentScreen() const;
    bool onboardingCompleted() const;

    QString languageLevel() const;
    void setLanguageLevel(const QString& level);

    QString characterType() const;
    void setCharacterType(const QString& type);
    QString themeTintLight() const;
    QString themeTintMid() const;
    QString themeTintDeep() const;
    QString themeAccentStart() const;
    QString themeAccentEnd() const;
    QString themeAccentSoftStart() const;
    QString themeAccentSoftEnd() const;
    QString themeTextPrimary() const;
    QString themeTextSecondary() const;
    QString themeTextStrong() const;
    QString themeTileBase() const;
    QString themeTileVeil() const;
    QString themeTileBorder() const;
    QString themeControlBorder() const;
    QString themeControlText() const;

    QVariantMap currentPuzzle() const;

    // Gallery accessors (read-only from QML).
    //   galleryImages       – list of { id, source, unlocked } maps for
    //                         every image visible under the user's
    //                         current character preference.
    //   unlockedImagesCount – number of those images that the user has
    //                         already revealed.
    //   totalImagesCount    – size of `galleryImages`. Both counters use
    //                         the same denominator so "x / y" is always
    //                         consistent, including after a preference
    //                         change.
    QVariantList galleryImages() const;
    int unlockedImagesCount() const;
    int totalImagesCount() const;

    void generateNewPuzzle();

    Q_INVOKABLE void goHome();
    Q_INVOKABLE void goPlay();
    Q_INVOKABLE void goGallery();
    Q_INVOKABLE void goSettings();
    Q_INVOKABLE void goOnboarding();
    Q_INVOKABLE void goBack();

    Q_INVOKABLE void completeOnboarding(const QString &languageLevel,
                                        const QString &characterType);

    Q_INVOKABLE void savePreferences();

    // Called by QML when the player finishes the current puzzle.
    // Increments the solved-puzzle counter (drives difficulty scaling)
    // and persists user state.
    Q_INVOKABLE void notifyPuzzleSolved();

  signals:
    void currentScreenChanged();
    void languageLevelChanged();
    void characterTypeChanged();
    void currentPuzzleChanged();
    void galleryImagesChanged();

  private:
    void navigateTo(const Screen& screen);

    bool ongoingPuzzlePresent() const;

    // Re-load the gallery image cache from StorageManager using the
    // user's currently-selected character preference, then notify QML.
    // Cheap: images.json is small and parsed once per call.
    void refreshGalleryImages();

    UserData m_userData;
    QSharedPointer<StorageManager> m_storageManager;
    PuzzleManager m_puzzleManager;

    Screen m_currentScreen = Home;
    QVector<Screen> m_history;
    GeneratedPuzzle m_currentPuzzle;
    bool hasOngoingPuzzle = false;

    // Cached gallery feed for the current character preference.
    // Kept in C++ so QML never has to filter / look up unlock state.
    QVector<ImageEntry> m_galleryImages;

};

#endif // APPSTATEMANAGER_H
