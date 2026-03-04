#include "gameview.h"
#include "enemybullet.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "healthpickup.h"
#include "soundmanager.h"
#include "powerup.h"

#include <QBrush>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QPen>
#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <cmath>
#include <QPushButton>
#include <QTimer>

GameView::GameView(QWidget *parent)
    : QGraphicsView(parent)
{

    setFixedSize(800, 600); // storleken på fönstret

    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    // setupScene rects

    m_menuScene.setSceneRect(0, 0, 800, 600);
    m_gameScene.setSceneRect(0, 0, 800, 600);

    setBackgroundBrush(QBrush(QColor(20, 20, 20)));

    buildMenu();
    showMenu();

    connect(&m_timer, &QTimer::timeout, this, &GameView::tick);
    m_timer.start(16);

}

void GameView::buildMenu()
{
    // 1) stoppa animationen inna nvi rensar scenen
    if(m_titleTimeLine){
        m_titleTimeLine->stop();
    }
    // 2 ) nollap ekare så vi inte råkar avända bortstädade items
    m_menuTitle = nullptr;
    m_previewPlayer = nullptr;
    m_startBtn = nullptr;
    m_quitBtn = nullptr;

    // 3 rensa scenen
    m_menuScene.clear();
    //titel
    m_menuTitle = m_menuScene.addText("MY 2D SHOOTER");
    m_menuTitle->setDefaultTextColor(Qt::white);
    m_menuTitle->setScale(2.0);
    m_menuTitle->setPos(300, 120);


    // instruktioner

    auto* info = m_menuScene.addText("WASD/Arrows to move\nSpace to shoot\nR to reload\nEnter to restart (in game over)\nEsc to return to menu");
    info->setDefaultTextColor(Qt::lightGray);
    info->setPos(300,210);

// för hover känsla det funkar bara
    const QString btnStyle = R"(
QPushButton{
background-color: #2a2a2a;
color: white;
bordet 2px solid #555;
border-radius: 10px;
font-size: 18px;
padding: 8px;
}
QPushButton:hover{
background-color: #3a3a3a;
border: 2px solid #8fd3ff;
}
QPushBUtton:pressed{
background-color: #1f1f1f;
border: 2px solid #ffffff;
}
)";

    // start button
    m_startBtn = new QPushButton("Start");
    m_startBtn->setFixedSize(200, 45);
    m_startBtn->setStyleSheet(btnStyle);
    auto* startProxy = m_menuScene.addWidget(m_startBtn);
    startProxy->setPos(300,360);
    connect(m_startBtn, &QPushButton::clicked, this, &GameView::startGameFromMenu);

    // quit button

    m_quitBtn = new QPushButton("Quit");
    m_quitBtn->setFixedSize(200,45);
    m_quitBtn->setStyleSheet(btnStyle);
    auto* quitProxy = m_menuScene.addWidget(m_quitBtn);
    quitProxy->setPos(300, 420);
    connect(m_quitBtn, &QPushButton::clicked, this, &GameView::quitFromMenu);


// settings bytton
    m_settingsBtn = new QPushButton("Settings");
    m_settingsBtn->setFixedSize(200,45);
    m_settingsBtn ->setStyleSheet(btnStyle);
    auto* settingsProxy = m_menuScene.addWidget(m_settingsBtn);
    settingsProxy->setPos(300, 480);
    connect(m_settingsBtn, &QPushButton::clicked, this, &GameView::openSettings);

    // Preview "player" i bakgrunden
    m_previewPlayer = new QGraphicsRectItem();
    m_previewPlayer->setRect(0, 0, 28, 28);
    m_previewPlayer->setBrush(QBrush(QColor(80, 160, 255)));
    m_previewPlayer->setPen(Qt::NoPen);
    m_previewPlayer->setOpacity(0.35); // så den känns som bakgrund
    m_previewPlayer->setZValue(-10); // bkaom text / knappar

    m_menuScene.addItem(m_previewPlayer);

    m_previewBaseY = 420;
    m_previewT = 0.0;
    m_previewVel = QPointF(2.2, 0.0);
    m_previewPlayer->setPos(60, m_previewBaseY);


    //andas funktion

    if(!m_titleTimeLine){
        m_titleTimeLine = new QTimeLine(1200, this); // 1.2 sek loop
        m_titleTimeLine->setLoopCount(0);

        connect(m_titleTimeLine, &QTimeLine::valueChanged, this, [this](qreal v){
            if(!m_menuTitle) return;

            // v går 0....1 vi gör en andning
            qreal pulse = (v < 0.5) ? (v * 2.0) : ((1.0 - v) * 2.0);

            // skala mellan 2.15 och 2.35
            qreal s = 2.15 + pulse * 0.20;
            m_menuTitle->setScale(s);

            // liten "glow"- känsla via opacity
            m_menuTitle->setOpacity(0.75 + pulse * 0.25);

        });

        m_titleTimeLine->start();
    }


}
// -- menu shit --
void GameView::showMenu()
{
    setScene(&m_menuScene);
    SoundManager::instance().startMenuMusic();
}

void GameView::showGame()
{
    setScene(&m_gameScene);
}

void GameView::startGameFromMenu()
{
    resetGame();
    showGame();
    SoundManager::instance().startGameMusic();

}

void GameView::buildSettings()
{
    m_settingsScene.clear();
    m_settingsScene.setSceneRect(0, 0, 800, 600);

    // titel
    auto* title = m_settingsScene.addText("SETTINGS");
    title->setDefaultTextColor(Qt::white);
    title->setScale(2.0);
    title->setPos(300,80);

    // volume
    auto* volumeText = m_settingsScene.addText("Volume");
    volumeText->setDefaultTextColor(Qt::lightGray);
    volumeText->setPos(250,200);

    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(m_volume);
    m_volumeSlider->setFixedWidth(300);

    auto* volProxy = m_settingsScene.addWidget(m_volumeSlider);
    volProxy->setPos(250, 240);

    connect(m_volumeSlider, &QSlider::valueChanged, this, [this](int value){
        m_volume = value;
        SoundManager::instance().setMasterVolume(value);
        // här koppar jag in ljud senare

    });

    // --Difficulty--
    auto* diffText = m_settingsScene.addText("Difficulty");
    diffText->setDefaultTextColor(Qt::lightGray);
    diffText->setPos(250, 320);

    m_difficultyBox = new QComboBox();
    m_difficultyBox->addItem("Easy");
    m_difficultyBox->addItem("Normal");
    m_difficultyBox->addItem("Hard");

    m_difficultyBox->setCurrentIndex(1); // normal by default

    auto* diffProxy = m_settingsScene.addWidget(m_difficultyBox);
    diffProxy->setPos(250, 360);
    connect(m_difficultyBox, &QComboBox::currentIndexChanged, this
            ,[this](int index){
        if(index == 0 ) m_difficulty = Difficulty::Easy;
        if(index == 1 ) m_difficulty = Difficulty::Normal;
        if(index == 2 ) m_difficulty = Difficulty::Hard;

    });

    // -- Back Button --

    m_backBtn = new QPushButton("Back");
    m_backBtn->setFixedSize(200, 45);

    auto* backProxy = m_settingsScene.addWidget(m_backBtn);
    backProxy->setPos(300, 470);

    connect(m_backBtn, &QPushButton::clicked, this, &GameView::showMenu);
}
void GameView::openSettings()
{
    buildSettings();
    showSettings();
}
void GameView::showSettings()
{
    setScene(&m_settingsScene);
}

void GameView::quitFromMenu()
{
    close(); // stänger programmet
}
void GameView::resetGame()
{

    // rensa game-scenen
    m_gameScene.clear();

    //töm listor (de är rensade pga clear , men vi nollar pointer)
    m_bullets.clear();
    m_enemies.clear();
    m_player = nullptr;
    m_pickups.clear();
    m_pickupSpawnTimer.restart();
    m_powerups.clear();
    m_powerupTimer.restart();
    for(auto* b : m_enemyBullets) {m_gameScene.removeItem(b); delete b; }
    m_enemyBullets.clear();
    m_score = 0;
    m_gameOver =false;
    m_bossSpawned =false;
    // HUD

    m_hud = m_gameScene.addText("");
    m_hud->setDefaultTextColor(Qt::white);
    m_hud->setZValue(999);
    m_hud->setPos(10, 10);

    m_player = new Player();
    m_gameScene.addItem(m_player);
    m_player->setPos(100, 300);

    m_spawnTimer.restart();
    spawnEnemy();
    updateHud();
}

void GameView::keyPressEvent(QKeyEvent* event)
{

    // ESC: tillbaka till menyn (från game)
    if(event->key()== Qt::Key_Escape){
        showMenu();
        event->accept();
        return;
    }

    // om vi är i menyn : Enter startar
    if(scene() == &m_menuScene){
        if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter){
            startGameFromMenu();
            event->accept();
            return;
        }
        QGraphicsView::keyPressEvent(event);
        return;
    }

    // gameScene:
    if(m_gameOver && (event->key() == Qt:: Key_Return || event->key() == Qt::Key_Enter)){
        resetGame();
        event->accept();
        return;
    }

    // space försök skjut (gameview äger listan och scen-add)
    if(scene() == &m_gameScene && !m_gameOver && event->key() == Qt::Key_Space && m_player) {
        Bullet* b = m_player->tryShoot();
        qDebug() << "Space pressed, try shoot returned: " << (b ? "BULLET" : "nullptr")
                 <<"ammo:" << m_player->ammoInMag()
                 <<"reloading:" << m_player->isReloading();
        if(b){
            m_gameScene.addItem(b);
            m_bullets.append(b);
            SoundManager::instance().playShoot();

        }
        event->accept();
        return;
    }
    QGraphicsView::keyPressEvent(event);

}

void GameView::spawnPowerup()
{
    //slumpa typ : mer health tidigt, mer buffar senare
    int r = QRandomGenerator::global()->bounded(100);

    Powerup::Type t = Powerup::Type::Health;
    if (r < 15) t = Powerup::Type::Shield; // 15%
    else if (r < 30) t = Powerup::Type::Minigun; // 15%
    else t = Powerup::Type::Health; // 70%

    auto* p = new Powerup(t);
    m_gameScene.addItem(p);

    QRectF b = m_gameScene.sceneRect();
    qreal x = QRandomGenerator::global()->bounded((int)b.width() -40 );
    qreal y = QRandomGenerator::global()->bounded((int)b.height() - 40);
    p->setPos(x, y);
    m_powerups.append(p);

}



void GameView::spawnEnemy()
{
    auto* e = new Enemy();
    m_gameScene.addItem(e);

    // slumpa spawn nära kanter

    const QRectF b = m_gameScene.sceneRect();
    int side = QRandomGenerator::global()->bounded(4);

    qreal x = 0, y =0;
    if (side == 0) {x = b.left() +10; y = QRandomGenerator::global()->bounded((int)b.height()); }
    if (side == 1) {x = b.right() - 50; y = QRandomGenerator::global()->bounded((int)b.height());}
    if (side == 2) {x = QRandomGenerator::global()->bounded((int)b.width()); y= b.top() +10; }
    if (side == 3) {x = QRandomGenerator::global()->bounded((int)b.width()); y= b.bottom() -50; }

    e->setPos(x, y);
    m_enemies.append(e);
}
void GameView::spawnEnemy(Enemy::Kind kind)
{
    auto* e = new Enemy(kind);
    m_gameScene.addItem(e);

    // slumpa spawn nära kanter

    const QRectF b = m_gameScene.sceneRect();
    int side = QRandomGenerator::global()->bounded(4);

    qreal x = 0, y =0;
    if (side == 0) {x = b.left() +10; y = QRandomGenerator::global()->bounded((int)b.height()); }
    if (side == 1) {x = b.right() - 60; y = QRandomGenerator::global()->bounded((int)b.height());}
    if (side == 2) {x = QRandomGenerator::global()->bounded((int)b.width()); y= b.top() +10; }
    if (side == 3) {x = QRandomGenerator::global()->bounded((int)b.width()); y= b.bottom() -50; }

    e->setPos(x, y);
    m_enemies.append(e);
}

void GameView::updateHud()
{
    if(!m_hud || !m_player) return;

    if(m_gameOver){
        m_hud->setPlainText("GAME OVER - Press ENTER to restart or ESC for menu");
        return;
    }
    QString reloadText = m_player->isReloading() ? " (RELOADING) " : "" ;
    QString buffs;
    if(m_player->shieldActive()) buffs += " SHIELD";
    if(m_player->minigunActive()) buffs += " MINIGUN";
    m_hud->setPlainText(QString("HP: %1     Ammo: %2/%3%4   Score: %5   Enemeies:%6      Buffs: %7  [R = reload, Enter = Restart]")
                            .arg(m_player->hp())
                            .arg(m_player->ammoInMag())
                            .arg(m_player->magSize())
                            .arg(reloadText)
                            .arg(m_score)
                            .arg(m_enemies.size())
                            .arg(buffs));



}

void GameView::cleanup()
{
    // inget här just nu men bra plats för framtide pooler
}

void GameView::spawnHealthPickup()
{
    auto* p = new HealthPickUp();
    m_gameScene.addItem(p);

    QRectF b = m_gameScene.sceneRect();
    qreal x = QRandomGenerator::global()->bounded((int)b.width() -40);
    qreal y = QRandomGenerator::global()->bounded((int)b.height() - 40);

    p->setPos(x, y);
    m_pickups.append(p);
}
void GameView::tick()
{
    if(scene() != &m_gameScene){
        return;
    }
    const QRectF bounds = m_gameScene.sceneRect();

    if(scene() == &m_menuScene){
        if(m_previewPlayer){
            m_previewT += 0.08;

            QPointF p = m_previewPlayer->pos();
            p.setX(p.x() + m_previewVel.x());

            // studsa på kanter

            if(p.x() < 40) { p.setX(40); m_previewVel.setX(std::abs(m_previewVel.x()));}
            if(p.x() >760) { p.setX(760); m_previewVel.setX(-std::abs(m_previewVel.x()));}

            p.setY(m_previewBaseY + std::sin(m_previewT) * 18.0);
            m_previewPlayer->setPos(p);

        }
        return;
    }
    if(!m_player ) return;

    if(!m_gameOver){
        //spawn NÄR DU SKA SPAWNA ITEMS

        int difficultyModifier = 0;

        switch (m_difficulty)
        {
        case Difficulty::Easy: difficultyModifier = 800; break;
        case Difficulty::Normal: difficultyModifier = 0; break;
        case Difficulty::Hard: difficultyModifier = -800; break;

        }

        int difficultyReduction = (m_score / 100) * 200;
        int currentSpawnMs = m_baseSpawnMs - difficultyReduction + difficultyModifier;


        if(currentSpawnMs < m_minSpawnMs)
            currentSpawnMs = m_minSpawnMs;
        const int maxPowerupsOnMap = 3;

        if(m_spawnTimer.elapsed() >= currentSpawnMs) {
            m_spawnTimer.restart();


            auto pickKind = [this](){
                           int r = QRandomGenerator::global()->bounded(100);

                if (m_score < 300) {
                return Enemy::Kind::Chaser;
                }else if( m_score < 700){
                    if (r < 70) return Enemy::Kind::Chaser;
                    if (r < 90) return Enemy::Kind::Shooter;
                    return Enemy::Kind::Tank;
                }else{
                    if( r< 55) return Enemy::Kind::Chaser;
                    if (r <80) return Enemy::Kind::Shooter;
                    return Enemy::Kind::Tank;
                }

            };
            Enemy::Kind kind = pickKind();
            spawnEnemy(kind);
        }

        if(!m_bossSpawned && m_score >= 1000){
            spawnEnemy(Enemy::Kind::Miniboss);
            m_bossSpawned = true;
        }

        // powerups
        if (m_powerups.size() < maxPowerupsOnMap &&
            m_powerupTimer.elapsed() >= m_powerupEveryMs){
            m_powerupTimer.restart();
            spawnPowerup();
        }




        // player Update (movement + reload timer)

        m_player->updateMovement(bounds);

        // bullets update + collisions (SÄKERT)

        QList<Bullet*> bulletsToRemove;
        QList<Enemy*> enemiesToRemove;
        QList<Powerup*> pRemove;
        QList<EnemyBullet*> enemyBulletsToRemove;


        for(Powerup* p: m_powerups){
            if(p->collidesWithItem(m_player)){
                // apply effekt
                if(p->powerupType() == Powerup::Type::Health){
                    m_player->takeDamage(-1);
                    auto* t = m_gameScene.addText("+HP");

                    t->setDefaultTextColor(Qt::green);
                    t->setPos(m_player->pos().x(), m_player->pos().y() -20);
                    t->setZValue(1000);
                    QTimer::singleShot(600, this, [this, t](){
                        m_gameScene.removeItem(t);
                        delete t;
                    });
                }
                if(p->powerupType() == Powerup::Type::Shield) m_player->activateShield(6000); // 6 sek
                if(p->powerupType() == Powerup::Type::Minigun) m_player->activateMinigun(6000);

                SoundManager::instance().playPickup();
                pRemove.append(p);
            }
        }

        for(Powerup* p: pRemove){
            m_powerups.removeOne(p);
            m_gameScene.removeItem(p);
            delete p;
        }
        const QPointF playerCenter = m_player->sceneBoundingRect().center();
        // enemyupdate + contact damage


        for(Enemy* e : m_enemies){
            e->step(playerCenter);
            if(e->collidesWithItem(m_player)){
                m_player->takeDamage(1);

            }
                if(e->wantsToShoot()){
                    e->onShotFired();
                    QPointF d = e->shootDirection(playerCenter);
                    const qreal spd = (e->kind() == Enemy::Kind::Miniboss) ? 7.0 : 6.0;

                    auto* eb = new EnemyBullet(d.x()*spd, d.y()*spd);


                    eb->setPos(e->sceneBoundingRect().center());

                    m_gameScene.addItem(eb);
                    m_enemyBullets.append(eb);
                }
                  e->updateVisuals();

        }

// player bullets
        for(Bullet* b : m_bullets) {
            b->step();

            if(!bounds.intersects(b->sceneBoundingRect())){

                bulletsToRemove.append(b);
                continue;
            }

            for(Enemy* e : m_enemies){
                if(b->collidesWithItem(e)){
                    e->hit();
                    bulletsToRemove.append(b);
                    if(e->isDead()){
                        enemiesToRemove.append(e);
                        m_score += 10;
                    }
                    break;
                }

            }

        }

        // enemy bullets logik

        for(EnemyBullet* b : m_enemyBullets){
            b->step();

            if(!bounds.intersects(b->sceneBoundingRect())){
                enemyBulletsToRemove.append(b);
                continue;
            }
            if(b->collidesWithItem(m_player)){
                m_player->takeDamage(1);
                enemyBulletsToRemove.append(b);
            }

        }
         // ta bort enemy bullets
        for(EnemyBullet* b: enemyBulletsToRemove){
            m_enemyBullets.removeOne(b);
            m_gameScene.removeItem(b);
            delete b;
        }
        // ta bort bullets
        for (Bullet* b : bulletsToRemove){
            m_bullets.removeOne(b);
            m_gameScene.removeItem(b);
            delete b;
        }

        // ta bort enemies
        for(Enemy* e : enemiesToRemove){
            m_enemies.removeOne(e);
            m_gameScene.removeItem(e);
            delete e;
        }

        // game over

        if (m_player->hp() <= 0){
            m_gameOver = true;
        }

    }


     updateHud();


}
