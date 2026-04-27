#include "AppStateManager.h"
#include <QDebug>

AppStateManager::AppStateManager(QSharedPointer<StorageManager> storageManager, QObject *parent) : m_storageManager(storageManager), QObject{parent} {}

bool AppStateManager::init() {
    m_userData = m_storageManager->loadUserData();

    if(m_userData.isOnboardingCompleted) navigateTo(Home);
        else navigateTo(Onboarding);


    return true;
}

bool AppStateManager::onboardingCompleted() const {
    return m_userData.isOnboardingCompleted;
}

AppStateManager::Screen AppStateManager::currentScreen() const
{
    return m_currentScreen;
}

void AppStateManager::navigateTo(const Screen& screen)
{
    if (m_currentScreen == screen)
        return;

    m_history.append(m_currentScreen);
    m_currentScreen = screen;

    emit currentScreenChanged();
}

void AppStateManager::goHome()
{
    m_history.clear();
    m_currentScreen = Home;
    emit currentScreenChanged();
}

void AppStateManager::goPlay()
{
    navigateTo(Play);
}

void AppStateManager::goGallery()
{
    navigateTo(Gallery);
}

void AppStateManager::goSettings()
{
    navigateTo(Settings);
}

void AppStateManager::goOnboarding()
{
    navigateTo(Onboarding);
}

void AppStateManager::goBack()
{
    if (m_history.isEmpty()) {
        goHome();
        return;
    }

    m_currentScreen = m_history.takeLast();
    emit currentScreenChanged();
}
