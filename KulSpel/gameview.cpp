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
    setMouseTracking(true);
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
    setFocus();

    if(m_player)
        m_player->setFocus();
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

    // för hover känsla det funkar bara
    const QString btnStyle = R"(
QPushButton{
background-color: #2a2a2a;
color: white;
border 2px solid #555;
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
    const QString comboBoxStyle = R"(
QComboBox{
background-color: #2a2a2a;
color: white;
border 2px solid #555;
border-radius: 10px;
font-size: 18px;
padding: 8px;
}
QComboBox:hover{
background-color: #3a3a3a;
border: 2px solid #8fd3ff;
}
QComboBox:pressed{
background-color: #1f1f1f;
border: 2px solid #ffffff;
}
)";
    const QString sliderStyle = R"(
QSlider{
background-color: #2a2a2a;
color: white;
border 2px solid #555;
border-radius: 10px;
font-size: 18px;
padding: 8px;
}
QSlider:hover{
background-color: #3a3a3a;
border: 2px solid #8fd3ff;
}
QSlider:pressed{
background-color: #1f1f1f;
border: 2px solid #ffffff;
}
)";

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
    m_volumeSlider->setStyleSheet(sliderStyle);

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
    m_difficultyBox->setStyleSheet(comboBoxStyle);
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
    m_backBtn->setStyleSheet(btnStyle);
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
void GameView::startScreenShake(int durationMs, int strength)
{
    m_screenShakeActive = true;
    m_ScreenShakeDurationMs = durationMs;
    m_screenShakeStrength = strength;
    m_screenShakeTimer.restart();
    m_originalViewPos = pos();
}

void GameView::updateScreenShake()
{
    if(!m_screenShakeActive)
        return;

    if(m_screenShakeTimer.elapsed() >= m_ScreenShakeDurationMs){
        m_screenShakeActive = false;
        move(m_originalViewPos);
        return;
    }

    int offsetX = QRandomGenerator::global()->bounded(-m_screenShakeStrength, m_screenShakeStrength +1);
    int offsetY = QRandomGenerator::global()->bounded(-m_screenShakeStrength, m_screenShakeStrength +1);

    move(m_originalViewPos + QPoint(offsetX, offsetY));
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

    m_player->setFlag(QGraphicsItem::ItemIsFocusable, true);
    m_player->setFocus();

    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    m_spawnTimer.restart();
    spawnEnemy();
    updateHud();
}

void GameView::keyPressEvent(QKeyEvent* event)
{

    m_keysDown.insert(event->key());
    qDebug()<< "Gameview key press " << event->key();
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
    if(scene() == &m_gameScene && !m_gameOver && event->key() == Qt::Key_R && m_player){
        m_player->startReload();
        event->accept();
        return;
    }
    // space försök skjut (gameview äger listan och scen-add)
    if(scene() == &m_gameScene && !m_gameOver && event->key() == Qt::Key_Space && m_player) {
        Bullet* b = m_player->tryShoot();

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

void GameView::keyReleaseEvent(QKeyEvent* event)
{
    m_keysDown.remove(event->key());
    QGraphicsView::keyReleaseEvent(event);
}

void GameView::mouseMoveEvent(QMouseEvent* event)
{
    if(scene() == &m_gameScene && m_player){
        QPointF mouseScenePos = mapToScene(event->pos());
        QPointF playerCenter = m_player->sceneBoundingRect().center();

        QPointF dir = mouseScenePos - playerCenter;
        m_player->setAimDirection(dir);

    }
    QGraphicsView::mouseMoveEvent(event);
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

void GameView::createBossHealthBar()
{
    if (!m_currentBoss)
        return;

    // Ta bort gamla bars säkert
    if (m_bossHealthBarBg) {
        if (m_bossHealthBarBg->scene())
            m_gameScene.removeItem(m_bossHealthBarBg);
        delete m_bossHealthBarBg;
        m_bossHealthBarBg = nullptr;
    }

    if (m_bossHealthBarFill) {
        if (m_bossHealthBarFill->scene())
            m_gameScene.removeItem(m_bossHealthBarFill);
        delete m_bossHealthBarFill;
        m_bossHealthBarFill = nullptr;
    }

    m_bossHealthBarBg = new QGraphicsRectItem(150, 40, 500, 24);
    m_bossHealthBarBg->setPen(QPen(Qt::white));
    m_bossHealthBarBg->setBrush(QBrush(QColor(40, 40, 40)));
    m_bossHealthBarBg->setZValue(10000);
    m_gameScene.addItem(m_bossHealthBarBg);

    m_bossHealthBarFill = new QGraphicsRectItem(152, 42, 496, 20);
    m_bossHealthBarFill->setPen(Qt::NoPen);
    m_bossHealthBarFill->setBrush(QBrush(QColor(200, 50, 50)));
    m_bossHealthBarFill->setZValue(10001);
    m_gameScene.addItem(m_bossHealthBarFill);
}

void GameView::updateBossHealthBar()
{
    if(!m_currentBoss || !m_bossHealthBarFill)
        return;

    if(m_currentBoss->maxHp() <= 0)
        return;

    qreal ratio = (qreal)m_currentBoss->hp() / (qreal)m_currentBoss->maxHp();
    if(ratio < 0) ratio = 0;
    if(ratio > 1) ratio = 1;

    m_bossHealthBarFill->setRect(153, 42, 496 * ratio, 20);
}

void GameView::removeBossHealthBar()
{
    if (m_bossHealthBarBg) {
        if (m_bossHealthBarBg->scene())
            m_gameScene.removeItem(m_bossHealthBarBg);
        delete m_bossHealthBarBg;
        m_bossHealthBarBg = nullptr;
    }

    if (m_bossHealthBarFill) {
        if (m_bossHealthBarFill->scene())
            m_gameScene.removeItem(m_bossHealthBarFill);
        delete m_bossHealthBarFill;
        m_bossHealthBarFill = nullptr;
    }
}

void GameView::clearEnemies()
{
    for(Enemy* e : m_enemies){
        m_gameScene.removeItem(e);
        delete(e);
    }

    m_enemies.clear();
    m_currentBoss = nullptr;
}
void GameView::tick()
{
    if(scene() != &m_gameScene){
        return;
    }
    const QRectF bounds = m_gameScene.sceneRect();
// här ska movement ligga
    if(m_player){
    qreal dx = 0;
    qreal dy = 0;
    qreal speed = 5.0;

    if(m_keysDown.contains(Qt::Key_Left) || m_keysDown.contains(Qt::Key_A)) dx -= speed;
    if(m_keysDown.contains(Qt::Key_Right) || m_keysDown.contains(Qt::Key_D)) dx += speed;
    if(m_keysDown.contains(Qt::Key_Up) || m_keysDown.contains(Qt::Key_W)) dy -= speed;
    if(m_keysDown.contains(Qt::Key_Down) || m_keysDown.contains(Qt::Key_S)) dy = speed;

    m_player->movePlayer(dx, dy, bounds);
    }



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
        //spawn NÄR DU SKA SPAWNA ITEMS och ta bort saker. ska vara innanför !m_gameover men utanför andra funktioner

        int difficultyModifier = 0;

        switch (m_difficulty)
        {
        case Difficulty::Easy: difficultyModifier = 800; break;
        case Difficulty::Normal: difficultyModifier = 0; break;
        case Difficulty::Hard: difficultyModifier = -800; break;

        }
        updateScreenShake();
        updateBossHealthBar();
        int difficultyReduction = (m_score / 100) * 200;
        int currentSpawnMs = m_baseSpawnMs - difficultyReduction + difficultyModifier;


        if(currentSpawnMs < m_minSpawnMs)
            currentSpawnMs = m_minSpawnMs;
        const int maxPowerupsOnMap = 4;

        if(!m_bossPauseActive && m_spawnTimer.elapsed() >= currentSpawnMs) {
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



        // powerups
        if (m_powerups.size() < maxPowerupsOnMap &&
            m_powerupTimer.elapsed() >= m_powerupEveryMs){
            m_powerupTimer.restart();
            spawnPowerup();
        }

        // funktion för när boss spawnas ska alla andra enemies försvinna i 10 sekunder innan dem börjar spawna igen
        // spelet ska även implementera wildcards där bullet damage++ samt andra roliga funktioner


        // ska kunna skjuta när player moves


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
                //if(m_score>=500? m_player->activateShield(8000) : m_player->activateShield(600)); funkar inte men konceptet finns där
                SoundManager::instance().playPickup();
                if(!pRemove.contains(p))
                    pRemove.append(p);
            }
        }

        for(Powerup* p: pRemove){
            m_powerups.removeOne(p);
            m_gameScene.removeItem(p);
                qDebug() << "Deleting powerup" << p;
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
                    const qreal spd = (e->kind() == Enemy::Kind::MiniBoss) ? 7.0 : 6.0;

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

                if(!bulletsToRemove.contains(b))
                    bulletsToRemove.append(b);
                continue;
            }

            for(Enemy* e : m_enemies){

                if(enemiesToRemove.contains(e))
                    continue;
                if(b->collidesWithItem(e)){
                    if(!bulletsToRemove.contains(b))
                        bulletsToRemove.append(b);

                    e->hit();

                    if(e->isDead() && !enemiesToRemove.contains(e)){
                        enemiesToRemove.append(e);
                        m_score += 10;
                    }
                    break;
                }

            }

        }

        if(m_bossPauseActive && m_bossPauseTimer.elapsed() >= m_bossPauseMs){
            m_bossPauseActive = false;
        }

        // enemy bullets logik

        for(EnemyBullet* b : m_enemyBullets){
            b->step();

            if(!bounds.intersects(b->sceneBoundingRect())){
                if(!enemyBulletsToRemove.contains(b))
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
            //qDebug() << "Deleting enemy bullet" << b;
            delete b;
        }
        // ta bort bullets
        for (Bullet* b : bulletsToRemove){
            m_bullets.removeOne(b);
            m_gameScene.removeItem(b);
                //qDebug() << "Deleting bullet" << b;
            delete b;
        }

        // ta bort enemies
        for(Enemy* e : enemiesToRemove){
            if(e == m_currentBoss){
                m_currentBoss = nullptr;
                removeBossHealthBar();
            }
            m_enemies.removeOne(e);
            m_gameScene.removeItem(e);
            //qDebug() << "Deleting enemy " << e;
            delete e;
            // gameOvER...
        }
        if (m_player->hp() <= 0){
            m_gameOver = true;
        }

        if(!m_bossSpawned && m_score >= 1000){

            m_spawnBossNextTick = true;
            m_bossSpawned = true;

        }

    }

    /*if(m_spawnBossNextTick) {
        m_spawnBossNextTick = false;

        clearEnemies();
        spawnEnemy(Enemy::Kind::MiniBoss);

        if(!m_enemies.isEmpty()){
            m_currentBoss = m_enemies.last();
            createBossHealthBar();
        }

        //-----Screen Flash-----
        QGraphicsRectItem* flash = m_gameScene.addRect(
            m_gameScene.sceneRect(),
            Qt::NoPen,
            QBrush(QColor(255,255,255,140))
            );
        flash->setZValue(9999);

        QTimer::singleShot(120, [this, flash]() {
            m_gameScene.removeItem(flash);
            delete flash;
        });

        startScreenShake(400, 8);

        m_bossPauseActive = true;
        m_bossPauseTimer.restart();

        for(EnemyBullet* b : m_enemyBullets){
            m_gameScene.removeItem(b);
            delete b;
        }
        m_enemyBullets.clear();

    }*/

    if (m_spawnBossNextTick) {
        qDebug() << "boss block start";
        m_spawnBossNextTick = false;

        qDebug() << "before clearEnemies";
        clearEnemies();
        qDebug() << "after clearEnemies";

        qDebug() << "before spawnEnemy";
        spawnEnemy(Enemy::Kind::MiniBoss);
        qDebug() << "after spawnEnemy";

        if (!m_enemies.isEmpty()) {
            qDebug() << "before set current boss";
            m_currentBoss = m_enemies.last();
            qDebug() << "after set current boss";

            qDebug() << "before createBossHealthBar";
            createBossHealthBar();
            qDebug() << "after createBossHealthBar";
        }

        qDebug() << "before flash";
        QGraphicsRectItem* flash = m_gameScene.addRect(
            m_gameScene.sceneRect(),
            Qt::NoPen,
            QBrush(QColor(255,255,255,140))
            );
        flash->setZValue(9999);
        qDebug() << "after flash";

        QTimer::singleShot(120, [this, flash]() {
            qDebug() << "flash delete";
            m_gameScene.removeItem(flash);
            delete flash;
        });

        qDebug() << "before shake";
        startScreenShake(400, 8);
        qDebug() << "after shake";

        m_bossPauseActive = true;
        m_bossPauseTimer.restart();

        qDebug() << "boss block end";
    }


     updateHud();


}
