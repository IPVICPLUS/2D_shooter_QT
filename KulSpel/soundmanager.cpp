#include "soundmanager.h"
#include <QUrl>


SoundManager& SoundManager::instance()
{
    static SoundManager s;
    return s;
}
SoundManager::SoundManager(QObject *parent)
    : QObject{parent}
{
    // koppla music player ->audio outpout qt6
    m_musicPlayer.setAudioOutput(&m_musicOut);

    // ladda ljud (lägg i .qrc: : /sfx/shoot.wav etc
    m_shoot.setSource(QUrl("qrc:/sfx/shoot.wav"));
    m_hit.setSource(QUrl("qrc:/sfx/hit.wav"));
    m_pickup.setSource(QUrl("qrc:/sfx/pickup.wav"));
    m_reload.setSource(QUrl("qrc:/sfx/reload.wav"));

    // lite volumbalans multipliceras med master
    m_shoot.setVolume(0.8);
    m_hit.setVolume(0.8);
    m_pickup.setVolume(0.9);
    m_reload.setVolume(0.7);

    applyVolumes();
}

void SoundManager::setMasterVolume(int v01_100)
{
    if(v01_100 < 0 ) v01_100 = 0;
    if(v01_100 > 100 ) v01_100 = 100;
    m_volume = v01_100;
    applyVolumes();
}

void SoundManager::applyVolumes()
{
   // QSoundEffect har egen volume 0.1 redan satt per sfx
    // music output styr här
   m_musicOut.setVolume(m_volume / 100.0);
}

void SoundManager::playShoot() {m_shoot.play();}
void SoundManager::playHit() {m_hit.play();}
void SoundManager::playPickup() {m_pickup.play();}
void SoundManager::playReload() {m_reload.play();}

void SoundManager::startMenuMusic()
{
    m_musicPlayer.setSource(QUrl("qrc:/music/menu.mp3"));
    m_musicPlayer.setLoops(QMediaPlayer::Infinite);
    m_musicPlayer.play();
}

void SoundManager::startGameMusic()
{
    m_musicPlayer.setSource(QUrl("qrc:/music/game.mp3"));
    m_musicPlayer.setLoops(QMediaPlayer::Infinite);
    m_musicPlayer.play();
}

void SoundManager::stopMusic()
{
    m_musicPlayer.stop();
}
