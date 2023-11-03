#include "ogre_texture_import.h"
#include "ogre_texture_node.h"

#include <QtQuick/QQuickWindow>

OgreTextureItem::OgreTextureItem(QQuickItem *parent):
  QQuickItem(parent)
{
  setFlag(ItemHasContents, true);
}

void OgreTextureItem::invalidateSceneGraph()
{
  m_node = nullptr;
}

void OgreTextureItem::releaseResources()
{
  m_node = nullptr;
}

QSGNode *OgreTextureItem::updatePaintNode(QSGNode *node,
                                          UpdatePaintNodeData *data)
{
  (void) data;
  auto *n = dynamic_cast<OgreTextureNode *>(node);

  if (n == nullptr && (width() <= 0 || height() <= 0))
      return nullptr;

  if (n == nullptr) {
      m_node = new OgreTextureNode(this);
      n = m_node;
  }

  m_node->sync();

  n->setTextureCoordinatesTransform(QSGSimpleTextureNode::NoTransform);
  n->setFiltering(QSGTexture::Linear);
  n->setRect(0, 0, width(), height());

  window()->update();  // ensure getting to beforeRendering() at some point

  return n;
}

void OgreTextureItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);

    if (newGeometry.size() != oldGeometry.size())
        update();
}
