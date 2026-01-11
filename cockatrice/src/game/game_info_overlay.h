/**
 * @file game_info_overlay.h
 * @ingroup GameGraphics
 * @ingroup GameWidgets
 * @brief Overlay panel displaying player game statistics
 */

#ifndef GAME_INFO_OVERLAY_H
#define GAME_INFO_OVERLAY_H

#include "../game_graphics/board/abstract_graphics_item.h"

#include <QGraphicsObject>
#include <QMap>

class Player;

/**
 * @brief Overlay displaying game statistics for a player
 *
 * Shows real-time information about:
 * - Number of permanents in play
 * - Number of lands in play
 * - Number of creatures in play
 * - Breakdown of card types in graveyard
 */
class GameInfoOverlay : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

private:
    Player *player;
    double width, height;
    int permanentCount;
    int landCount;
    int creatureCount;
    QMap<QString, int> graveyardTypes;

    void calculateStatistics();
    QString getMainCardType(const QString &fullType) const;

public:
    explicit GameInfoOverlay(Player *_player, QGraphicsItem *parent = nullptr);
    [[nodiscard]] QRectF boundingRect() const override;
    void setSize(double _width, double _height);
    void retranslateUi();

public slots:
    void updateStatistics();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

#endif // GAME_INFO_OVERLAY_H
