#include "AppStateManager.h"
#include <QDebug>

AppStateManager::AppStateManager(QSharedPointer<StorageManager> storageManager, QObject *parent) : m_storageManager(storageManager), QObject{parent} {}

bool AppStateManager::init() {
    m_userData = m_storageManager->loadUserData();

    if(m_userData.isOnboardingCompleted) navigateTo(Home);
        else navigateTo(Onboarding);


    return true;
}

AppStateManager::Screen AppStateManager::currentScreen() const
{
    return m_currentScreen;
}

bool AppStateManager::onboardingCompleted() const {
    return m_userData.isOnboardingCompleted;
}

QString AppStateManager::languageLevel() const
{
    return UserData::languageLevelToString(m_userData.level);
}

void AppStateManager::setLanguageLevel(const QString& level)
{
    auto newLevel = UserData::languageLevelFromString(level);

    if (m_userData.level == newLevel)
        return;

    m_userData.level = newLevel;
    emit languageLevelChanged();
}

QString AppStateManager::characterType() const
{
    return UserData::characterTypeToString(m_userData.characterType);
}

void AppStateManager::setCharacterType(const QString& type)
{
    auto newType = UserData::characterTypeFromString(type);

    if (m_userData.characterType == newType)
        return;

    m_userData.characterType = newType;
    emit characterTypeChanged();
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

void AppStateManager::completeOnboarding(const QString &languageLevel,
                                         const QString &characterType)
{
    if (languageLevel.isEmpty() || characterType.isEmpty())
        return;

    m_userData.level = UserData::languageLevelFromString(languageLevel);
    m_userData.characterType = UserData::characterTypeFromString(characterType);
    m_userData.isOnboardingCompleted = true;

    m_storageManager->saveUser(m_userData);

    navigateTo(Home);
}

void AppStateManager::savePreferences()
{
    m_storageManager->saveUser(m_userData);
}
