#include "game_info_overlay.h"

#include "board/card_item.h"
#include "player/player.h"
#include "zones/logic/card_zone_logic.h"
#include "zones/logic/pile_zone_logic.h"
#include "zones/logic/table_zone_logic.h"

#include <QPainter>
#include <libcockatrice/card/card_info.h>

GameInfoOverlay::GameInfoOverlay(Player *_player, QGraphicsItem *parent)
    : QObject(), QGraphicsItem(parent), player(_player), width(130), height(95), permanentCount(0), landCount(0),
      creatureCount(0)
{
    setCacheMode(DeviceCoordinateCache);

    // Connect to zone change signals to update statistics
    if (player) {
        auto *tableZone = player->getTableZone();
        auto *graveZone = player->getGraveZone();

        if (tableZone) {
            connect(tableZone, &CardZoneLogic::cardCountChanged, this, &GameInfoOverlay::updateStatistics);
        }

        if (graveZone) {
            connect(graveZone, &CardZoneLogic::cardCountChanged, this, &GameInfoOverlay::updateStatistics);
        }
    }

    updateStatistics();
}

QRectF GameInfoOverlay::boundingRect() const
{
    return {0, 0, width, height};
}

void GameInfoOverlay::setSize(double _width, double _height)
{
    prepareGeometryChange();
    width = _width;
    height = _height;
}

void GameInfoOverlay::retranslateUi()
{
    update();
}

QString GameInfoOverlay::getMainCardType(const QString &fullType) const
{
    // Split the type line by " — " to separate main types from subtypes
    QStringList typeParts = fullType.split(" — ");
    if (typeParts.isEmpty())
        return QString();

    // The first part contains the main type(s)
    QString mainTypes = typeParts[0].trimmed();

    // Extract individual types (handles multiple types like "Legendary Creature")
    QStringList types = mainTypes.split(" ", Qt::SkipEmptyParts);

    // Common main types we care about
    QStringList mainTypeKeywords = {"Creature", "Land",      "Artifact", "Enchantment", "Planeswalker",
                                    "Instant",  "Sorcery",   "Battle",   "Kindred",     "Tribal",
                                    "Dungeon",  "Conspiracy"};

    for (const QString &type : types) {
        if (mainTypeKeywords.contains(type, Qt::CaseInsensitive)) {
            return type;
        }
    }

    // If no recognized type found, return the first non-supertype word
    // Skip supertypes like "Legendary", "Basic", "Snow", etc.
    QStringList supertypes = {"Legendary", "Basic", "Snow", "World", "Ongoing", "Elite", "Host"};
    for (const QString &type : types) {
        if (!supertypes.contains(type, Qt::CaseInsensitive)) {
            return type;
        }
    }

    return mainTypes;
}

void GameInfoOverlay::calculateStatistics()
{
    if (!player) {
        return;
    }

    // Reset counts
    permanentCount = 0;
    landCount = 0;
    creatureCount = 0;
    graveyardTypes.clear();

    // Count cards in table zone (battlefield)
    auto *tableZone = player->getTableZone();
    if (tableZone) {
        const CardList &cards = tableZone->getCards();
        permanentCount = cards.size();

        for (CardItem *card : cards) {
            if (!card || card->getCard().isEmpty()) {
                continue;
            }

            QString cardType = card->getCardInfo().getCardType();

            if (cardType.contains("Land", Qt::CaseInsensitive)) {
                landCount++;
            }
            if (cardType.contains("Creature", Qt::CaseInsensitive)) {
                creatureCount++;
            }
        }
    }

    // Analyze graveyard types
    auto *graveZone = player->getGraveZone();
    if (graveZone) {
        const CardList &cards = graveZone->getCards();

        for (CardItem *card : cards) {
            if (!card || card->getCard().isEmpty()) {
                continue;
            }

            QString mainType = getMainCardType(card->getCardInfo().getCardType());
            if (!mainType.isEmpty()) {
                graveyardTypes[mainType]++;
            }
        }
    }
}

void GameInfoOverlay::updateStatistics()
{
    calculateStatistics();
    update();
}

void GameInfoOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    // Semi-transparent background
    QColor bgColor(30, 30, 30, 140);
    painter->fillRect(boundingRect(), bgColor);

    // Border
    painter->setPen(QPen(QColor(100, 100, 100), 1));
    painter->drawRect(boundingRect().adjusted(0, 0, -1, -1));

    // Text settings - smaller font for compact overlay
    QFont font = painter->font();
    font.setPixelSize(9);
    painter->setFont(font);
    painter->setPen(Qt::white);

    double y = 10;
    const double lineHeight = 12;
    const double leftMargin = 5;

    // Player name
    if (player) {
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(QPointF(leftMargin, y), player->getPlayerInfo()->getName());
        y += lineHeight + 2;

        font.setBold(false);
        painter->setFont(font);
    }

    // Battlefield statistics
    painter->drawText(QPointF(leftMargin, y), tr("Permanents: %1").arg(permanentCount));
    y += lineHeight;

    painter->drawText(QPointF(leftMargin, y), tr("Lands: %1").arg(landCount));
    y += lineHeight;

    painter->drawText(QPointF(leftMargin, y), tr("Creatures: %1").arg(creatureCount));
    y += lineHeight;

    // Graveyard total count (compact)
    int graveyardTotal = 0;
    for (int count : graveyardTypes) {
        graveyardTotal += count;
    }
    if (graveyardTotal > 0) {
        painter->drawText(QPointF(leftMargin, y), tr("Graveyard: %1").arg(graveyardTotal));
    }
}
