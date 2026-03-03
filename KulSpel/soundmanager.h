#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H
#pragma once
#include <QObject>
#include <QSoundEffect>
#include <QMediaPlayer>
#include <QAudioOutput>


class SoundManager : public QObject
{
    Q_OBJECT
public:
    static SoundManager& instance();

    void setMasterVolume(int v01_100); // 0..100
    int masterVolume() const {return m_volume; }

    // SFX
    void playShoot();
    void playHit();
    void playPickup();
    void playReload();

    // Music
    void startMenuMusic();
    void startGameMusic();
    void stopMusic();

private:
    explicit SoundManager(QObject *parent = nullptr);
    void applyVolumes();

    int m_volume = 50;

    // SFX

    QSoundEffect m_shoot;
    QSoundEffect m_hit;
    QSoundEffect m_pickup;
    QSoundEffect m_reload;

    // Music
    QMediaPlayer m_musicPlayer;
    QAudioOutput m_musicOut;

signals:
};

#endif // SOUNDMANAGER_H
