#include "AppStateManager.h"

AppStateManager::AppStateManager(QSharedPointer<StorageManager> storageManager, QObject *parent) : m_storageManager(storageManager), QObject{parent} {}

bool AppStateManager::init() {
    m_userData = m_storageManager->loadUserData();

    emit onboardingCompletedChanged();

    return true;
}

bool AppStateManager::onboardingCompleted() const {
    return m_userData.isOnboardingCompleted;
}
