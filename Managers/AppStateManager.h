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

    Q_PROPERTY(bool onboardingCompleted
                   READ onboardingCompleted
                       NOTIFY onboardingCompletedChanged)

  public:
    explicit AppStateManager(QSharedPointer<StorageManager> storageManager, QObject *parent = nullptr);

    bool init();

    bool onboardingCompleted() const;

  signals:

    void onboardingCompletedChanged();

  private:

    UserData m_userData;
    QSharedPointer<StorageManager> m_storageManager;

};

#endif // APPSTATEMANAGER_H
