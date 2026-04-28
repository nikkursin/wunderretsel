#ifndef APPSTATEMANAGER_H
#define APPSTATEMANAGER_H

#include <QObject>
#include <QSharedPointer>

#include "Models/UserData.h"
#include "StorageManager.h"

class AppStateManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(AppStateManager)

    Q_PROPERTY(Screen currentScreen READ currentScreen NOTIFY currentScreenChanged)
    Q_PROPERTY(QString languageLevel READ languageLevel WRITE setLanguageLevel NOTIFY languageLevelChanged)
    Q_PROPERTY(QString characterType READ characterType WRITE setCharacterType NOTIFY characterTypeChanged)

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

  private:
    void navigateTo(const Screen& screen);

    UserData m_userData;
    QSharedPointer<StorageManager> m_storageManager;

    Screen m_currentScreen = Home;
    QVector<Screen> m_history;

};

#endif // APPSTATEMANAGER_H
