#ifndef APPSTATEMANAGER_H
#define APPSTATEMANAGER_H

#include <QObject>
#include <QSharedPointer>

#include "Models/UserData.h"
#include "StorageManager.h"
#include "PuzzleManager.h"

class AppStateManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Screen currentScreen READ currentScreen NOTIFY currentScreenChanged)
    Q_PROPERTY(QString languageLevel READ languageLevel WRITE setLanguageLevel NOTIFY languageLevelChanged)
    Q_PROPERTY(QString characterType READ characterType WRITE setCharacterType NOTIFY characterTypeChanged)
    Q_PROPERTY(GeneratedPuzzle currentPuzzle
                   READ currentPuzzle
                       NOTIFY currentPuzzleChanged)

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

    GeneratedPuzzle currentPuzzle() const;

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

  signals:
    void currentScreenChanged();
    void languageLevelChanged();
    void characterTypeChanged();
    void currentPuzzleChanged();

  private:
    void navigateTo(const Screen& screen);

    bool ongoingPuzzlePresent() const;

    UserData m_userData;
    QSharedPointer<StorageManager> m_storageManager;
    PuzzleManager m_puzzleManager;

    Screen m_currentScreen = Home;
    QVector<Screen> m_history;
    GeneratedPuzzle m_currentPuzzle;
    bool hasOngoingPuzzle = false;

};

#endif // APPSTATEMANAGER_H
