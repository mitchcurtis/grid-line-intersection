#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickPaintedItem>
#include <QPainter>
#include <QtMath>

class TileRayCast : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QSize sizeInTiles READ sizeInTiles WRITE setSizeInTiles NOTIFY sizeInTilesChanged)
    Q_PROPERTY(int tileSize READ tileSize WRITE setTileSize NOTIFY tileSizeChanged)
    Q_PROPERTY(QPoint startPos READ startPos WRITE setStartPos NOTIFY startPosChanged)
    Q_PROPERTY(QPoint endPos READ endPos WRITE setEndPos NOTIFY endPosChanged)

public:
    TileRayCast() :
        mSizeInTiles(QSize(10, 10)),
        mTileSize(32)
    {
    }

    void paint(QPainter *painter) override
    {
        for (int y = 0; y < mSizeInTiles.height(); ++y) {
            for (int x = 0; x < mSizeInTiles.width(); ++x) {
                painter->setBrush(mTiles.at(y * mSizeInTiles.width() + x));
                painter->drawRect(x * mTileSize, y * mTileSize, mTileSize, mTileSize);
            }
        }

        const QPoint startTile = startTilePos();
        const QPointF startTilePosCentre = startTile + QPointF(0.5, 0.5);
        const QPoint startPosCentre = (startTilePosCentre * mTileSize).toPoint();

        const QPoint endTile = endTilePos();
        const QPointF endTilePosCentre = endTile + QPointF(0.5, 0.5);
        const QPoint endPosCentre = (endTilePosCentre * mTileSize).toPoint();

        painter->setPen(Qt::black);
        painter->drawLine(startPosCentre, endPosCentre);

        painter->drawText(startPosCentre, QString::fromLatin1("%1, %2").arg(startTile.x()).arg(startTile.y()));
        painter->drawText(endPosCentre, QString::fromLatin1("%1, %2").arg(endTile.x()).arg(endTile.y()));
    }

    QSize sizeInTiles() const
    {
        return mSizeInTiles;
    }

    void setSizeInTiles(const QSize &sizeInTiles)
    {
        if (sizeInTiles == mSizeInTiles)
            return;

        mSizeInTiles = sizeInTiles;
        updateSize();
        rayCast();
        update();
        emit sizeInTilesChanged();
    }

    int tileSize() const
    {
        return mTileSize;
    }

    void setTileSize(int tileSize)
    {
        tileSize = qBound(8, tileSize, 128);
        if (tileSize == mTileSize)
            return;

        mTileSize = tileSize;
        updateSize();
        rayCast();
        update();
        emit tileSizeChanged();
    }

    QPoint startPos() const
    {
        return mStartPos;
    }

    void setStartPos(QPoint startPos)
    {
        startPos = QPoint(qBound(0, startPos.x(), qFloor(implicitWidth() - 1)), qBound(0, startPos.y(), qFloor(implicitHeight() - 1)));
        if (startPos == mStartPos)
            return;

        mStartPos = startPos;
        rayCast();
        update();
        emit startPosChanged();
    }

    QPoint endPos() const
    {
        return mEndPos;
    }

    void setEndPos(QPoint endPos)
    {
        endPos = QPoint(qBound(0, endPos.x(), qFloor(implicitWidth() - 1)), qBound(0, endPos.y(), qFloor(implicitHeight() - 1)));
        if (endPos == mEndPos)
            return;

        mEndPos = endPos;
        rayCast();
        update();
        emit endPosChanged();
    }

signals:
    void sizeInTilesChanged();
    void tileSizeChanged();
    void startPosChanged();
    void endPosChanged();

protected:
    void componentComplete() override
    {
        QQuickPaintedItem::componentComplete();
        updateSize();
        rayCast();
        update();
    }

private:
    QPoint startTilePos() const
    {
        // Avoid QPoint's operator/ because it uses qRound().
        return QPoint(mStartPos.x() / mTileSize, mStartPos.y() / mTileSize);
    }

    QPoint endTilePos() const
    {
        return QPoint(mEndPos.x() / mTileSize, mEndPos.y() / mTileSize);
    }

    void visit(int x, int y)
    {
        mTiles[y * mSizeInTiles.width() + x] = QColor::fromRgb(255, 99, 71);
    }

    void rayCast()
    {
        if (!isComponentComplete())
            return;

        mTiles.clear();
        mTiles.fill(QColor::fromRgb(255, 222, 173), mSizeInTiles.width() * mSizeInTiles.height());

        const QPoint startTile = startTilePos();
        const QPoint endTile = endTilePos();
        // http://playtechs.blogspot.no/2007/03/raytracing-on-grid.html
        int x0 = startTile.x();
        int y0 = startTile.y();
        int x1 = endTile.x();
        int y1 = endTile.y();

        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int x = x0;
        int y = y0;
        int n = 1 + dx + dy;
        int x_inc = (x1 > x0) ? 1 : -1;
        int y_inc = (y1 > y0) ? 1 : -1;
        int error = dx - dy;
        dx *= 2;
        dy *= 2;

        for (; n > 0; --n)
        {
            visit(x, y);

            if (error > 0)
            {
                x += x_inc;
                error -= dy;
            }
            else if (error < 0)
            {
                y += y_inc;
                error += dx;
            }
            else if (error == 0) {
                // Ensure that perfectly diagonal lines don't take up more tiles than necessary.
                // http://playtechs.blogspot.no/2007/03/raytracing-on-grid.html?showComment=1281448902099#c3785285092830049685
                x += x_inc;
                y += y_inc;
                error -= dy;
                error += dx;
                --n;
            }
        }

        update();
    }

    void updateSize()
    {
        setImplicitWidth(mSizeInTiles.width() * mTileSize);
        setImplicitHeight(mSizeInTiles.height() * mTileSize);
    }

    QSize mSizeInTiles;
    int mTileSize;
    QPoint mStartPos;
    QPoint mEndPos;

    QVector<QColor> mTiles;
};

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    qmlRegisterType<TileRayCast>("RayCast", 1, 0, "TileRayCast");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

#include "main.moc"
