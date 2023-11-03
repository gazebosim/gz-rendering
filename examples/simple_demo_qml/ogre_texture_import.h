#ifndef OGRE_TEXTURE_IMPORT_HH
#define OGRE_TEXTURE_IMPORT_HH

#include <QtQuick/QQuickItem>

class OgreTextureNode;

class OgreTextureItem: public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit OgreTextureItem(QQuickItem *parent = nullptr);

protected:
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;
    void geometryChange(
      const QRectF &newGeometry, const QRectF &oldGeometry) override;

private slots:
    void invalidateSceneGraph();

private:
    void releaseResources() override;

    OgreTextureNode *m_node = nullptr;
};

#endif  // OGRE_TEXTURE_IMPORT_HH
