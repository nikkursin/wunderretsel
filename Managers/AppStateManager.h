#ifndef APPSTATEMANAGER_H
#define APPSTATEMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include <QVariantList>
#include <QVariantMap>

#include "Models/EntriesData.h"
#include "Models/UserData.h"
#include "PuzzleManager.h"
#include "StorageManager.h"

class AppStateManager : public QObject
{
    Q_OBJECT

    // Theme — all driven by characterType
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

    // Navigation & state
    Q_PROPERTY(Screen currentScreen READ currentScreen NOTIFY currentScreenChanged)
    Q_PROPERTY(QVariantMap currentPuzzle READ currentPuzzle NOTIFY currentPuzzleChanged)
    Q_PROPERTY(QVariantList galleryImages READ galleryImages NOTIFY galleryImagesChanged)

    Q_PROPERTY(int unlockedImagesCount READ unlockedImagesCount NOTIFY galleryImagesChanged)
    Q_PROPERTY(int totalImagesCount READ totalImagesCount NOTIFY galleryImagesChanged)

    Q_PROPERTY(
        QString languageLevel READ languageLevel WRITE setLanguageLevel NOTIFY languageLevelChanged)
    Q_PROPERTY(
        QString characterType READ characterType WRITE setCharacterType NOTIFY characterTypeChanged)

  public:
    enum Screen
    {
        Home,
        Play,
        Gallery,
        Settings,
        Onboarding
    };
    Q_ENUM(Screen)

    explicit AppStateManager(QSharedPointer<StorageManager> storageManager,
                             QObject *parent = nullptr);

    bool init();

    // Screen
    Screen currentScreen() const;

    // User preferences
    bool onboardingCompleted() const;
    QString languageLevel() const;
    QString characterType() const;
    void setLanguageLevel(const QString &level);
    void setCharacterType(const QString &type);

    // Theme
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

    // Puzzle
    QVariantMap currentPuzzle() const;
    void generateNewPuzzle();

    // Gallery
    QVariantList galleryImages() const;
    int unlockedImagesCount() const;
    int totalImagesCount() const;

    // QML-invokable actions
    Q_INVOKABLE void goHome();
    Q_INVOKABLE void goPlay();
    Q_INVOKABLE void goGallery();
    Q_INVOKABLE void goSettings();
    Q_INVOKABLE void goOnboarding();
    Q_INVOKABLE void goBack();
    Q_INVOKABLE void completeOnboarding(const QString &languageLevel, const QString &characterType);
    Q_INVOKABLE void savePreferences();
    Q_INVOKABLE void notifyPuzzleSolved();

  signals:
    void currentScreenChanged();
    void languageLevelChanged();
    void characterTypeChanged();
    void currentPuzzleChanged();
    void galleryImagesChanged();

  private:
    void navigateTo(const Screen &screen);
    void clearActivePuzzle();
    void refreshGalleryImages();
    bool ongoingPuzzlePresent() const;

    QSharedPointer<StorageManager> m_storageManager;
    PuzzleManager m_puzzleManager;
    UserData m_userData;

    Screen m_currentScreen = Home;
    bool hasOngoingPuzzle = false;
    QVector<Screen> m_history;

    GeneratedPuzzle m_currentPuzzle;
    QVector<ImageEntry> m_galleryImages;
};

#endif // APPSTATEMANAGER_H